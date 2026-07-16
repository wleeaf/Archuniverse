#pragma once

#include "archuniverse/items/item.hpp"

namespace arch {

// A trade good with no direct combat use. A carrier for special effects and
// economy value.
class Ware : public Item {
public:
    Ware(Id id, std::string name, Grade grade, int worth)
        : Item(id, std::move(name), Kind::Ware, grade, worth) {}
};

}  // namespace arch
