#include "doctest.h"

#include <vector>

#include "archuniverse/archuniverse.hpp"

using namespace arch;

TEST_CASE("a duel resolves to a single winner and awards xp") {
    World world{7};
    Character& strong =
        world.spawn_character("Strong", Character::Sex::Male, 5, 0, 40, 20, 120, 100, 100, 1.2);
    Character& weak =
        world.spawn_character("Weak", Character::Sex::Female, 1, 0, 8, 5, 60, 100, 100, 0.8);
    const int strong_xp_before = strong.xp();

    world.combat().start(strong, weak);
    world.run_for(60.0f);  // plenty of time for the fight to end

    CHECK(world.combat().active_count() == 0);
    CHECK((strong.dead() || weak.dead()));
    CHECK(weak.dead());          // the stronger, faster fighter should win
    CHECK(strong.alive());
    CHECK(strong.xp() > strong_xp_before);  // winner was rewarded
}

TEST_CASE("combat is announced through the event bus") {
    World world{7};
    int hits = 0;
    int deaths = 0;
    bool ended = false;
    world.bus().subscribe<DamageDealt>([&](const DamageDealt&) { ++hits; });
    world.bus().subscribe<EntityDied>([&](const EntityDied&) { ++deaths; });
    world.bus().subscribe<CombatEnded>([&](const CombatEnded&) { ended = true; });

    Character& a =
        world.spawn_character("A", Character::Sex::Male, 3, 0, 30, 10, 100, 100, 100, 1.0);
    Character& b =
        world.spawn_character("B", Character::Sex::Male, 1, 0, 10, 5, 50, 100, 100, 1.0);

    world.combat().start(a, b);
    world.run_for(60.0f);

    CHECK(hits > 0);
    CHECK(deaths >= 1);
    CHECK(ended);
}

TEST_CASE("attack channel selection prefers the stronger affordable option") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);

    // With no melee/magic training the two channels tie; ties go to melee.
    auto both = hero.plan_attack();
    CHECK(both.type == FightType::Melee);

    // Out of stamina, only magic is affordable.
    hero.stamina().deplete();
    auto magic_only = hero.plan_attack();
    CHECK(magic_only.type == FightType::Magic);

    // Out of both, cannot attack.
    hero.mana().deplete();
    auto neither = hero.plan_attack();
    CHECK(neither.type == FightType::CannotAttack);
}

TEST_CASE("regeneration restores vitals once per second while alive") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);
    hero.health_regen().set_base(5);
    hero.health().set_current(50);

    world.run_for(3.0f);  // 3 ticks of regen
    CHECK(hero.health().current() == 65);
}

TEST_CASE("a bled entity keeps taking damage after the hit") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);
    hero.health_regen().set_base(0);  // isolate bleed from regen
    hero.health().set_current(50);

    hero.add_status(std::make_unique<BleedEffect>(3.0f, 4));  // 4 dmg/sec for 3s
    CHECK(hero.status_count() == 1);

    world.run_for(3.0f);
    CHECK(hero.health().current() == 50 - 12);
    CHECK(hero.status_count() == 0);  // expired and detached
}
