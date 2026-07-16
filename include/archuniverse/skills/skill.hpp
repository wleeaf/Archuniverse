#pragma once

#include <functional>
#include <string>
#include <vector>

#include "archuniverse/core/result.hpp"

namespace arch {

class LivingEntity;

// An unlockable node. Its effect runs once, the moment it is unlocked, applying
// a permanent bonus (usually a modifier) to the entity.
class Skill {
public:
    using EffectFn = std::function<void(LivingEntity&)>;

    Skill(std::string name, std::string description, int required_level, EffectFn effect);

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& description() const noexcept { return description_; }
    [[nodiscard]] int required_level() const noexcept { return required_level_; }
    [[nodiscard]] bool unlocked() const noexcept { return unlocked_; }

    void add_prerequisite(const Skill* skill) { prerequisites_.push_back(skill); }

    [[nodiscard]] bool can_unlock(const LivingEntity& entity) const;
    Result<> unlock(LivingEntity& entity);

private:
    std::string name_;
    std::string description_;
    int required_level_;
    EffectFn effect_;
    std::vector<const Skill*> prerequisites_;
    bool unlocked_ = false;
};

}  // namespace arch
