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

# Run (works from any directory — paths resolve relative to exe)
cd build && ./MyGame.exe
```

No test suite exists yet. No linter is configured.

**Build environment note**: `CMakePresets.json` hardcodes Scoop-install paths for vcpkg (`C:/Scoop/apps/vcpkg/current/`) and MinGW (`C:/Scoop/apps/mingw-winlibs/current/`). If building on a different machine, update these paths or override via CMake cache variables.

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

### Application Entry Point (`main.cpp`)

Global objects are constructed at file scope in `main.cpp` — `gWindow`, `gRenderer`, `gEventHandler`, `gGameLoop`, `gSceneManager`, `gFont`, `gPlayerData`. GameLoop callbacks (`onInit`, `onUpdate`, `onRender`, `onCleanup`) are free functions that access these globals directly.

`JsonLoader::initBasePath()` is called first in `onInit()` — it probes the exe directory and parent directory for `data/`, enabling the game to run from any working directory. `DataManager::loadAll()` follows. If either fails, the game exits immediately.

Scene transition `MainMenuScene → BigWorldScene` is handled in `onUpdate()` by checking `active->mName == "主菜单"` and reading `shouldStart()`/`shouldQuit()` flags. This is the only place where scene type is checked by name — all other transitions use `pushScene`/`requestPopScene`.

A Windows SEH crash handler (`SetUnhandledExceptionFilter`) logs exception type and address to `game.log` before terminating.

**Font loading** uses a fallback chain: `fontPrimary` (from config.json) → `fontFallback` (from config.json) → `C:/Windows/Fonts/msyh.ttc` → `C:/Windows/Fonts/simsun.ttc`.

**No `assets/` directory exists yet.** All terrain, units, and UI are rendered using colored rectangles and text — no sprites or textures are loaded.

**Test data** is initialized in `onInit()`: player gets 飞剑·青萍 lifebound treasure, 5 talismans equipped, and joins 剑宗 sect — for development/testing only.

### Layered Module Design

```
main.cpp (application layer — owns global objects, callbacks wired to GameLoop)
  │
├─ Game layer (src/Game/) — scenes, combat, world, player state
│   ├─ Core/ — pure data structs: PlayerData, Skill, Item, LifeboundTreasure, Talisman
│   ├─ Data/ — DataManager singleton, loads all JSON at startup, O(1) lookup by ID
│   ├─ Combat/ — GridMap, CombatUnit, TurnManager, CombatAI, CombatScene
│   ├─ World/ — WorldMap (40×30 terrain grid), BigWorldScene (exploration)
│   └─ Menu/ — MainMenuScene, SectScene (门派)
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

**Dependency rule**: Game layer depends on Core. Core modules depend on SDL3. `Renderer` holds a `Window&`. `Texture`/`Font` need a `Renderer&`. `UIManager` takes a `Renderer*` (set before render — constructed before Renderer exists). `Audio` and `Log` are standalone.

**Include paths**: CMake adds `src/` as an include directory. Files in `src/Core/` include each other directly (`"Renderer.h"`). Files in Game subdirectories use `../` for cross-directory same-layer includes (e.g. `#include "../Core/PlayerData.h"`) or `Core/` prefixes for Core layer. The umbrella header `Core/Core.h` includes all Core layer headers.

**Resource ownership**: Each class owns its SDL pointer via RAII. Copy is deleted; move is supported. Destruction order: Renderer before Window, Font before TTF_Quit, UIManager before Font.

### Core Module API Summary

