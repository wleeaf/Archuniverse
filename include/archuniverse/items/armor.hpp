#pragma once

#include "archuniverse/core/stat.hpp"
#include "archuniverse/items/item.hpp"

namespace arch {

class Armor : public Item {
public:
    Armor(Id id, std::string name, Grade grade, int worth, int defence);

    [[nodiscard]] bool equippable() const noexcept override { return true; }

    [[nodiscard]] int defence_value() const noexcept { return defence_.value(); }
    [[nodiscard]] int defence_base() const noexcept { return defence_.base(); }
    [[nodiscard]] Attribute& defence() noexcept { return defence_; }

private:
    Attribute defence_;
};

}  // namespace arch
