#include "archuniverse/items/armor.hpp"

#include <utility>

namespace arch {

Armor::Armor(Id id, std::string name, Grade grade, int worth, int defence)
    : Item(id, std::move(name), Kind::Armor, grade, worth), defence_(defence) {}

}  // namespace arch
