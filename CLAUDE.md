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
GameLoop (loop controller — calls OnInit/OnUpdate/OnRender/OnCleanup)
  │
Core modules (src/Core/) — each wraps one SDL3 subsystem:
  Window ←─ Renderer ←─ Texture
                ↑          Font renders to SDL_Texture, handed to Renderer
  EventHandler ←─ GameLoop
  Audio (independent — Mixer/Audio/Track with tag-based grouping)
```

**Dependency rule**: higher-level modules depend on lower-level ones. `Renderer` holds a `Window&` reference. `Texture`/`Font` need a `Renderer&` to load/render. `Audio` is standalone. `GameLoop` holds an `EventHandler&`.

**Resource ownership**: Each class owns its SDL pointer via RAII. Copy is deleted; move is supported. Destruction order matters: Renderer before Window, Font before TTF_Quit.

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

1. **道基 (Dao Foundation)** — Core engine, grid map, movement ← current
2. **初识斗法** — Full combat loop, skills, enemy AI, ImGui HUD
3. **机缘首获** — Loot drops, inventory, affix system, first build
4. **山海初现** — Beast capture/raising, pet AI, pet-linked affixes
5. **洞府探秘** — Multi-level dungeons, five-element terrain, sects, crafting
6. **大道万千** — Endgame, weapon spirits, seasonal content

The design document calls for data-driven architecture: all equipment, beasts, skills, and affixes should be JSON-defined via cJSON, not hardcoded. The affix system follows `AffixEffect` base class + factory pattern with event-driven triggers (`ON_KILL`, `ON_ATTACK`, etc.).
