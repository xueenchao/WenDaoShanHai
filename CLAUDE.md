# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

《问道·山海》 — a turn-based Xianxia (cultivation) game combining BG3-style 2D grid tactical combat, Diablo 3 loot-driven affix systems, and Pokémon-style Shan Hai Jing beast capture/raising. Solo indie project with a handcrafted C++ engine using AI-generated art/music assets.

Design document (Chinese): `《问道·山海》.md`

## Build Commands

```bash
# Configure (uses CMakePresets.json: MinGW Makefiles + vcpkg toolchain)
cmake --preset default

# Build
cmake --build --preset default
# Or directly:
cd build && mingw32-make

# Run
./build/MyGame.exe
```

No test suite exists yet. No linter is configured.

## Tech Stack

- **C++20**, MinGW-Winlibs (GCC), CMake 3.10+
- **SDL3** (not SDL2) — window, rendering, input, events
- **SDL3_image** — image loading (PNG/JPG/BMP/WEBP)
- **SDL3_ttf** — TrueType font rendering
- **SDL3_mixer** — audio (Mixer→Audio→Track model, NOT legacy channel-based API)
- **GLM** — math (vectors, matrices)
- **cJSON** — JSON parsing for data-driven content
- **vcpkg** (x64-mingw-dynamic triplet) — package management via `CMakePresets.json`

Source and execution charset are forced to UTF-8 (`-finput-charset=UTF-8 -fexec-charset=UTF-8`). `SDL_MAIN_HANDLED` is defined — we control `main()` ourselves.

## Architecture

### Layered Module Design

```
main.cpp (application layer — callbacks wired to GameLoop)
  │
├─ Game layer (src/Game/) — combat, AI, skills (gameplay logic)
│   └─ CombatScene ─ integrates GridMap, TurnManager, CombatAI, Camera2D, UIManager
│
├─ Core modules (src/Core/) — engine subsystems wrapping SDL3
│   ├─ Window ←─ Renderer ←─ Texture
│   │                 ↑          Font renders to SDL_Texture
│   ├─ Camera2D — world/screen coordinate transform, zoom, smooth follow
│   ├─ CollisionSystem — AABB + spatial hash grid, layer-mask filtering
│   ├─ Scene / SceneManager — stack-based scene lifecycle (push/pop/switch)
│   ├─ ResourceManager — generic ResourceCache<T> for Texture/Font/Audio
│   ├─ EventHandler ←─ GameLoop
│   ├─ Audio (independent — Mixer/Audio/Track + tag-based grouping)
│   ├─ Log (standalone — singleton, 5-level macros, console+file dual output)
│   └─ UI/ (widget framework)
│       ├─ UIElement (base: tree hierarchy, hit-test, hover tracking)
│       ├─ UIPanel, UILabel, UIButton (3-state), UIProgressBar
│       └─ UIManager (root node, input routing, render dispatch)
│
GameLoop (loop controller — calls OnInit → pollEvents → OnUpdate → OnRender → update → cap FPS)
```

