#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "archuniverse/core/id.hpp"
#include "archuniverse/core/result.hpp"
#include "archuniverse/items/effect.hpp"

namespace arch {

class Character;

// Base for every carryable thing. Items are owned by the World; a Character's
// inventory and equipment slots hold non-owning pointers into that storage.
class Item {
public:
    enum class Kind { Ware, Food, Weapon, Armor, Potion };

    enum class Grade {
        Ordinary, Common, Uncommon, Rare, Saint, Heroic, King, Legendary, God
    };

    Item(Id id, std::string name, Kind kind, Grade grade, int worth);
    virtual ~Item();

    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;

    [[nodiscard]] Id id() const noexcept { return id_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] Kind kind() const noexcept { return kind_; }
    [[nodiscard]] Grade grade() const noexcept { return grade_; }
    [[nodiscard]] int worth() const noexcept { return worth_; }

    [[nodiscard]] Character* owner() const noexcept { return owner_; }
    void set_owner(Character* owner) noexcept { owner_ = owner; }
    [[nodiscard]] bool equipped() const noexcept { return equipped_; }
    void set_equipped(bool value) noexcept { equipped_ = value; }
    [[nodiscard]] bool used() const noexcept { return used_; }

    [[nodiscard]] virtual bool equippable() const noexcept { return false; }

    // Consumables override this; equipment leaves it defaulted.
    virtual Result<> on_use() { return fail(GameError::Cancelled); }

    // Effect management. apply_effects/revert_effects run at equip/unequip
    // (or use/expire for consumables).
    void add_effect(std::unique_ptr<Effect> effect);
    void apply_effects();
    void revert_effects();
    [[nodiscard]] std::size_t effect_count() const noexcept { return effects_.size(); }

protected:
    Id id_;
    std::string name_;
    Kind kind_;
    Grade grade_;
    int worth_;
    Character* owner_ = nullptr;
    bool equipped_ = false;
    bool used_ = false;
    std::vector<std::unique_ptr<Effect>> effects_;
};

[[nodiscard]] constexpr std::string_view to_string(Item::Grade g) noexcept {
    switch (g) {
        case Item::Grade::Ordinary:  return "Ordinary";
        case Item::Grade::Common:    return "Common";
        case Item::Grade::Uncommon:  return "Uncommon";
        case Item::Grade::Rare:      return "Rare";
        case Item::Grade::Saint:     return "Saint";
        case Item::Grade::Heroic:    return "Heroic";
        case Item::Grade::King:      return "King";
        case Item::Grade::Legendary: return "Legendary";
        case Item::Grade::God:       return "God";
    }
    return "Unknown";
}

}  // namespace arch
