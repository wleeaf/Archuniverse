#include "archuniverse/world/world.hpp"

#include <utility>

namespace arch {

Character& World::spawn_character(std::string name, Character::Sex gender, int level, int gold,
                                  int base_attack, int base_defence, int max_health, int max_mana,
                                  int max_stamina, double speed) {
    auto entity = std::make_unique<Character>(context(), ids_.next(), std::move(name), gender,
                                              level, gold, base_attack, base_defence, max_health,
                                              max_mana, max_stamina, speed);
    Character& ref = *entity;
    entities_.push_back(std::move(entity));
    return ref;
}

Monster& World::spawn_monster(std::string name, int level, int base_attack, int base_defence,
                              int max_health, int max_mana, int max_stamina, double speed) {
    auto entity = std::make_unique<Monster>(context(), ids_.next(), std::move(name), level,
                                            base_attack, base_defence, max_health, max_mana,
                                            max_stamina, speed);
    Monster& ref = *entity;
    entities_.push_back(std::move(entity));
    return ref;
}

Weapon& World::make_weapon(std::string name, Item::Grade grade, int worth, int attack,
                           int defence) {
    auto item = std::make_unique<Weapon>(ids_.next(), std::move(name), grade, worth, attack,
                                         defence);
    Weapon& ref = *item;
    items_.push_back(std::move(item));
    return ref;
}

Armor& World::make_armor(std::string name, Item::Grade grade, int worth, int defence) {
    auto item = std::make_unique<Armor>(ids_.next(), std::move(name), grade, worth, defence);
    Armor& ref = *item;
    items_.push_back(std::move(item));
    return ref;
}

Potion& World::make_potion(std::string name, Item::Grade grade, int worth, int health_boost,
                           int mana_boost, int stamina_boost, float effect_seconds) {
    auto item = std::make_unique<Potion>(ids_.next(), std::move(name), grade, worth, health_boost,
                                        mana_boost, stamina_boost, effect_seconds);
    Potion& ref = *item;
    items_.push_back(std::move(item));
    return ref;
}

Food& World::make_food(std::string name, Item::Grade grade, int worth, int health_boost) {
    auto item = std::make_unique<Food>(ids_.next(), std::move(name), grade, worth, health_boost);
    Food& ref = *item;
    items_.push_back(std::move(item));
    return ref;
}

Ware& World::make_ware(std::string name, Item::Grade grade, int worth) {
    auto item = std::make_unique<Ware>(ids_.next(), std::move(name), grade, worth);
    Ware& ref = *item;
    items_.push_back(std::move(item));
    return ref;
}

}  // namespace arch