**Dependency rule**: Game layer (`src/Game/`) depends on Core. Core modules depend on SDL3. `Renderer` holds a `Window&`. `Texture`/`Font` need a `Renderer&`. `UIManager` takes a `Renderer*` (set just before render — it's constructed before the Renderer exists). `Audio` and `Log` are standalone (no SDL dependency — pure C++).

**Include paths**: CMake adds `src/` as an include directory. Files in `src/Core/` include each other directly (`"Renderer.h"`). Files in subdirectories (`src/Core/UI/`, `src/Game/`) use `../` for same-layer files or `Core/` / `Game/` prefixes for cross-layer includes.

**Resource ownership**: Each class owns its SDL pointer via RAII. Copy is deleted; move is supported. Destruction order matters: Renderer before Window, Font before TTF_Quit, UIManager before Font.

### Core Module API Summary

| Module | Wraps | Key pattern |
|--------|-------|-------------|
| `Window` | `SDL_Window*` | create()/destroy() lifecycle; caches title/size |
| `Renderer` | `SDL_Renderer*` | Bound to Window ref; drawTexture overloads (position/scale/clip/rotate); logical size with letterbox |
| `Texture` | `SDL_Texture*` | loadFromFile/loadFromMemory/createBlank/createFromSurface; color mod/alpha/blend |
| `Audio` | `MIX_Mixer*` + `MIX_Audio*` map | Name-based audio lookup; playWithTag for group control (e.g. "bgm", "sfx"); master + per-tag gain |
| `Font` | `TTF_Font*` | Three quality levels (Solid/Shaded/Blended); renderText returns `SDL_Texture*` (caller must destroy); measureText for layout |
| `EventHandler` | Keyboard/mouse state maps | Dual-frame state (current + previous) for isKeyJustPressed/isKeyJustReleased; must call update() each frame end |
| `GameLoop` | Callback functions | std::function callbacks; deltaTime capped at 0.1s; FPS tracking |
| `Log` | (none — pure C++) | Singleton with LOG_DEBUG/INFO/WARNING/ERROR/FATAL macros; auto-captures `__FILE__`/`__LINE__`/`__FUNCTION__`; console + file output; `setMinLevel()` filtering; thread-safe via `std::mutex` |
| `Scene` | Scene stack | pushScene/popScene/switchScene; onEnter/onExit/onUpdate/onRender lifecycle |
| `ResourceManager` | `ResourceCache<T>` | Template cache keyed by string name; loadTexture/loadFont/loadAudio with fallback |
| `Camera2D` | Viewport transform | worldToScreen/screenToWorld; exponential-decay smooth follow; bounds clamping; zoom 0.1–10 |
| `CollisionSystem` | Spatial hash grid | AABB overlap + layer-mask filtering; checkMove for movement validation; swap-and-pop removal |

### UI System (`src/Core/UI/`)

Screen-space widget hierarchy. Coordinates are pixels, independent of Camera2D.

- **`UIElement`** — base: `mX/mY/mWidth/mHeight`, parent-relative position, `getWorldPosition()` walks up to root, `hitTest()` finds deepest element at point
- **`UIPanel`** — filled rectangle with background color, child container
- **`UILabel`** — renders text via Font; auto-sizes if width/height left at 0
- **`UIButton`** — 3 visual states (Normal/Hover/Pressed) with configurable colors per state; `mOnClick` callback fires on mouse-up within bounds
- **`UIProgressBar`** — dual-layer bar (background + foreground); ratio from `mValue/mMaxValue`; optional centered text label
- **`UIManager`** — owns root element, exposes `getElementAt(mx, my)` / `updateHover()` / `handleMouseDown()` / `handleMouseUp()` for aggregated-input model (no direct SDL_Event processing); `setRenderer()` called before `render()` each frame

### Game Layer — Combat System (`src/Game/`)

BG3-style turn-based tactical grid combat. See `《问道·山海》.md` §2 for full design spec.

- **`GridMap`** — square grid (8×6 default, 64px tiles). BFS `getMoveRange(gx, gy, movePoints)` for movement preview; BFS `getPath(from, to)` for AI pathfinding; `getTilesInRange()` for skill targeting. Occupancy tracked per cell via unit ID.
- **`CombatUnit`** — stats (HP/Attack/Defense/Speed), dual resources (灵力 spirit power / 身法 movement points), five-element type (`Element::Jin/Mu/Shui/Huo/Tu`), `std::vector<Skill>`, team ID (0=player, 1=enemy). `calculateDamage()` applies element counter (克制 ×1.3, 被克 ×0.7) and defense reduction.
- **`Skill`** — data struct: spirit cost, range (Manhattan), area type (`Single`/`Cross`/`Square`), base damage, cooldown. Five preset factory methods (`makeFireball`, `makeSwordSlash`, etc.).
- **`TurnManager`** — initiative queue sorted by Speed desc. Phases: `PlayerTurn → EnemyTurn → Animating → Victory/Defeat`. `endTurn()` reduces cooldowns, restores 2 spirit power, refills movement points.
- **`CombatAI`** — stepped decision: find nearest player unit → BFS path toward it → if in range, use highest-damage affordable skill → end turn. Small delay timer between steps for readability.
- **`CombatScene`** — owns all of the above + `Camera2D` + `UIManager`. Player input: left-click blue cells to move → click skill button in right panel → click red-highlighted enemy to attack → click "结束回合". Enemy turns auto-execute via AI.

## Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Class | PascalCase | `EventHandler`, `GameLoop` |
| Member variable | m + PascalCase | `mRenderer`, `mCurrentKeys` |
| Method | camelCase | `loadFromFile()`, `isKeyJustPressed()` |
| Parameter / local | camelCase | `fadeOutMs`, `keyStates` |
| Enum class value | PascalCase | `RenderQuality::Blended` |
| Include guard | UPPER_H | `RENDERER_H` |
| File name | PascalCase | `Window.h`, `Window.cpp` |

Comments are written in Chinese for readability by the target audience (Chinese indie game developers new to C++).

## Key SDL3 Differences from SDL2

This project uses **SDL3**, not SDL2. Common traps when writing code:
- `SDL_CreateWindow()` now takes flags as the 4th parameter (no separate height/flags split change, but API is different)
- `SDL_RenderTexture()` replaces `SDL_RenderCopy()` / `SDL_RenderCopyEx()`
- Rendering uses `SDL_FRect` (float) not `SDL_Rect` (int)
- `SDL_DestroySurface()` replaces `SDL_FreeSurface()`
- `SDL_EVENT_MOUSE_WHEEL` not `SDL_MOUSEWHEEL`
- `TTF_OpenFont()` takes `float ptsize` not `int`
- `TTF_RenderText_Blended()` takes `size_t length` parameter
- SDL3_mixer uses `MIX_Mixer*`/`MIX_Audio*`/`MIX_Track*` — completely different from SDL2_mixer's channel-based `Mix_Chunk*`/`Mix_Music*`

## Development Roadmap (from design doc)

1. **道基 (Dao Foundation)** — Core engine, grid map, movement ✅
2. **初识斗法** — Full combat loop, skills, enemy AI, UI framework ← current
3. **机缘首获** — Loot drops, inventory, affix system, first build
4. **山海初现** — Beast capture/raising, pet AI, pet-linked affixes
5. **洞府探秘** — Multi-level dungeons, five-element terrain, sects, crafting
6. **大道万千** — Endgame, weapon spirits, seasonal content

The design document calls for data-driven architecture: all equipment, beasts, skills, and affixes should be JSON-defined via cJSON, not hardcoded. The affix system follows `AffixEffect` base class + factory pattern with event-driven triggers (`ON_KILL`, `ON_ATTACK`, etc.).
