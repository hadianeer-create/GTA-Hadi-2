# Architecture

## Two layers

```
+--------------------------------------------------------------------------------------+
| Unreal layer (Source/HadiViceCity/{Public,Private}/<Characters|Vehicles|World|...>)  |
|   Actors, components, input, rendering, physics, UI, saving.                         |
|                                    | uses                                            |
+------------------------------------v-------------------------------------------------+
| Simulation core (Source/HadiViceCity/{Public,Private}/Core, namespace hadi)          |
|   Pure C++17, no engine headers. City generator, road graph + A* GPS, Hadi's rig,    |
|   procedural animation solver, wardrobe, time-of-day curves, minimap projection.     |
|   Compiled by UnrealBuildTool as part of the module AND by Tools/CoreTests (g++).    |
+--------------------------------------------------------------------------------------+
```

Everything that can be expressed as data or math lives in the core. That keeps it testable
without the engine and lets the browser previewer render exactly what the game spawns.
`World/HadiCoreBridge.h` converts between core and Unreal types: `Vec3` to `FVector`, `Rot` to
`FRotator`, and sRGB `Color` to linear `FLinearColor`. Core units and axes match Unreal (cm,
X forward/north, Y right/east, Z up; rotations use `FRotator` semantics).

Core code rules: no Unreal macros, no `<random>` distributions (their output differs between
MSVC and libstdc++), and file-local helpers go in uniquely named namespaces (`citygen`,
`rigdata`) so unity builds never collide.

## Boot sequence

1. `DefaultEngine.ini` sets `GameDefaultMap=/Engine/Maps/Entry` and `GlobalDefaultGameMode=HadiGameMode`.
2. `AHadiGameMode::StartPlay` finds or spawns `AHadiEnvironmentManager` and `AHadiCityGenerator`,
   calls `Generate()`, and spawns the waypoint marker. If a designer places these actors in a
   real map, those instances are used.
3. `AHadiCityGenerator::Generate` runs `hadi::GenerateCity`, then:
   * groups primitives by streaming cell and by (shape, material, colour, collision, shadow, cull),
     producing one `UInstancedStaticMeshComponent` per group inside an `AHadiCityCell` actor;
   * creates pooled point lights for street lamps and neon spill;
   * spawns shops, the safehouse, collectibles and parked cars from POIs.
4. `AHadiPlayerController::BeginPlay` builds the Enhanced Input assets, starts the menu fly-over
   camera and shows the cursor. `AHadiHUD::BeginPlay` opens the main menu.
5. New Game or Continue calls `AHadiGameMode::StartNewGame` / `ContinueGame`, which spawns and
   possesses `AHadiCharacter`, applies the profile (outfit, collectibles) and switches to gameplay input.

## Main classes and where they live