| Module | Wraps | Key pattern |
|--------|-------|-------------|
| `Window` | `SDL_Window*` | create()/destroy() lifecycle; caches title/size |
| `Renderer` | `SDL_Renderer*` | Bound to Window ref; drawTexture overloads (position/scale/clip/rotate); logical size with letterbox; fillRect/drawRect/drawLine |
| `Texture` | `SDL_Texture*` | loadFromFile/loadFromMemory/createBlank/createFromSurface; color mod/alpha/blend |
| `Audio` | `MIX_Mixer*` + `MIX_Audio*` map | Name-based audio lookup; playWithTag for group control; master + per-tag gain |
| `Font` | `TTF_Font*` | Three quality levels (Solid/Shaded/Blended); renderText returns `SDL_Texture*` (caller must destroy); measureText for layout |
| `EventHandler` | Keyboard/mouse state maps | Dual-frame state (current + previous) for isKeyJustPressed/isKeyJustReleased; must call update() each frame end |
| `GameLoop` | Callback functions | std::function callbacks; deltaTime capped at 0.1s; FPS tracking |
| `JsonLoader` | cJSON | `initBasePath()` probes exe/parent for `data/` (call once before loading); `loadJsonFile(path)` tries exe-relative then CWD-relative; `freeJson()` wraps cJSON_Delete |
| `Log` | (pure C++) | Singleton with LOG_DEBUG/INFO/WARNING/ERROR/FATAL macros; auto-captures `__FILE__`/`__LINE__`/`__FUNCTION__`; console + file output; thread-safe via `std::mutex` |
| `Scene` | Scene stack | pushScene/popScene/switchScene; onEnter/onExit/onUpdate/onRender; `requestPopScene()` defers pop to after onUpdate() to avoid use-after-free |
| `Camera2D` | Viewport transform | worldToScreen/screenToWorld; exponential-decay smooth follow; bounds clamping; zoom 0.1–10 |

### UI System (`src/Core/UI/`)

Screen-space widget hierarchy. Coordinates are pixels, independent of Camera2D.

- **`UIElement`** — base: `mX/mY/mWidth/mHeight`, parent-relative position, `getWorldPosition()` walks up to root, `hitTest()` finds deepest element at point, `mVisible`/`mEnabled` flags
- **`UIPanel`** — filled rectangle with configurable RGBA background, child container
- **`UILabel`** — renders text via Font; auto-sizes if width/height left at 0; recreates SDL_Texture per frame
- **`UIButton`** — 3 visual states (Normal/Hover/Pressed) with configurable colors per state; `mOnClick` callback fires on mouse-up within bounds
- **`UIProgressBar`** — dual-layer bar; ratio from `mValue/mMaxValue`; optional centered text label
- **`UIManager`** — owns root element, constructed with `Font*`; input routed via `updateHover(mx, my)` / `handleMouseDown(mx, my)` / `handleMouseUp(mx, my)`; `setRenderer()` called before `render()`; `resetInteractionState()` must be called before destroying elements that may be hovered/pressed

### Scene Flow

```
MainMenuScene → switchScene → BigWorldScene
                                   │
                    pushScene ─────┼───── pushScene (T key, if in a sect)
                        ↓          │          ↓
                   CombatScene    SectScene (门派)
                        │          │
               requestPopScene ───┘── requestPopScene / ESC
```

All scenes except `MainMenuScene` receive `PlayerData*` and `SceneManager*` in their constructors. Constructor pattern: `(EventHandler&, Font*, float vpW, float vpH, PlayerData*, SceneManager*)`.

### Game Layer Data Structures

**PlayerData** (`src/Game/Core/PlayerData.h`): Plain struct — no destructor, no .cpp.
- **Core stats**: name, level, xp, hp/maxHP, attack, defense, speed, spiritPower/maxSpiritPower, movementPoints/maxMovementPoints, element (Element enum)
- **Skills**: `std::vector<Skill> skills`
- **Inventory**: `std::vector<Item> inventory` (max 20)
- **Equipment slots**: `Item* weapon/armor/accessory` — raw owning pointers, allocated with `new`, freed via `unequipSlot()` or re-equip
- **Lifebound treasure**: `LifeboundTreasure* lifeboundTreasure` — unique, non-removable, bound once via `bindLifeboundTreasure()`
- **Talismans**: `std::vector<Talisman> talismanInventory` (max 20), `std::vector<Talisman> equippedTalismans` (max 5 per battle)
- **Sect**: `std::string sectId`, `std::string sectName`
- **Key methods**: `getEffectiveAttack()`, `getEffectiveDefense()`, `getEffectiveMaxHP()`, `getEffectiveSpeed()`, `bindLifeboundTreasure()`, `addTalisman()`, `removeTalisman()`, `addItem()`, `removeItem()`, `equipItem()`, `unequipSlot()`, `heal()`, `restoreSP()`

