#pragma once

#include "archuniverse/core/fight_type.hpp"

namespace arch {

class LivingEntity;

// Events published on the World's EventBus. Handlers (logging, XP rewards,
// death cleanup) subscribe to these instead of combat calling them directly.

struct DamageDealt {
    LivingEntity* attacker;
    LivingEntity* target;
    FightType type;
    int amount;
    bool caused_bleed;
};

struct AttackFizzled {
    LivingEntity* attacker;
    FightType attempted;  // what they tried before running out of resource
};

struct EntityDied {
    LivingEntity* entity;
    LivingEntity* killer;  // may be null (e.g. bleed-out)
};

struct LeveledUp {
    LivingEntity* entity;
    int new_level;
};

struct CombatStarted {
    LivingEntity* a;
    LivingEntity* b;
};

struct CombatEnded {
    LivingEntity* winner;  // null on a double knockout
    LivingEntity* loser;
};

}  // namespace arch
