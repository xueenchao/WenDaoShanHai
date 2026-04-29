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
main.cpp (application layer — owns global objects, callbacks wired to GameLoop)
  │
├─ Game layer (src/Game/) — scenes, combat, world, player state (gameplay logic)
│   ├─ DataManager — singleton, loads all JSON data at startup, provides O(1) lookup by ID
│   ├─ Scenes (Scene subclasses on the stack):
│   │   MainMenuScene → BigWorldScene ↔ CombatScene (push on encounter, pop on end)
│   ├─ PlayerData — persistent state shared across scenes via pointer (stats/skills/inventory/equipment)
│   ├─ Item — pure data struct (id/name/desc/type/stats/effects), loaded from JSON by DataManager
│   ├─ WorldMap — 40×30 tile terrain map (Grass/Water/Mountain/Forest/Path/Sand), walkable checks
│   └─ Combat — GridMap, CombatUnit, Skill (pure data), TurnManager, CombatAI
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
│   ├─ JsonLoader — cJSON file I/O utility (readFileToString + cJSON_Parse)
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
| `JsonLoader` | cJSON | `loadJsonFile(path)` reads file + parses with cJSON; `freeJson()` wraps cJSON_Delete. Returns nullptr on error with LOG_ERROR detail. |
| `Log` | (none — pure C++) | Singleton with LOG_DEBUG/INFO/WARNING/ERROR/FATAL macros; auto-captures `__FILE__`/`__LINE__`/`__FUNCTION__`; console + file output; `setMinLevel()` filtering; thread-safe via `std::mutex` |
| `Scene` | Scene stack | pushScene/popScene/switchScene; onEnter/onExit/onUpdate/onRender lifecycle; `requestPopScene()` defers pop to after onUpdate() to avoid use-after-free |
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

### Game Layer (`src/Game/`)

Full game flow: `MainMenuScene → BigWorldScene → (push) CombatScene → (pop) BigWorldScene`

**Scenes:**
- **`MainMenuScene`** — Start menu with direct rendering (no UIManager). Title + "开始游戏"/"退出游戏" buttons with manual hit-testing. Sets flags that `main.cpp` reads to transition.
- **`BigWorldScene`** — Overworld exploration. Free movement (WASD/arrow keys) with per-axis collision detection against `WorldMap` walkability. Camera2D follows player. Encounter markers trigger `pushScene(CombatScene)`. Inventory overlay (B key) and character panel overlay (C key) built with UIManager—toggled via `mVisible` on the backdrop element.
- **`CombatScene`** — Same as below, now accepts `PlayerData*` + `SceneManager*` for world integration. On victory: grants XP/loot to PlayerData. On battle end: calls `mSceneManager->requestPopScene()` to return to BigWorldScene.

**Data-driven architecture:**
- **`DataManager`** — Singleton (`DataManager::getInstance()`). `loadAll()` loads all JSON files from `data/` at startup (called in `main.cpp onInit()` before SDL init). Provides `getSkill(id)`, `getItem(id)`, `createItem(id)`, `getEnemyTemplate(id)`, `getTerrainDef(id)`, `getConfig()`. Also stores starting inventory/skills loaded from `items.json`. Enum parsing helpers (`parseElement`, `parseAreaType`, etc.) use first-character switch statements.
- **`data/skills.json`** — 5 skills (fireball, ice_shard, sword_slash, rock_smash, vine_whip)
- **`data/items.json`** — 7 items + `startingInventory` array + `startingSkills` array
- **`data/enemies.json`** — 8 enemy templates with element-to-skill mapping
- **`data/config.json`** — Window, viewport, FPS, font, world/combat grid dimensions, player defaults, encounter params, AI timing
- **`data/terrains.json`** — 6 terrain types with RGB colors and walkability

