#pragma once

#include "archuniverse/entities/status_effect.hpp"

namespace arch {

// Damage-over-time applied on a bleeding hit. Ticks once per second on the
// entity that carries it and expires after its duration (or when the host dies).
class BleedEffect : public StatusEffect {
public:
    BleedEffect(float duration_seconds, int damage_per_tick)
        : remaining_(duration_seconds), damage_per_tick_(damage_per_tick) {}

    bool advance(LivingEntity& owner, float dt) override;
    [[nodiscard]] std::string_view name() const override { return "Bleed"; }

private:
    float remaining_;
    int damage_per_tick_;
    float accumulator_ = 0.0f;
};

}  // namespace arch
