#pragma once

#include <expected>
#include <string_view>

namespace arch {

// Every fallible operation reports one of these instead of throwing.
enum class GameError {
    Cancelled,
    InventoryFull,
    ItemAlreadyOwned,
    ItemNotInInventory,
    ItemNotOwned,
    NotEquippable,
    InsufficientGold,
    InsufficientMana,
    InsufficientStamina,
    InsufficientHealth,
    LevelTooLow,
    SkillAlreadyUnlocked,
    NoSkillPoints,
    PrerequisiteMissing,
};

// A value-or-error result. Result<> (void) is a plain success/failure status.
template <class T = void>
using Result = std::expected<T, GameError>;

using Status = Result<void>;

[[nodiscard]] inline constexpr Status ok() noexcept { return Status{}; }

[[nodiscard]] inline constexpr std::unexpected<GameError> fail(GameError e) noexcept {
    return std::unexpected(e);
}

[[nodiscard]] constexpr std::string_view to_string(GameError e) noexcept {
    switch (e) {
        case GameError::Cancelled:            return "Cancelled";
        case GameError::InventoryFull:        return "InventoryFull";
        case GameError::ItemAlreadyOwned:     return "ItemAlreadyOwned";
        case GameError::ItemNotInInventory:   return "ItemNotInInventory";
        case GameError::ItemNotOwned:         return "ItemNotOwned";
        case GameError::NotEquippable:        return "NotEquippable";
        case GameError::InsufficientGold:     return "InsufficientGold";
        case GameError::InsufficientMana:     return "InsufficientMana";
        case GameError::InsufficientStamina:  return "InsufficientStamina";
        case GameError::InsufficientHealth:   return "InsufficientHealth";
        case GameError::LevelTooLow:          return "LevelTooLow";
        case GameError::SkillAlreadyUnlocked: return "SkillAlreadyUnlocked";
        case GameError::NoSkillPoints:        return "NoSkillPoints";
        case GameError::PrerequisiteMissing:  return "PrerequisiteMissing";
    }
    return "Unknown";
}

}  // namespace arch
