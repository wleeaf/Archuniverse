# Archuniverse

A modern C++ RPG combat engine. Real-time, tick-based, event-driven, and engine
agnostic: no Unity, no framework, just a clean library you can drive from a
console demo today and embed in a renderer later.

This is a ground-up rewrite of an earlier C# prototype. Same spirit (living
entities, real-time combat, items, skills, progression), rebuilt around value
semantics, `std::expected`, a modifier-based stat system, and a decoupled event
bus.

## Requirements

- A C++23 compiler (developed with GCC 13.3)
- CMake 3.28+

## Build and run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

./build/archuniverse_demo          # the Izroth vs Lila duel
./build/archuniverse_net_demo      # a client mirrors a server world over TCP
ctest --test-dir build             # run the unit tests
```

The engine builds warning-clean under `-Wall -Wextra -Wpedantic -Wshadow
-Wconversion -Wsign-conversion`.

## Architecture

The engine is a static library (`archuniverse`) plus a demo executable and a
doctest suite. Everything lives under `namespace arch`.

```
include/archuniverse/
  core/         result, ids, rng, vec3, stat (Attribute/Vital), event bus,
                tickable, fixed-timestep game loop, context, events
  entities/     LivingEntity, Character, Monster, StatusEffect
  items/        Item + Weapon/Armor/Potion/Food/Ware, Effect, special effects
  skills/       Skill, SkillTree
  combat/       Combat, CombatManager, BleedEffect
  ai/           Behavior, IdleBehavior/WanderBehavior/HunterBehavior, AiSystem
  persistence/  save/load snapshot (tab-separated text)
  net/          Channel, Loopback + TCP transports, protocol, GameServer/GameClient
  world/        World (owns everything; the one injectable context)
src/            implementations mirroring the headers
apps/demo/      the combat demo
apps/net_demo/  a TCP client/server replication demo
tests/          doctest unit tests
third_party/    vendored doctest single header
```

### Load-bearing ideas

- **`Attribute` + `Vital` with removable modifiers** (`core/stat.hpp`). Health,
  attack, regen, and so on are not raw ints. A value is `(base + flats) * (1 +
  percents/100)`, and every buff/gear bonus/skill upgrade is a modifier with a
  handle. Removing a bonus is exact, so swapping gear or expiring a buff can
  never leak stat bonuses (a real bug in the original).

- **`std::expected`-based results** (`core/result.hpp`). Fallible operations
  return `Result<T>` / `Status` with a `GameError`, instead of an ad-hoc enum
  returned by convention.

- **Event bus** (`core/event_bus.hpp`). Combat publishes `DamageDealt`,
  `EntityDied`, `LeveledUp`, `CombatEnded`, and friends. Logging, XP rewards,
  and UI subscribe. The simulation prints nothing itself.

- **Fixed-timestep game loop** (`core/game_loop.hpp`). Deterministic
  `run_for(seconds)` for tests/headless runs and a real-time `run()` with an
  accumulator. Registration is safe mid-tick (queued adds, tombstoned removes),
  so an entity dying and unregistering itself never invalidates iteration.

- **Status effects owned by entities** (`entities/status_effect.hpp`). Bleed and
  timed buffs are carried and advanced by the entity, so their lifetime is tied
  to their host instead of floating free on a global loop.

- **One `World`, no singletons** (`world/world.hpp`). The World owns the loop,
  bus, rng, all entities, all items, the combat manager, and the AI system, and
  hands each object a `Context` of references. This replaces the old scattered
  `GameLoop` / `SoulLedger` / `ItemManager` singletons.

- **Spatial world** (`core/vec3.hpp`, `world/world.hpp`). Entities carry a
  position and velocity; the World answers range and nearest-neighbor queries
  (`entities_within`, `nearest`) used by the AI.

- **Monster AI** (`ai/`). A `Behavior` is invoked per tick by the `AiSystem`
  with full World access. Ships with idle, wander (random walk around a home),
  and hunter (chase the nearest character and open combat in range) behaviors.

- **Persistence** (`persistence/snapshot.hpp`). A tab-separated text save that
  round-trips entities, items, progression, equipment, and positions. It
  re-equips gear and re-unlocks skills on load, so modifier-based bonuses are
  replayed rather than double-counted. (Transient status effects and code-
  attached enchantments are not persisted yet.)

- **Networking** (`net/`). A transport-agnostic `Channel` with an in-process
  `LoopbackPair` (for tests and single-process play) and a real length-framed
  POSIX `TcpChannel`/`TcpListener`. `GameServer` applies client commands
  (`STEP`, `START_COMBAT`, `SNAPSHOT_REQUEST`) to the authoritative World;
  `GameClient` issues commands and loads replicated snapshots into a mirror
  World. State replication reuses the persistence format.

## What changed from the C# prototype

- Modifier-based stats fix the equipment-swap bonus leak by construction.
- Correct resource/error reporting (the old code returned "insufficient mana"
  when stamina ran out).
- Correct defender resource costs in combat resolution.
- Deterministic, seedable simulation (`World{seed}`) so fights and tests are
  reproducible.
- Presentation fully decoupled from simulation via the event bus.

## Status

The combat core, items, skills, progression, world, spatial queries, monster
AI, save/load persistence, and networking (loopback + TCP) are in place and
tested. Scripting, a proper spatial index for large worlds, richer AI, and
client-side prediction are future work.
