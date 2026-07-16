#include "archuniverse/entities/living_entity.hpp"

#include <algorithm>
#include <utility>

#include "archuniverse/core/event_bus.hpp"
#include "archuniverse/core/events.hpp"
#include "archuniverse/core/game_loop.hpp"
#include "archuniverse/skills/skill_tree.hpp"

namespace arch {

LivingEntity::LivingEntity(Context ctx, Id id, std::string name, int level, int base_attack,
                           int base_defence, int max_health, int max_mana, int max_stamina,
                           double speed)
    : ctx_(ctx),
      id_(id),
      name_(std::move(name)),
      health_(max_health),
      mana_(max_mana),
      stamina_(max_stamina),
      base_attack_(base_attack),
      base_defence_(base_defence),
      level_(level < 1 ? 1 : level),
      speed_(speed) {
    skills_ = std::make_unique<SkillTree>(*this);
    ctx_.loop.add(this);
}

LivingEntity::~LivingEntity() { ctx_.loop.remove(this); }

Status LivingEntity::spend_for(FightType channel, int amount) {
    switch (channel) {
        case FightType::Melee:
            if (!stamina_.has_at_least(amount)) return fail(GameError::InsufficientStamina);
            stamina_.damage(amount);
            return ok();
        case FightType::Magic:
            if (!mana_.has_at_least(amount)) return fail(GameError::InsufficientMana);
            mana_.damage(amount);
            return ok();
        default:
            return fail(GameError::Cancelled);
    }
}

int LivingEntity::melee_damage() const { return base_attack_.value() + melee_.value(); }
int LivingEntity::magic_damage() const { return base_attack_.value() + magic_.value(); }
int LivingEntity::melee_defence() const { return base_defence_.value() + melee_.value(); }
int LivingEntity::magic_defence() const { return base_defence_.value() + magic_.value(); }

int LivingEntity::attack_cost() const noexcept {
    constexpr int kBase = 15;
    constexpr int kFloor = 5;
    return std::max(kFloor, kBase - level_ / 2);
}

int LivingEntity::defend_cost() const noexcept {
    constexpr int kBase = 8;
    constexpr int kFloor = 3;
    return std::max(kFloor, kBase - level_ / 2);
}

LivingEntity::Action LivingEntity::plan_attack() const {
    const int cost = attack_cost();
    const bool can_melee = stamina_.has_at_least(cost);
    const bool can_magic = mana_.has_at_least(cost);
    const int melee = melee_damage();
    const int magic = magic_damage();

    if (can_melee && can_magic)
        return (melee >= magic) ? Action{FightType::Melee, melee} : Action{FightType::Magic, magic};
    if (can_melee) return Action{FightType::Melee, melee};
    if (can_magic) return Action{FightType::Magic, magic};
    return Action{FightType::CannotAttack, 0};
}

LivingEntity::Action LivingEntity::plan_defence() const {
    const int cost = defend_cost();
    const bool can_melee = stamina_.has_at_least(cost);
    const bool can_magic = mana_.has_at_least(cost);
    const int melee = melee_defence();
    const int magic = magic_defence();

    if (can_melee && can_magic)
        return (melee >= magic) ? Action{FightType::Melee, melee} : Action{FightType::Magic, magic};
    if (can_melee) return Action{FightType::Melee, melee};
    if (can_magic) return Action{FightType::Magic, magic};
    return Action{FightType::CannotDefend, 0};
}

void LivingEntity::move_toward(const Vec3& target, double max_step) noexcept {
    const Vec3 delta = target - position_;
    const double dist = delta.length();
    if (dist <= max_step || dist == 0.0)
        position_ = target;
    else
        position_ = position_ + delta.normalized() * max_step;
}

int LivingEntity::apply_damage(int amount) {
    const int before = health_.current();
    health_.damage(std::max(0, amount));
    return before - health_.current();
}

void LivingEntity::add_status(std::unique_ptr<StatusEffect> effect) {
    if (!effect) return;
    effect->on_attach(*this);
    statuses_.push_back(std::move(effect));
}

void LivingEntity::add_xp(int amount) {
    if (amount <= 0) return;
    xp_ += amount;
    level_up_from_xp();
}

int LivingEntity::xp_for_level(int level) noexcept { return 100 * level * level; }

void LivingEntity::level_up_from_xp() {
    while (xp_ >= xp_for_level(level_ + 1)) level_up();
}

void LivingEntity::level_up() {
    ++level_;
    skills_->gain_point();
    ctx_.bus.publish(LeveledUp{this, level_});
}

void LivingEntity::tick(float dt) {
    if (dead()) return;

    // Kinematic integration (velocity is zero unless something drives it).
    if (velocity_ != Vec3{}) position_ = position_ + velocity_ * static_cast<double>(dt);

    for (auto it = statuses_.begin(); it != statuses_.end();) {
        if ((*it)->advance(*this, dt)) {
            (*it)->on_detach(*this);
            it = statuses_.erase(it);
        } else {
            ++it;
        }
    }

    regen_timer_ += dt;
    while (regen_timer_ + kTickEpsilon >= 1.0f) {
        regen_timer_ -= 1.0f;
        health_.heal(health_regen_.value());
        mana_.heal(mana_regen_.value());
        stamina_.heal(stamina_regen_.value());
    }
}

}  // namespace arch
