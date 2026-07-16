#pragma once

#include <string_view>

namespace arch {

// The channel an attack or defence uses. Melee spends stamina, Magic spends
// mana; the two "Cannot" states mean the entity is too depleted to act.
enum class FightType {
    Natural,
    Melee,
    Magic,
    CannotAttack,
    CannotDefend,
};

[[nodiscard]] constexpr std::string_view to_string(FightType t) noexcept {
    switch (t) {
        case FightType::Natural:       return "Natural";
        case FightType::Melee:         return "Melee";
        case FightType::Magic:         return "Magic";
        case FightType::CannotAttack:  return "CannotAttack";
        case FightType::CannotDefend:  return "CannotDefend";
    }
    return "Unknown";
}

}  // namespace arch
