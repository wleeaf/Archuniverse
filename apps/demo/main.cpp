#include <iomanip>
#include <iostream>
#include <memory>

#include "archuniverse/archuniverse.hpp"

using namespace arch;

namespace {

void print_vitals(const LivingEntity& e) {
    std::cout << "  " << std::left << std::setw(8) << e.name() << (e.alive() ? " ALIVE" : " DEAD ")
              << "  HP " << std::setw(4) << e.health().current() << " MP " << std::setw(4)
              << e.mana().current() << " SP " << std::setw(4) << e.stamina().current()
              << " (lvl " << e.level() << ")\n";
}

}  // namespace

int main() {
    World world{1337};  // fixed seed -> reproducible fight

    // Presentation subscribes to simulation events; combat itself prints nothing.
    world.bus().subscribe<DamageDealt>([](const DamageDealt& e) {
        std::cout << "  " << e.attacker->name() << " hits " << e.target->name() << " for "
                  << e.amount << " (" << to_string(e.type) << ")"
                  << (e.caused_bleed ? "  *bleeding*" : "") << "\n";
    });
    world.bus().subscribe<EntityDied>([](const EntityDied& e) {
        std::cout << "  >>> " << e.entity->name() << " has fallen!\n";
    });
    world.bus().subscribe<LeveledUp>([](const LeveledUp& e) {
        std::cout << "  *** " << e.entity->name() << " reached level " << e.new_level << "\n";
    });
    world.bus().subscribe<CombatEnded>([](const CombatEnded& e) {
        if (e.winner != nullptr)
            std::cout << "\n=== " << e.winner->name() << " wins the duel ===\n";
        else
            std::cout << "\n=== double knockout ===\n";
    });

    // Two fighters.
    Character& izroth =
        world.spawn_character("Izroth", Character::Sex::Male, 1, 2000, 20, 20, 100, 100, 100, 1.0);
    Character& lila =
        world.spawn_character("Lila", Character::Sex::Female, 1, 1500, 15, 30, 100, 100, 100, 1.0);

    // Gear. Izroth's weapon carries an enchantment to exercise the modifier path.
    Armor& izroth_armor = world.make_armor("Ordinary Armor", Item::Grade::Ordinary, 100, 15);
    Weapon& izroth_weapon = world.make_weapon("Keen Blade", Item::Grade::Uncommon, 300, 35, 5);
    izroth_weapon.add_effect(std::make_unique<Aggressiveness>(20));  // +20% attack while equipped

    Armor& lila_armor = world.make_armor("Ordinary Armor", Item::Grade::Ordinary, 100, 25);
    Weapon& lila_weapon = world.make_weapon("Ordinary Weapon", Item::Grade::Ordinary, 100, 35, 5);

    izroth.add_and_equip(izroth_armor);
    izroth.add_and_equip(izroth_weapon);
    lila.add_and_equip(lila_armor);
    lila.add_and_equip(lila_weapon);

    // Progression.
    izroth.add_xp(1750);
    (void)izroth.skills().increase_ability(SkillTree::Ability::Melee, 3);
    lila.add_xp(1000);
    (void)lila.skills().increase_ability(SkillTree::Ability::Melee, 2);

    std::cout << "-- Archuniverse combat demo --\n\n";
    std::cout << "Izroth attack (melee): " << izroth.melee_damage()
              << "   [+20% enchant folded into the weapon]\n";
    std::cout << "Lila   attack (melee): " << lila.melee_damage() << "\n\n";
    print_vitals(izroth);
    print_vitals(lila);
    std::cout << "\n";

    world.combat().start(izroth, lila);

    // Drive the simulation a simulated second at a time and report vitals.
    constexpr float dt = GameLoop::kDefaultDt;
    for (int second = 1; second <= 60; ++second) {
        world.run_for(1.0f, dt);
        std::cout << "[t=" << std::setw(2) << second << "s]\n";
        print_vitals(izroth);
        print_vitals(lila);
        if (izroth.dead() || lila.dead()) break;
    }

    return 0;
}