**Item** (`src/Game/Core/Item.h`): `ItemType` enum (Consumable/Equipment/Material/Key), `EquipSlot` enum (Weapon/Armor/Accessory). Fields: id, name, desc, type, quantity, maxStack, equipSlot, atkBonus/defBonus/hpBonus, healHP/healSP.

**Skill** (`src/Game/Core/Skill.h`): Fields: mName, mDescription, mSpiritCost, mRange, mAreaType (Single/Cross/Square), mBaseDamage, mCooldown, mCurrentCooldown, mElementId. `isReady()` checks cooldown.

**LifeboundTreasure** (`src/Game/Core/LifeboundTreasure.h`): id, name, desc, element, level, maxLevel, atkBonus/defBonus/hpBonus/speedBonus, talentPath (ShaFa/ShouHu/XiaoYao). `getScaled*()` methods return base + (level-1) × scaling. Bound once — `bindLifeboundTreasure()` returns false if already bound. Displayed in character panel.

**Talisman** (`src/Game/Core/Talisman.h`): id, name, desc, type (Attack/Defense/Support/Summon), value, element, quantity. Used in CombatScene talisman panel — attack targets nearest enemy, support heals or grants movement. Consumed on use.

**SectDef / NPCInfo** (in `DataManager.h`): Sect data from `data/sects.json`. 4 sects (剑宗/丹宗/符宗/御灵宗), each with stat bonuses and 3 NPCs (name/title/greeting). `SectScene` renders NPC list + detail panel. NPC interaction is stubbed — clicking shows greeting only.

### DataManager (`src/Game/Data/`)

Singleton. `JsonLoader::initBasePath()` must be called before `loadAll()` — it resolves the `data/` directory relative to the exe. `loadAll()` loads 8 JSON files in order:
1. `data/skills.json` (5 skills)
2. `data/items.json` (7 items + starting inventory/skills)
3. `data/enemies.json` (8 enemy templates)
4. `data/config.json` (window, viewport, FPS, font, world/combat dimensions, player defaults)
5. `data/terrains.json` (6 terrain types)
6. `data/lifebound_treasures.json` (4 treasure templates)
7. `data/talismans.json` (6 talisman templates)
8. `data/sects.json` (4 sects with NPCs)

Query API: `getSkill(id)`, `getItem(id)`, `createItem(id)`, `getEnemyTemplate(id)`, `getTerrainDef(id)`, `getLifeboundTreasure(id)`, `createLifeboundTreasure(id)`, `getTalisman(id)`, `createTalisman(id)`, `getSect(id)`, `getConfig()`, `getStartingSkills()`, `createStartingInventory()`.

Enum parsing helpers (`parseElement`, `parseAreaType`, etc.) use first-character switch statements. Adding new data: edit JSON → add `load*()` method → add call in `loadAll()`.

### BigWorldScene (`src/Game/World/`)

Exploration on a 40×30 tile WorldMap. Player movement via WASD/arrow keys with per-axis collision detection against walkable tiles. Camera2D follows player. Encounter markers (5 randomly placed on walkable tiles) trigger `pushScene(CombatScene)` when approached.

**Key bindings**: `B` = inventory overlay, `C` = character panel (includes lifebound treasure display), `T` = town portal to SectScene (if `PlayerData::sectId` is set).

