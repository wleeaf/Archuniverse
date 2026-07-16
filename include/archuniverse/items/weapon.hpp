#pragma once

#include "archuniverse/core/stat.hpp"
#include "archuniverse/items/item.hpp"

namespace arch {

// Attack and defence are Attributes so enchantments attach as modifiers.
class Weapon : public Item {
public:
    Weapon(Id id, std::string name, Grade grade, int worth, int attack, int defence);

    [[nodiscard]] bool equippable() const noexcept override { return true; }

    [[nodiscard]] int attack_value() const noexcept { return attack_.value(); }
    [[nodiscard]] int defence_value() const noexcept { return defence_.value(); }
    [[nodiscard]] Attribute& attack() noexcept { return attack_; }
    [[nodiscard]] Attribute& defence() noexcept { return defence_; }

private:
    Attribute attack_;
    Attribute defence_;
};

}  // namespace arch
