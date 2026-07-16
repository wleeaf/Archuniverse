#include "archuniverse/ai/behaviors.hpp"

#include <cmath>

#include "archuniverse/combat/combat_manager.hpp"
#include "archuniverse/core/rng.hpp"
#include "archuniverse/entities/character.hpp"
#include "archuniverse/entities/monster.hpp"
#include "archuniverse/world/world.hpp"

namespace arch {

void WanderBehavior::update(Monster& self, World& world, float dt) {
    if (!has_target_ || distance(self.position(), target_) < 0.5) {
        Rng& rng = world.rng();
        const double angle = rng.range(0.0f, 6.2831853f);  // [0, 2*pi)
        const double reach = rng.range(0.0f, static_cast<float>(radius_));
        target_ = home_ + Vec3{std::cos(angle) * reach, std::sin(angle) * reach, 0.0};
        has_target_ = true;
    }
    self.move_toward(target_, speed_ * static_cast<double>(dt));
}

void HunterBehavior::update(Monster& self, World& world, float dt) {
    if (engaged_) return;

    LivingEntity* prey = world.nearest(self.position(), [](const LivingEntity& candidate) {
        return dynamic_cast<const Character*>(&candidate) != nullptr;
    });
    if (prey == nullptr) return;

    if (self.distance_to(*prey) <= engage_range_) {
        world.combat().start(self, *prey);
        engaged_ = true;
    } else {
        self.move_toward(prey->position(), speed_ * static_cast<double>(dt));
    }
}

}  // namespace arch
