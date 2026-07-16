#pragma once

#include "archuniverse/items/item.hpp"

namespace arch {

// A one-shot consumable that restores health immediately.
class Food : public Item {
public:
    Food(Id id, std::string name, Grade grade, int worth, int health_boost);

    Result<> on_use() override;

    [[nodiscard]] int health_boost() const noexcept { return health_boost_; }

private:
    int health_boost_;
};

}  // namespace arch
