#include "archuniverse/combat/combat.hpp"

#include <algorithm>
#include <memory>

#include "archuniverse/combat/bleed.hpp"
#include "archuniverse/core/event_bus.hpp"
#include "archuniverse/core/events.hpp"
#include "archuniverse/core/game_loop.hpp"
#include "archuniverse/core/rng.hpp"
#include "archuniverse/entities/character.hpp"
#include "archuniverse/entities/living_entity.hpp"

namespace arch {
namespace {

// When a defender can't pay to defend, only worn armor still counts.
int passive_defence(LivingEntity& entity) {
    if (auto* character = dynamic_cast<Character*>(&entity)) {
        if (Armor* armor = character->equipped_armor()) return armor->defence_value();
    }
    return 0;
}

}  // namespace

Combat::Combat(Context ctx, LivingEntity& a, LivingEntity& b) : ctx_(ctx), a_(a), b_(b) {
    ctx_.loop.add(this);
    ctx_.bus.publish(CombatStarted{&a_, &b_});
}

Combat::~Combat() { ctx_.loop.remove(this); }

float Combat::attack_interval(const LivingEntity& fighter) const {
    const double speed = fighter.speed() > 0.0 ? fighter.speed() : 1.0;
    return static_cast<float>(kBaseAttackInterval / speed);
}

void Combat::resolve(LivingEntity& attacker, LivingEntity& defender) {
    const LivingEntity::Action attack = attacker.plan_attack();
    if (attack.type == FightType::CannotAttack) {
        ctx_.bus.publish(AttackFizzled{&attacker, attack.type});
        return;
    }
    if (!attacker.spend_for(attack.type, attacker.attack_cost())) {
        ctx_.bus.publish(AttackFizzled{&attacker, attack.type});
        return;
    }

    const LivingEntity::Action defence = defender.plan_defence();
    int defence_value = defence.amount;
    if (defence.type == FightType::CannotDefend ||
        !defender.spend_for(defence.type, defender.defend_cost())) {
        defence_value = passive_defence(defender);
    }

    const int damage = std::max(0, attack.amount - defence_value);
    const int dealt = defender.apply_damage(damage);

    bool caused_bleed = false;
    if (dealt > 0 && ctx_.rng.chance(0.5)) {
        const float duration = ctx_.rng.range(1.0f, 5.0f);
        defender.add_status(std::make_unique<BleedEffect>(duration, 1));
        caused_bleed = true;
    }

    ctx_.bus.publish(DamageDealt{&attacker, &defender, attack.type, dealt, caused_bleed});
    if (defender.dead()) ctx_.bus.publish(EntityDied{&defender, &attacker});
}

void Combat::tick(float dt) {
    if (finished_) return;
    if (a_.dead() || b_.dead()) {
        finish();
        return;
    }

    cooldown_a_ -= dt;
    cooldown_b_ -= dt;

    if (cooldown_a_ <= 0.0f && a_.alive() && b_.alive()) {
        resolve(a_, b_);
        cooldown_a_ = attack_interval(a_);
    }
    if (cooldown_b_ <= 0.0f && a_.alive() && b_.alive()) {
        resolve(b_, a_);
        cooldown_b_ = attack_interval(b_);
    }

    if (a_.dead() || b_.dead()) finish();
}

void Combat::finish() {
    if (finished_) return;
    finished_ = true;
    ctx_.loop.remove(this);

    if (a_.dead() && b_.dead()) {
        winner_ = nullptr;
        loser_ = nullptr;
    } else if (a_.dead()) {
        winner_ = &b_;
        loser_ = &a_;
    } else if (b_.dead()) {
        winner_ = &a_;
        loser_ = &b_;
    }

    ctx_.bus.publish(CombatEnded{winner_, loser_});
}

}  // namespace arch
