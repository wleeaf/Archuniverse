#include "archuniverse/entities/monster.hpp"

#include <utility>

#include "archuniverse/ai/behavior.hpp"

namespace arch {

Monster::~Monster() = default;

void Monster::set_behavior(std::unique_ptr<Behavior> behavior) {
    behavior_ = std::move(behavior);
}

}  // namespace arch
