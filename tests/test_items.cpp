#include "doctest.h"

#include <memory>

#include "archuniverse/archuniverse.hpp"

using namespace arch;

TEST_CASE("equipping a weapon adds its attack and reverting is exact") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Female, 1, 0, 10, 10, 100, 100, 100, 1.0);
    const int bare = hero.melee_damage();

    Weapon& sword = world.make_weapon("Sword", Item::Grade::Common, 100, 30, 0);
    CHECK(hero.add_and_equip(sword).has_value());
    CHECK(hero.melee_damage() == bare + 30);
    CHECK(hero.equipped_weapon() == &sword);

    CHECK(hero.unequip(sword).has_value());
    CHECK(hero.melee_damage() == bare);
    CHECK(hero.equipped_weapon() == nullptr);
    CHECK(hero.holds(sword));  // back in the bag
}

TEST_CASE("weapon enchantment applies on equip and fully reverts on unequip") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);

    Weapon& blade = world.make_weapon("Keen Blade", Item::Grade::Rare, 500, 100, 0);
    blade.add_effect(std::make_unique<Aggressiveness>(20));  // +20%

    CHECK(blade.attack_value() == 100);
    CHECK(hero.add_and_equip(blade).has_value());
    CHECK(blade.attack_value() == 120);  // enchantment live

    CHECK(hero.unequip(blade).has_value());
    CHECK(blade.attack_value() == 100);  // enchantment removed, no leak
}

TEST_CASE("swapping equipment does not leak the previous item's bonus") {
    // This is the regression test for the original C# swap bug.
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);

    Weapon& first = world.make_weapon("First", Item::Grade::Rare, 500, 40, 0);
    first.add_effect(std::make_unique<Aggressiveness>(50));  // +50%
    Weapon& second = world.make_weapon("Second", Item::Grade::Rare, 500, 60, 0);

    CHECK(hero.add_and_equip(first).has_value());
    CHECK(first.attack_value() == 60);  // 40 * 1.5

    CHECK(hero.add_item(second).has_value());
    CHECK(hero.equip(second).has_value());  // swaps first out

    CHECK(hero.equipped_weapon() == &second);
    CHECK(first.attack_value() == 40);  // first's enchantment was reverted
    CHECK_FALSE(first.equipped());
    CHECK(hero.holds(first));  // returned to inventory
}

TEST_CASE("food heals its owner and is consumed") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Female, 1, 0, 10, 10, 100, 100, 100, 1.0);
    hero.health().set_current(40);

    Food& bread = world.make_food("Bread", Item::Grade::Ordinary, 5, 25);
    CHECK(hero.add_item(bread).has_value());
    CHECK(hero.use(bread).has_value());

    CHECK(hero.health().current() == 65);
    CHECK_FALSE(hero.holds(bread));  // eaten
}

TEST_CASE("trading moves an item and gold between characters") {
    World world{1};
    Character& seller =
        world.spawn_character("Seller", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);
    Character& buyer =
        world.spawn_character("Buyer", Character::Sex::Male, 1, 100, 10, 10, 100, 100, 100, 1.0);

    Ware& gem = world.make_ware("Gem", Item::Grade::Rare, 50);
    CHECK(seller.add_item(gem).has_value());

    CHECK(buyer.buy_item(gem, 50, seller).has_value());
    CHECK(buyer.holds(gem));
    CHECK_FALSE(seller.holds(gem));
    CHECK(seller.gold() == 50);
    CHECK(buyer.gold() == 50);
}

TEST_CASE("errors report the correct code") {
    World world{1};
    Character& hero =
        world.spawn_character("Hero", Character::Sex::Male, 1, 0, 10, 10, 100, 100, 100, 1.0);

    // Draining stamina when empty reports stamina, not mana (the old C# bug).
    hero.stamina().deplete();
    auto stamina_result = hero.spend_for(FightType::Melee, 5);
    REQUIRE_FALSE(stamina_result.has_value());
    CHECK(stamina_result.error() == GameError::InsufficientStamina);

    hero.mana().deplete();
    auto mana_result = hero.spend_for(FightType::Magic, 5);
    REQUIRE_FALSE(mana_result.has_value());
    CHECK(mana_result.error() == GameError::InsufficientMana);
}
