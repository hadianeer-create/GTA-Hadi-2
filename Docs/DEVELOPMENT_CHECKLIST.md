# Development checklist

Legend: ✅ done and verified in this repo · 🟡 partially done / first version · ⬜ not started.
"Verified" means covered by the core unit tests, the browser preview or the Unreal-layer stub compile
check (see README › Verification). Everything in the Unreal layer still needs the in-engine smoke test below
on a machine with UE 5.6.

## Phase 1 — Project & engine setup
- ✅ UE5 C++ project (`HadiViceCity.uproject`, game/editor targets, module build rules)
- ✅ Lumen GI + reflections, Virtual Shadow Maps, TSR, Nanite enabled, DX12 / SM6, no baked lighting
- ✅ Hardware ray tracing compiled in, runtime toggle (Lumen HWRT)
- ✅ Enhanced Input as default input classes; all actions created in code
- ✅ Python editor bootstrap that generates master materials (`Content/Python`)
- ✅ Packaging: always-cook `/Game/Hadi` + engine primitives; IoStore; Entry map cooked
- ✅ Git: `.gitignore`, LFS attributes for binary assets
- ✅ Folder structure & architecture doc

## Phase 2 — Hadi
- ✅ Procedural articulated body (18 joints, 60+ parts) with planted-feet IK-style pelvis solve
- ✅ Walk / jog / sprint gait blending, idle breathing & weight shift, turn lean, head look-at
- ✅ Jump / fall, swim (tread + crawl), drive (seated), climb/mantle, collapsed poses
- ✅ Facial expressions (neutral/happy/angry/surprised/sad/focused), blinking, talking mouth
- ✅ 6 outfits with jackets, sleeves, shorts, caps, sunglasses; wardrobe & clothing store
- ✅ Smooth third-person camera (lag, collision, zoom, sprint FOV kick, swim offset)
- ⬜ High-detail skeletal mesh + mocap animation set (Blender) — slot documented in ARCHITECTURE.md
- ⬜ Hand IK on the steering wheel, door enter/exit animations

## Phase 3 — First district (Coral Key)
- ✅ Island landmass, seawalls, beach with sloped surf, ocean & sea floor
- ✅ 5 avenues + 10 streets, lane markings, crosswalks; connected road graph
- ✅ 180 buildings across 6 styles: deco hotels, shops, nightclubs, offices, villas, towers
- ✅ Parks with fountains, parking lot, promenades, marina piers & yachts, lifeguard towers, umbrellas
- ✅ ~200 street lamps, neon signs, palms; causeway to the mainland with downtown skyline
- ✅ District & street names for the HUD
- 🟡 Distance streaming (200 m cells, visual) — memory streaming arrives in Phase 7
- ⬜ Building interiors (Coral Cafe, Tide & Thread, Hadi's apartment)

## Phase 4 — Movement, camera & interaction
- ✅ Walk toggle, sprint with stamina, jump, mantle onto ledges / out of water
- ✅ Swimming with buoyancy and shallow-water exit
- ✅ Interaction system (interface + registry, facing-weighted selection, prompts)
- ✅ Fall damage, health regen to 60%, blackout & respawn with clinic bill
- ✅ Main / pause / settings / stats / controls menus with mouse, keyboard, gamepad

## Phase 5 — Vehicles
- ✅ Vento GT: ray-cast suspension, tyre friction circle, anti-roll, RWD engine curve, brakes, reverse,
  handbrake drift, drag & downforce, self-righting, headlights, chase + interior cameras, enter/exit
- ⬜ More classes: supercar, SUV, motorcycle, truck, boat (buoyancy), helicopter, plane
- ⬜ Damage model (deformation, smoke, fire), customization garage, vehicle storage
- ⬜ Weather-dependent grip (hook exists: `AHadiCityGenerator::SetVehicleGrip`)

## Phase 6 — Traffic & NPC AI
- ⬜ Pedestrians with daily schedules (home → work → shops → nightlife), reactions, conversations
- ⬜ Traffic on the road graph: lanes, intersections with working signals, varied driver personalities
- ⬜ Navmesh generation for the procedural city

## Phase 7 — Expand the open world
- ⬜ Downtown, Barrio Palmera, Port of Vice, Vice International Airport, Sawgrass Flats countryside,
  Coquina Ridge hills, Key Ibis island + bridges
- ⬜ Async per-cell generation & unloading; World Partition map for hand-authored landmarks
- ⬜ Pause-menu full map with waypoint placement

## Phase 8 — Missions & story
- 🟡 Objective system with events, targets, GPS, waypoint, rewards, autosave (prologue: 3 steps)
- ⬜ Mission scripting (fail states, checkpoints), cutscene sequencer, dialogue system with choices
- ⬜ Acts I–III per STORY_BIBLE.md, side missions, properties

## Phase 9 — Weather, lighting, audio & UI
- ✅ 24-hour day/night, sun/moon, sky atmosphere, volumetric clouds, fog, exposure, night emissives
- ⬜ Weather states (sunny, cloudy, rain, thunderstorm, fog) affecting grip, visibility, NPCs
- ⬜ MetaSounds: footsteps, engines, ambience, weather, radio stations with original music
- ⬜ Smartphone UI, key rebinding (Enhanced Input user settings), audio settings

## Phase 10 — Optimize, test, polish
- ✅ Core unit tests, browser preview renderer, Unreal-layer stub compile check
- ⬜ Unreal automation tests (functional tests for spawn, save/load, vehicle)
- ⬜ Profiling pass (Unreal Insights), scalability tuning for 60 FPS on mid-range GPUs, packaging

## In-engine smoke test

Run after the first successful build (Development Editor, PIE, then Standalone):

1. Output Log shows `[Hadi] materials ready` (first editor launch) and
   `Vice City generated: 31387 primitives -> ... instances`.
2. Main menu appears over a slow aerial orbit; mouse hover + click and W/S + Enter both work.
3. **New Game** → Hadi stands outside the Seagrape Apartments; the "WELCOME HOME" banner shows; objective
   "Grab breakfast at the Coral Cafe" with a purple GPS line on the minimap and a yellow marker ahead.
4. Walk / jog / sprint (stamina bar drains), toggle walk (Ctrl), jump; climb onto a planter or the
   lifeguard-tower ramp; walk into the sea until Hadi swims; press Space at the marina wall to climb out.
5. At the cafe press **E** → buy food → health rises and the objective advances to the car.
6. **E** at the Vento GT → camera blends into the chase cam; drive (W/S/A/D), handbrake (Space),
   interior camera (V); exit below 30 km/h (E).
7. Drive to Marina Row → "PROLOGUE COMPLETE" banner, +$500, autosave notification.
8. Tide & Thread on Flamingo Avenue: buy and wear an outfit. Apartment door: Save, Sleep 6 h, Wardrobe.
9. Find a Neon Flamingo (e.g. on the deck of the aqua lifeguard tower on the beach, south of the apartment) → +$100 notification.
10. Pause (P) → Settings: change quality, toggle ray tracing / motion blur / FPS; Statistics shows distances.
11. Quit to Main Menu → **Continue** restores position, money, outfit and collectibles.
12. Let time run (1 game hour = 2 real minutes) or Sleep: windows, neon and street lights come on at night.
