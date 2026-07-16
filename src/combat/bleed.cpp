#include "archuniverse/combat/bleed.hpp"

#include "archuniverse/entities/living_entity.hpp"

namespace arch {

bool BleedEffect::advance(LivingEntity& owner, float dt) {
    if (owner.dead()) return true;

    remaining_ -= dt;
    accumulator_ += dt;
    while (accumulator_ + kTickEpsilon >= 1.0f) {
        accumulator_ -= 1.0f;
        owner.apply_damage(damage_per_tick_);
    }
    return remaining_ <= kTickEpsilon;
}

}  // namespace arch
