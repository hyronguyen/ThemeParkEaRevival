# Gameplay recovery status

This map records original-rule recovery, separately from the user-authorized offline playtest. `ThemeParkGame.exe` now implements provisional placement, NPC walking/queue/ride states, capacities/timers, desktop UI and local progress saves. See `playtest_mechanics.md` for those rules. Native function bodies were sampled with Capstone; Android methods were indexed at DEX instruction boundaries. A user-supplied video is available, but no original runtime has been executed here and original-rule equivalence is untested.

| System | Evidence located | Current recovery and reuse decision |
|---|---|---|
| Main loop | RFLib::tick, PMStateManager::tick, StateInGame::update/updateGame/render | Direct update/render relationships identified. Scheduling and timing need deeper control-flow analysis; reimplement platform loop, translate verified rules later. |
| World/park state | GameMap::InitMap/loadRideSlots, MapData::loadLevel, Control::loadData, WorldInstance | Original static paths and slot footprints recovered and rendered diagnostically. Owned buildings, initial active park and service response schema remain incomplete. Reuse static data. |
| Placement | RideSlot::read/getSlotCenter3D; Android Build slot clicked and Ride/Shop Menu instructions | 247 original footprints. Original permissions/unlocks remain unresolved; playtest uses empty-slot fit and reachable-gate checks. |
| Ride creation | ThemeParkSoap::StartBuild constructs StartBuildJob; Android StartBuild/CompleteBuild methods | Client lifecycle/service boundaries found; cannot treat a local spawn as equivalent. Requires understood data/configuration and independent offline state. |
| Guests | Guest::update/calculateNewTarget, PersonBase/AnimSetManager/PeopleModel | Original model references in walkAnim2 recovered; models 0–7 used. Provisional movement, FIFO queues, batches and returns are implemented separately; original needs/weights and passenger attachments remain unresolved. |
| Pathfinding | AStar::FindPath/ValidateNode and symbol-spelled `huristic` | A-star implementation candidate confirmed. Weights, constraints and tie-breaking not translated. Static path topology is not yet a validated navigation graph. |
| Economy | AttractionStruct::getUpgradeCost; ThemeParkSoap::ModifyCash/ModifyXP; Android entity parser fields | Cost/reward methods and service jobs located. No recovered complete price table or proof of local authority. No invented/free-currency substitute. |
| Attraction management | Attraction/RideStruct, upgrade and collection-related Android methods | Types and lifecycle candidates located; full operating timers/capacities/maintenance behavior unresolved. |
| Progression | Quests::processQuests call; Android unlock/park-rank/XP fields | Quest/progression paths located. Original unlock schedules, defaults and remote updates unresolved. |
| Save/load | PMProfileStorage::save/load plus PMBinaryReader/Writer; LoadData/WorldInstance XML parsing | Original park save/cloud schema remains unknown. The playtest's independent TPPLAY2 schema preserves simulation state with transactional validation and atomic file replacement. |
| Interaction | Touch/Control; Android onTouchEvent bridge | Platform bridges located. Preview mouse/keyboard uses its own inspected-data controls; not faithful original gestures/UI flow. |
| Networking/authentication | ThemeParkSoap jobs, Origin/Mayhem/Nimble, billing/download integrations | Strong service dependency evidence. No live services contacted. Full fallback/error/identity behavior remains unknown. |

The most useful next semantic work is to recover one complete attraction instance from LoadData/WorldInstance through placement/rendering, compare it to Android entity-parser behavior, then create fixtures for its original data and rules. The same approach should be used for an actual model/animation before generalizing a park engine. Decompiled output must be reviewed, not treated as drop-in portable source.
