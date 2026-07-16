#include "doctest.h"

#include "archuniverse/archuniverse.hpp"

using namespace arch;

TEST_CASE("xp thresholds follow the 100 * level^2 curve") {
    CHECK(LivingEntity::xp_for_level(2) == 400);
    CHECK(LivingEntity::xp_for_level(3) == 900);
    CHECK(LivingEntity::xp_for_level(4) == 1600);
    CHECK(LivingEntity::xp_for_level(5) == 2500);
}

TEST_CASE("gaining xp levels up and grants skill points") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);

    hero.add_xp(1750);   // -> level 4 (needs 1600, next is 2500)
    CHECK(hero.level() == 4);
    CHECK(hero.skills().unused_points() == 3);  // one per level gained (2,3,4)
}

TEST_CASE("training an ability scales the derived combat stat") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);
    hero.add_xp(1000);  // level 3 -> 2 points

    const int before = hero.melee().value();  // ability 1 -> 5
    CHECK(hero.skills().increase_ability(SkillTree::Ability::Melee, 2).has_value());
    CHECK(hero.melee().value() == before + 10);  // +2 ability * 5
    CHECK(hero.skills().unused_points() == 0);
}

TEST_CASE("unlocking a skill requires points and level, then applies its effect") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);

    // Level 1, no points: cannot unlock.
    auto too_early = hero.skills().unlock("Strong Body");
    REQUIRE_FALSE(too_early.has_value());
    CHECK(too_early.error() == GameError::NoSkillPoints);

    hero.add_xp(400);  // level 2 -> 1 point, meets Strong Body's level req
    const int max_hp = hero.health().max();
    CHECK(hero.skills().unlock("Strong Body").has_value());
    CHECK(hero.health().max() == max_hp + 50);
    CHECK(hero.skills().unused_points() == 0);
}

TEST_CASE("increasing an ability without points fails") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);
    auto result = hero.skills().increase_ability(SkillTree::Ability::Magic, 1);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == GameError::NoSkillPoints);
}
