#include "archuniverse/items/weapon.hpp"

#include <utility>

namespace arch {

Weapon::Weapon(Id id, std::string name, Grade grade, int worth, int attack, int defence)
    : Item(id, std::move(name), Kind::Weapon, grade, worth),
      attack_(attack),
      defence_(defence) {}

}  // namespace arch
