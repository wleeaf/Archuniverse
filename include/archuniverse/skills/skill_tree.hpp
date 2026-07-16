#pragma once

#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include "archuniverse/core/result.hpp"
#include "archuniverse/skills/skill.hpp"

namespace arch {

class LivingEntity;

// Per-entity progression: two trainable abilities (Melee, Magic) that scale the
// derived combat stats, plus a set of unlockable passive skills bought with
// points earned on level-up.
class SkillTree {
public:
    enum class Ability { Melee, Magic };

    explicit SkillTree(LivingEntity& owner);

    [[nodiscard]] int ability(Ability ability) const;
    Result<> increase_ability(Ability ability, int amount = 1);

    void gain_point() noexcept { ++unused_points_; }
    [[nodiscard]] int unused_points() const noexcept { return unused_points_; }

    Result<> unlock(std::string_view skill_name);

    // Save/load helpers: set state directly without spending points.
    void set_ability(Ability ability, int level);
    void set_unused_points(int points) noexcept { unused_points_ = points; }
    Result<> force_unlock(std::string_view skill_name);
    [[nodiscard]] bool is_unlocked(std::string_view skill_name) const;
    [[nodiscard]] const std::vector<std::unique_ptr<Skill>>& skills() const noexcept {
        return skills_;
    }

    // Push ability levels into the owner's derived Melee/Magic attributes.
    void recompute_derived();

private:
    void add_skill(std::unique_ptr<Skill> skill);
    [[nodiscard]] Skill* find(std::string_view name);

    LivingEntity& owner_;
    std::map<Ability, int> abilities_;
    std::vector<std::unique_ptr<Skill>> skills_;
    int unused_points_ = 0;
};

}  // namespace arch
