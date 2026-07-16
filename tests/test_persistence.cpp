#include "doctest.h"

#include <sstream>
#include <string>

#include "archuniverse/archuniverse.hpp"

using namespace arch;

namespace {

Character* find_character(World& world, const std::string& name) {
    for (const auto& e : world.entities()) {
        if (auto* c = dynamic_cast<Character*>(e.get()); c != nullptr && c->name() == name) return c;
    }
    return nullptr;
}

LivingEntity* find_entity(World& world, const std::string& name) {
    for (const auto& e : world.entities())
        if (e->name() == name) return e.get();
    return nullptr;
}

}  // namespace

TEST_CASE("a populated world round-trips through save and load") {
    std::stringstream saved;

    {
        World w{1};
        Character& hero =
            w.spawn_character("Izroth", Character::Sex::Male, 1, 500, 20, 20, 100, 100, 100, 1.25);
        hero.add_xp(2500);  // -> level 5, 4 skill points
        CHECK(hero.level() == 5);
        CHECK(hero.skills().unlock("Strong Body").has_value());              // +50 max hp, 3 left
        CHECK(hero.skills().increase_ability(SkillTree::Ability::Melee, 2).has_value());  // 1 left
        hero.set_position(Vec3{7, -3, 2});
        hero.health().set_current(120);

        Weapon& sword = w.make_weapon("Longsword", Item::Grade::Rare, 400, 35, 5);
        Armor& mail = w.make_armor("Chainmail", Item::Grade::Uncommon, 200, 25);
        Food& apple = w.make_food("Apple", Item::Grade::Ordinary, 3, 10);
        hero.add_and_equip(sword);
        hero.add_and_equip(mail);
        hero.add_item(apple);  // stays in the bag

        Monster& goblin = w.spawn_monster("Goblin", 3, 12, 8, 70, 40, 50, 0.9);
        goblin.set_position(Vec3{40, 0, 0});

        persistence::save(w, saved);
    }

    World restored{999};
    REQUIRE(persistence::load(restored, saved).has_value());

    CHECK(restored.entity_count() == 2);
    CHECK(restored.item_count() == 3);

    Character* hero = find_character(restored, "Izroth");
    REQUIRE(hero != nullptr);
    CHECK(hero->level() == 5);
    CHECK(hero->xp() == 2500);
    CHECK(hero->gold() == 500);
    CHECK(hero->speed() == doctest::Approx(1.25));
    CHECK(hero->skills().unused_points() == 1);
    CHECK(hero->skills().is_unlocked("Strong Body"));
    CHECK(hero->health().max() == 150);       // base 100 + re-applied Strong Body
    CHECK(hero->health().current() == 120);   // clamped against the restored max
    CHECK(hero->melee().value() == 15);       // ability 3 * 5
    CHECK(hero->position() == Vec3{7, -3, 2});

    REQUIRE(hero->equipped_weapon() != nullptr);
    CHECK(hero->equipped_weapon()->name() == "Longsword");
    CHECK(hero->equipped_weapon()->attack_value() == 35);
    REQUIRE(hero->equipped_armor() != nullptr);
    CHECK(hero->equipped_armor()->defence_value() == 25);
    CHECK(hero->inventory().size() == 1);     // the apple
    CHECK(hero->inventory().front()->name() == "Apple");

    LivingEntity* goblin = find_entity(restored, "Goblin");
    REQUIRE(goblin != nullptr);
    CHECK(dynamic_cast<Monster*>(goblin) != nullptr);
    CHECK(goblin->level() == 3);
    CHECK(goblin->health().max() == 70);
    CHECK(goblin->position() == Vec3{40, 0, 0});
}

TEST_CASE("loading rejects a malformed stream") {
    World w{1};
    std::stringstream bad;
    bad << "NOT_A_SAVE\tgarbage\n";
    CHECK_FALSE(persistence::load(w, bad).has_value());
}