Both inventory and character panel are UIManager-based overlays. Toggling visibility requires calling `updateInventoryDisplay()`/`updateCharPanelDisplay()` to sync the backdrop element's `mVisible` flag.

### CombatScene (`src/Game/Combat/`)

8×6 grid, 64px tiles. Player controls unit selection, movement (blue cells), skill selection (right panel buttons), and attack targeting (red-highlighted enemies). Bottom bar has "结束回合" button. Talisman panel below skill panel.

Skill buttons are rebuilt only when `activeUnitId`, `phase`, or skill count changes — avoids destroying elements while UIManager holds raw pointers. Cosmetic updates (highlight color) mutate existing buttons in-place.

On victory: XP and loot granted to PlayerData. Surviving player HP/SP written back. `requestPopScene()` returns to BigWorldScene.

### SectScene (`src/Game/Menu/`)

Displays sect name, description, NPC list (left panel, clickable buttons), NPC detail (right panel: name/title/greeting), stat bonuses line at bottom. "下山历练" button at bottom calls `requestPopScene()` to return to BigWorldScene. ESC also returns. NPC functionality is stubbed — interaction beyond showing greeting is not implemented.

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

- `SDL_CreateWindow()` takes flags as the 4th parameter (no separate width/height split)
- `SDL_RenderTexture()` replaces `SDL_RenderCopy()` / `SDL_RenderCopyEx()`
- Rendering uses `SDL_FRect` (float) not `SDL_Rect` (int)
- `SDL_DestroySurface()` replaces `SDL_FreeSurface()`
- `SDL_EVENT_MOUSE_WHEEL` not `SDL_MOUSEWHEEL`
- `TTF_OpenFont()` takes `float ptsize` not `int`
- `TTF_RenderText_Blended()` takes `size_t length` parameter
- SDL3_mixer uses `MIX_Mixer*`/`MIX_Audio*`/`MIX_Track*` (NOT `Mix_Chunk*`/`Mix_Music*`)

## Development Roadmap

1. **道基 (Dao Foundation)** — Core engine, grid map, movement ✅
2. **初识斗法** — Full combat loop, skills, enemy AI, UI framework ← current
3. **机缘首获** — Loot drops, inventory, affix system, first build
4. **山海初现** — Beast capture/raising, pet AI, pet-linked affixes
5. **洞府探秘** — Multi-level dungeons, five-element terrain, sects, crafting
6. **大道万千** — Endgame, weapon spirits, seasonal content

## Known Pitfalls

### UIManager raw pointers and element destruction

`UIManager` stores raw pointers `mPressedElement` and `mHoveredElement`. Destroying a UI element while a mouse button is down on it causes a crash on the next `handleMouseUp()`. Before destroying elements:
1. Call `mUIManager.resetInteractionState()` to null out raw pointers
2. Use change-tracking to only rebuild when structural properties change
3. For cosmetic updates, mutate existing elements in-place without destroying them

### Scene self-removal during onUpdate

Use `SceneManager::requestPopScene()` (not `popScene()`) to exit a scene from within its own `onUpdate()`. The pop is deferred until after `onUpdate()` returns.

### Overlay visibility

When toggling overlay state (`mInventoryOpen`/`mCharPanelOpen`), call the corresponding `update*Display()` method to sync the backdrop element's `mVisible` flag.

### PlayerData owns heap-allocated equipment pointers

`weapon`, `armor`, `accessory`, and `lifeboundTreasure` are raw pointers allocated with `new`. PlayerData has no destructor — caller must ensure cleanup before destruction. Known debt for smart-pointer migration.

### EquipItem memory leak

When re-equipping an occupied slot, the old heap Item is overwritten without `delete`. The old item is copied to inventory via `addItem()` but the original heap pointer is leaked. Prefer `unequipSlot()` first.

### Adding new game data

Edit the corresponding JSON file in `data/` → add a `load*()` method in DataManager following existing cJSON traversal patterns → add call in `loadAll()`. No code generation — all parsing is handwritten.
