#pragma once

#include "archuniverse/core/stat.hpp"
#include "archuniverse/items/effect.hpp"

namespace arch {

// Weapon enchantment: raises attack by a percentage while equipped.
class Aggressiveness : public Effect {
public:
    explicit Aggressiveness(double attack_boost_percent) : percent_(attack_boost_percent) {}
    void apply(Item& item) override;
    void revert(Item& item) override;
    [[nodiscard]] std::string_view name() const override { return "Aggressiveness"; }

private:
    double percent_;
    ModifierId handle_{};
};

// Armor enchantment: raises defence by a percentage while equipped.
class Solidness : public Effect {
public:
    explicit Solidness(double defence_boost_percent) : percent_(defence_boost_percent) {}
    void apply(Item& item) override;
    void revert(Item& item) override;
    [[nodiscard]] std::string_view name() const override { return "Solidness"; }

private:
    double percent_;
    ModifierId handle_{};
};

// Potion buff: doubles the owner's health regeneration while in effect.
class HealthRegenBoost : public Effect {
public:
    void apply(Item& item) override;
    void revert(Item& item) override;
    [[nodiscard]] std::string_view name() const override { return "HealthRegenBoost"; }

private:
    ModifierId handle_{};
};

}  // namespace arch
