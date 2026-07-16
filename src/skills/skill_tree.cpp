#include "archuniverse/skills/skill_tree.hpp"

#include <algorithm>
#include <utility>

#include "archuniverse/core/stat.hpp"
#include "archuniverse/entities/character.hpp"
#include "archuniverse/entities/living_entity.hpp"

namespace arch {

SkillTree::SkillTree(LivingEntity& owner) : owner_(owner) {
    abilities_[Ability::Melee] = 1;
    abilities_[Ability::Magic] = 1;

    add_skill(std::make_unique<Skill>(
        "Strong Body", "Increases max health by 50", 2,
        [](LivingEntity& e) { e.health().max_attribute().add_modifier(ModOp::Flat, 50); }));

    add_skill(std::make_unique<Skill>(
        "Trained Mind", "Increases max mana by 40", 2,
        [](LivingEntity& e) { e.mana().max_attribute().add_modifier(ModOp::Flat, 40); }));

    add_skill(std::make_unique<Skill>(
        "Iron Body", "Increases base defence by 30", 2,
        [](LivingEntity& e) { e.base_defence().add_modifier(ModOp::Flat, 30); }));

    add_skill(std::make_unique<Skill>(
        "Faster Regen", "Regenerate +2 health per second", 3,
        [](LivingEntity& e) { e.health_regen().add_modifier(ModOp::Flat, 2); }));

    add_skill(std::make_unique<Skill>(
        "Carrier", "Doubles inventory capacity", 5, [](LivingEntity& e) {
            if (auto* c = dynamic_cast<Character*>(&e))
                c->set_inventory_capacity(c->inventory_capacity() * 2);
        }));

    recompute_derived();
}

int SkillTree::ability(Ability ability) const {
    const auto it = abilities_.find(ability);
    return it == abilities_.end() ? 0 : it->second;
}

Result<> SkillTree::increase_ability(Ability ability, int amount) {
    if (amount <= 0) return fail(GameError::Cancelled);
    if (unused_points_ < amount) return fail(GameError::NoSkillPoints);
    abilities_[ability] += amount;
    unused_points_ -= amount;
    recompute_derived();
    return ok();
}

Result<> SkillTree::unlock(std::string_view skill_name) {
    Skill* skill = find(skill_name);
    if (skill == nullptr) return fail(GameError::Cancelled);
    if (unused_points_ <= 0) return fail(GameError::NoSkillPoints);
    if (auto r = skill->unlock(owner_); !r) return r;
    --unused_points_;
    return ok();
}

void SkillTree::set_ability(Ability ability, int level) {
    abilities_[ability] = level;
    recompute_derived();
}

Result<> SkillTree::force_unlock(std::string_view skill_name) {
    Skill* skill = find(skill_name);
    if (skill == nullptr) return fail(GameError::Cancelled);
    return skill->unlock(owner_);
}

bool SkillTree::is_unlocked(std::string_view skill_name) const {
    const auto it = std::ranges::find_if(
        skills_, [&](const std::unique_ptr<Skill>& s) { return s->name() == skill_name; });
    return it != skills_.end() && (*it)->unlocked();
}

void SkillTree::recompute_derived() {
    owner_.melee().set_base(ability(Ability::Melee) * 5);
    owner_.magic().set_base(ability(Ability::Magic) * 5);
}

void SkillTree::add_skill(std::unique_ptr<Skill> skill) { skills_.push_back(std::move(skill)); }

Skill* SkillTree::find(std::string_view name) {
    const auto it = std::ranges::find_if(
        skills_, [&](const std::unique_ptr<Skill>& s) { return s->name() == name; });
    return it == skills_.end() ? nullptr : it->get();
}

}  // namespace arch
