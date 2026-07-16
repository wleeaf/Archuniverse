#include "doctest.h"

#include <memory>

#include "archuniverse/archuniverse.hpp"

using namespace arch;

TEST_CASE("Vec3 math: distance and normalization") {
    CHECK(distance(Vec3{0, 0, 0}, Vec3{3, 4, 0}) == doctest::Approx(5.0));
    const Vec3 n = Vec3{0, 5, 0}.normalized();
    CHECK(n.x == doctest::Approx(0.0));
    CHECK(n.y == doctest::Approx(1.0));
}

TEST_CASE("move_toward steps by at most max_step and snaps on arrival") {
    World world{1};
    Monster& m = world.spawn_monster("Slime", 1, 5, 5, 30, 30, 30, 1.0);
    m.set_position(Vec3{0, 0, 0});

    m.move_toward(Vec3{10, 0, 0}, 3.0);
    CHECK(m.position().x == doctest::Approx(3.0));

    m.move_toward(Vec3{10, 0, 0}, 100.0);  // overshoot -> snap
    CHECK(m.position() == Vec3{10, 0, 0});
}

TEST_CASE("World spatial queries find entities in range and the nearest match") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);
    hero.set_position(Vec3{0, 0, 0});
    Monster& near = world.spawn_monster("Near", 1, 5, 5, 30, 30, 30, 1.0);
    near.set_position(Vec3{2, 0, 0});
    Monster& far = world.spawn_monster("Far", 1, 5, 5, 30, 30, 30, 1.0);
    far.set_position(Vec3{50, 0, 0});

    const auto in_range = world.entities_within(Vec3{0, 0, 0}, 5.0);
    CHECK(in_range.size() == 2);  // hero + near

    LivingEntity* closest_monster = world.nearest(hero.position(), [](const LivingEntity& e) {
        return dynamic_cast<const Monster*>(&e) != nullptr;
    });
    CHECK(closest_monster == &near);
}

TEST_CASE("HunterBehavior chases a character and engages within range") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);
    hero.set_position(Vec3{0, 0, 0});

    Monster& hunter = world.spawn_monster("Hunter", 2, 15, 5, 60, 40, 40, 1.0);
    hunter.set_position(Vec3{20, 0, 0});
    hunter.set_behavior(std::make_unique<HunterBehavior>(/*speed*/ 6.0, /*engage_range*/ 1.5));

    CHECK(world.combat().total_count() == 0);

    // Not yet in range; should close distance but not engage.
    world.run_for(1.0f);
    CHECK(hunter.distance_to(hero) < 20.0);
    CHECK(world.combat().total_count() == 0);

    // Given enough time it reaches the hero and opens exactly one fight.
    world.run_for(10.0f);
    CHECK(world.combat().total_count() == 1);
}

TEST_CASE("WanderBehavior keeps a monster near its home") {
    World world{1};
    Monster& m = world.spawn_monster("Wanderer", 1, 5, 5, 30, 30, 30, 1.0);
    m.set_position(Vec3{0, 0, 0});
    m.set_behavior(std::make_unique<WanderBehavior>(Vec3{0, 0, 0}, /*radius*/ 5.0, /*speed*/ 4.0));

    world.run_for(20.0f);
    // Never strays far beyond the wander radius (plus a small step margin).
    CHECK(distance(m.position(), Vec3{0, 0, 0}) <= 6.0);
}
