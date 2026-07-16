#pragma once

#include <string_view>

namespace arch {

class Monster;
class World;

// A monster's decision-making, invoked once per tick by the AiSystem with full
// World access (so it can find targets, move, and start fights).
class Behavior {
public:
    virtual ~Behavior() = default;
    virtual void update(Monster& self, World& world, float dt) = 0;
    [[nodiscard]] virtual std::string_view name() const = 0;
};

}  // namespace arch
