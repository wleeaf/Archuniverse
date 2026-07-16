#pragma once

#include "archuniverse/core/context.hpp"
#include "archuniverse/core/tickable.hpp"

namespace arch {

class LivingEntity;

// A real-time duel between two fighters. Each has an independent, speed-scaled
// attack cooldown; when either drops, they strike. All outcomes (hits, fizzles,
// deaths, the end of the fight) are announced on the EventBus rather than
// printed, so presentation is entirely decoupled from the simulation.
class Combat : public Tickable {
public:
    Combat(Context ctx, LivingEntity& a, LivingEntity& b);
    ~Combat() override;

    Combat(const Combat&) = delete;
    Combat& operator=(const Combat&) = delete;

    [[nodiscard]] bool over() const noexcept { return finished_; }
    [[nodiscard]] LivingEntity* winner() const noexcept { return winner_; }
    [[nodiscard]] LivingEntity* loser() const noexcept { return loser_; }
    [[nodiscard]] LivingEntity& fighter_a() const noexcept { return a_; }
    [[nodiscard]] LivingEntity& fighter_b() const noexcept { return b_; }

    void tick(float dt) override;

private:
    void resolve(LivingEntity& attacker, LivingEntity& defender);
    void finish();
    [[nodiscard]] float attack_interval(const LivingEntity& fighter) const;

    static constexpr float kBaseAttackInterval = 1.0f;

    Context ctx_;
    LivingEntity& a_;
    LivingEntity& b_;
    float cooldown_a_ = 0.0f;
    float cooldown_b_ = 0.0f;
    bool finished_ = false;
    LivingEntity* winner_ = nullptr;
    LivingEntity* loser_ = nullptr;
};

}  // namespace arch