| File | Responsibility |
|---|---|
| `Characters/HadiCharacter` | Movement speeds, stamina, swimming (buoyant `MOVE_Flying` below the sea surface), ledge mantling (3-trace detection plus a capsule room check), interaction focus, health and regen, fall damage, blackout, entering and leaving vehicles, camera. |
| `Characters/HadiRigComponent` | Builds one pivot per joint and one mesh per body part from `hadi::GetRigSegments()`. Every frame it feeds `hadi::AnimInput` to `UpdateAnimState` / `ComputePose`. |
| `Vehicles/HadiVehicle` | Physics box chassis, 4 ray-cast wheels (spring/damper, anti-roll, friction circle), engine, brakes, reverse, handbrake, drag, downforce, self-righting, cameras, headlights, exit-location search. |
| `World/HadiCityGenerator` | Spawns the city, distance-streams cells (with hysteresis) and reassigns the nearest street and neon lights to the light pools. |
| `World/HadiEnvironmentManager` | Sun, moon, sky atmosphere, real-time sky light, volumetric clouds, height fog and post-process. Runs the clock and drives the global night factor. |
| `World/HadiMaterialSubsystem` | Loads the master materials (falling back to engine materials), caches one material instance per (surface, colour), and re-lights emissive instances at night. |
| `Gameplay/HadiInteractable` | `IHadiInteractable` interface and a world-subsystem registry. The character asks it for the best target in front of it. |
| `Gameplay/HadiShop`, `HadiSafehouse`, `HadiCollectible` | Restaurant and clothing store, apartment door, Neon Flamingos, objective waypoint marker. |
| `Game/HadiGameMode` | Session flow, money, wardrobe purchases, collectibles, objective chain plus waypoint, blackout and respawn, save and load, autosave. |
| `Game/HadiGameInstance`, `HadiProfile`, `HadiSaveGame` | Persistent profile (money, outfits, collectibles, story step, statistics) and the save slot format. |
| `Game/HadiGameUserSettings` | Extra options on top of `UGameUserSettings` (motion blur, DOF, Lumen HWRT, FOV, sensitivity, invert, FPS). |
| `Player/HadiPlayerController` | Creates every `UInputAction` and the mapping context in code (keyboard, mouse, gamepad) and routes input to Hadi, the vehicle or the menus. |
| `UI/HadiHUD`, `UI/HadiMinimap` | The whole UI is canvas drawn: menu stack with mouse, keyboard and gamepad navigation, plus HUD widgets. The minimap renders into a `UCanvasRenderTarget2D`. |

## Rendering and performance choices

* **Instancing:** about 31k primitives collapse into a few hundred instanced draw batches. Small props use
  per-instance cull distances (`hadi::CullClass`): road paint ~300 m, facade details ~700 m,
  buildings ~2 km.
* **Streaming:** 200 m cells shown and hidden around the camera (950 m in / 1,100 m out hysteresis
  beyond each cell's edge). This visual streaming is the first step. Phase 7 moves generation per
  cell to async tasks and unloads memory, and authored content will move to a World Partition map.
* **Lights:** around 200 street lamps and around 40 neon signs share pools of 64 and 16 unshadowed point lights,
  reassigned to the nearest emitters every 0.4 s. Emissive materials make distant lamps glow.
* **Lumen + VSM + TSR**, with DX12/SM6, are configured in `DefaultEngine.ini`. Hardware ray tracing is compiled
  in and toggled at runtime through `r.Lumen.HardwareRayTracing` (Settings > Ray Tracing).
* **Nanite** is enabled for the project. The current primitives are too simple to benefit; Nanite starts
  paying off with the authored Blender/Megascans assets in Phase 7.

## Replacing placeholders with authored art (Blender / Substance)

The procedural pieces are real gameplay systems, not stand-ins. Each one has a clear slot for authored assets:

* **Hadi's body:** export a rigged mesh from Blender, then create a Blueprint child of `AHadiCharacter`,
  assign the skeletal mesh to `Mesh` and hide `Rig` (`SetRigHidden(true)`). Drive an Animation Blueprint with
  the same signals `UpdateAnimation` builds in `hadi::AnimInput` (speed, swimming, driving, climbing, expression).
* **Buildings:** add a new `Shape` (for example `Mesh` plus an asset id) to `hadi::Prim` and map it in
  `UHadiMaterialSubsystem::GetShapeMesh`, or place hand-built landmark Blueprints at POIs.
* **Vehicles:** subclass `AHadiVehicle`, hide the generated parts, attach the Blender body mesh. The physics
  is independent of the visuals.
* **Materials:** edit the generated `M_Hadi_*` masters in place (for example add Substance texture sets). The
  C++ only sets the named parameters.

## Adding to the city

`Source/HadiViceCity/Private/Core/HadiCity.cpp` is organised by district (`BeachfrontBlock`, `NeonBlock`,
`VillaBlock`, ...) and by building kit (`DecoHotel`, `Shop`, `Nightclub`, `Office`, `Villa`, `TowerBlock`,
`Palm`, `Lamp`, ...). After changing it, run `Tools/CoreTests/run_tests.sh` (the invariants catch
overlaps and unreachable points) and `Tools/Preview/build_preview.sh` to look at the result.
