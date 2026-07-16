#pragma once

#include <memory>

#include "archuniverse/entities/living_entity.hpp"

namespace arch {

class Behavior;

// A non-player combatant. Carries an optional AI Behavior that the AiSystem
// drives each tick.
class Monster : public LivingEntity {
public:
    using LivingEntity::LivingEntity;
    ~Monster() override;  // out-of-line: owns unique_ptr<Behavior> (incomplete here)

    void set_behavior(std::unique_ptr<Behavior> behavior);
    [[nodiscard]] Behavior* behavior() const noexcept { return behavior_.get(); }

private:
    std::unique_ptr<Behavior> behavior_;
};

}  // namespace arch
