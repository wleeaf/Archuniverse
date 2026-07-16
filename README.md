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
ctest --test-dir build             # run the unit tests
```

The engine builds warning-clean under `-Wall -Wextra -Wpedantic -Wshadow
-Wconversion -Wsign-conversion`.

## Architecture

The engine is a static library (`archuniverse`) plus a demo executable and a
doctest suite. Everything lives under `namespace arch`.

```
include/archuniverse/
  core/      result, ids, rng, stat (Attribute/Vital), event bus,
             tickable, fixed-timestep game loop, context, events
  entities/  LivingEntity, Character, Monster, StatusEffect
  items/     Item + Weapon/Armor/Potion/Food/Ware, Effect, special effects
  skills/    Skill, SkillTree
  combat/    Combat, CombatManager, BleedEffect
  world/     World (owns everything; the one injectable context)
src/         implementations mirroring the headers
apps/demo/   the runnable demo
tests/       doctest unit tests
third_party/ vendored doctest single header
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
  bus, rng, all entities, and all items, and hands each object a `Context` of
  references. This replaces the old scattered `GameLoop` / `SoulLedger` /
  `ItemManager` singletons.

## What changed from the C# prototype

- Modifier-based stats fix the equipment-swap bonus leak by construction.
- Correct resource/error reporting (the old code returned "insufficient mana"
  when stamina ran out).
- Correct defender resource costs in combat resolution.
- Deterministic, seedable simulation (`World{seed}`) so fights and tests are
  reproducible.
- Presentation fully decoupled from simulation via the event bus.

## Status

Early. The combat core, items, skills, progression, and world are in place and
tested. Networking, persistence, spatial world, and scripting are future work.
