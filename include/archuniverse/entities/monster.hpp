#pragma once

#include "archuniverse/entities/living_entity.hpp"

namespace arch {

// A non-player combatant. Behaves as a plain LivingEntity for now; a place to
// hang AI, loot tables, and aggro later.
class Monster : public LivingEntity {
public:
    using LivingEntity::LivingEntity;
};

}  // namespace arch
