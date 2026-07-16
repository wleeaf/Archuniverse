#include "archuniverse/skills/skill.hpp"

#include <algorithm>
#include <utility>

#include "archuniverse/entities/living_entity.hpp"

namespace arch {

Skill::Skill(std::string name, std::string description, int required_level, EffectFn effect)
    : name_(std::move(name)),
      description_(std::move(description)),
      required_level_(required_level),
      effect_(std::move(effect)) {}

bool Skill::can_unlock(const LivingEntity& entity) const {
    if (unlocked_) return false;
    if (entity.level() < required_level_) return false;
    return std::ranges::all_of(prerequisites_,
                               [](const Skill* s) { return s->unlocked(); });
}

Result<> Skill::unlock(LivingEntity& entity) {
    if (unlocked_) return fail(GameError::SkillAlreadyUnlocked);
    if (entity.level() < required_level_) return fail(GameError::LevelTooLow);
    for (const Skill* prereq : prerequisites_)
        if (!prereq->unlocked()) return fail(GameError::PrerequisiteMissing);

    unlocked_ = true;
    if (effect_) effect_(entity);
    return ok();
}

}  // namespace arch