**Persistent state:**
- **`PlayerData`** — Plain struct (no .cpp). Name, level, XP, HP/SP/Attack/Defense/Speed, `Element`, `std::vector<Skill>`, `std::vector<Item>` inventory (max 20), `Item*` equipment pointers (weapon/armor/accessory, nullptr = empty). Helper methods: `addItem()`, `removeItem()`, `equipItem()`, `unequipSlot()`, `heal()`, `restoreSP()`, `getEffectiveAttack()` etc. (apply equipment bonuses).
- **`Item`** — `ItemType` enum (Consumable/Equipment/Material/Key), `EquipSlot` enum (Weapon/Armor/Accessory). `Item` struct is pure data (id/name/desc/type/quantity/maxStack/equip stats/consumable effects). All item templates are defined in JSON and loaded by DataManager — no hardcoded factory functions.

**World:**
- **`WorldMap`** — 40×30 tile grid, 64px tiles (2560×1920 world). `Terrain` enum (Grass/Water/Mountain/Forest/Path/Sand). `Tile` struct: terrain + walkable. `generateDemoMap()` creates hand-crafted terrain. Same coordinate transform API as GridMap (`worldToGrid`, `gridToWorld`, `gridToWorldCenter`).

**Combat (see `《问道·山海》.md` §2 for full design spec):**

- **`GridMap`** — square grid (8×6 default, 64px tiles). BFS `getMoveRange(gx, gy, movePoints)` for movement preview; BFS `getPath(from, to)` for AI pathfinding; `getTilesInRange()` for skill targeting. Occupancy tracked per cell via unit ID.
- **`CombatUnit`** — stats (HP/Attack/Defense/Speed), dual resources (灵力 spirit power / 身法 movement points), five-element type (`Element::Jin/Mu/Shui/Huo/Tu`), `std::vector<Skill>`, team ID (0=player, 1=enemy). `calculateDamage()` applies element counter (克制 ×1.3, 被克 ×0.7) and defense reduction.
- **`Skill`** — Pure data struct: spirit cost, range (Manhattan), area type (`Single`/`Cross`/`Square`), base damage, cooldown, element. All skill definitions loaded from `data/skills.json` by DataManager. `isReady()` checks cooldown. No factory methods — skills are copied from DataManager templates.
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

Phase 1 (data-driven architecture) is complete — all skills, items, enemies, terrain, and config are loaded from JSON via `DataManager`. The upcoming affix system follows `AffixEffect` base class + factory pattern with event-driven triggers (`ON_KILL`, `ON_ATTACK`, etc.).

## Known Pitfalls

### Destroying UI elements that UIManager holds references to

`UIManager` stores raw pointers `mPressedElement` and `mHoveredElement`. If you destroy a UI element (e.g., via `pop_back()` on children) while a mouse button is down on it, the next call to `handleMouseUp()` dereferences a dangling pointer → **crash**.

**The pattern to avoid** (this caused a real crash in CombatScene):
```cpp
// BAD: destroy and recreate buttons every frame
void updateHUD() {
    while (mSkillPanel->mChildren.size() > 1)
        mSkillPanel->mChildren.pop_back();  // destroys buttons — mPressedElement dangles!
    mSkillButtons.clear();
    // ... recreate buttons ...
}
```

**The fix**: Use change-tracking to only rebuild when structural properties change (active unit ID, phase, skill count). Before destroying, call `mUIManager.resetInteractionState()` to null out the raw pointers. For cosmetic updates (highlight color), mutate existing buttons in-place without destroying them.

### Scene self-removal during onUpdate

A scene must never call `popScene()` on itself during its own `onUpdate()` — it would destroy `this` while still executing. Use `SceneManager::requestPopScene()` instead, which sets a flag. `SceneManager::onUpdate()` checks this flag after `active->onUpdate()` returns and performs the actual pop safely.

### Overlay visibility requires explicit update calls

When toggling overlay state (`mInventoryOpen`/`mCharPanelOpen`), you must call `updateInventoryDisplay()`/`updateCharPanelDisplay()` to sync the backdrop element's `mVisible` flag. Setting the bool alone leaves the UI rendering stale.
