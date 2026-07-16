#pragma once

#include <string_view>

namespace arch {

class LivingEntity;

// A time-limited effect an entity carries and advances itself each tick:
// bleed, poison, a temporary buff. Replaces the old free-floating, self-
// registering Damage objects, so lifetime is tied to the entity that owns it.
class StatusEffect {
public:
    virtual ~StatusEffect() = default;

    // Called once when the effect is attached and again when it is removed,
    // for buffs that add/remove a modifier for their whole duration.
    virtual void on_attach(LivingEntity&) {}
    virtual void on_detach(LivingEntity&) {}

    // Advance by dt. Return true when the effect has finished and should be
    // detached and destroyed.
    virtual bool advance(LivingEntity& owner, float dt) = 0;

    [[nodiscard]] virtual std::string_view name() const = 0;
};

}  // namespace arch
