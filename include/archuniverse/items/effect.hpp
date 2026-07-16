#pragma once

#include <string_view>

namespace arch {

class Item;

// A special effect (enchantment / consumable buff) attached to an item.
// apply() installs modifiers on the item or its owner; revert() removes exactly
// those modifiers. Concrete effects store their own ModifierId handles so a
// revert is precise, which is what keeps gear swaps from leaking bonuses.
class Effect {
public:
    virtual ~Effect() = default;

    virtual void apply(Item& item) = 0;
    virtual void revert(Item& item) = 0;

    [[nodiscard]] virtual std::string_view name() const = 0;
    [[nodiscard]] bool applied() const noexcept { return applied_; }

protected:
    bool applied_ = false;
};

}  // namespace arch
