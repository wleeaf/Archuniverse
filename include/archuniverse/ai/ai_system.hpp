#pragma once

#include <cstddef>
#include <vector>

#include "archuniverse/core/tickable.hpp"

namespace arch {

class Monster;
class World;

// Drives every registered monster's Behavior once per tick. Owned by the World
// and registered on the loop like any other tickable.
class AiSystem : public Tickable {
public:
    explicit AiSystem(World& world);
    ~AiSystem() override;

    AiSystem(const AiSystem&) = delete;
    AiSystem& operator=(const AiSystem&) = delete;

    void register_monster(Monster& monster);
    void tick(float dt) override;

    [[nodiscard]] std::size_t count() const noexcept { return monsters_.size(); }

private:
    World& world_;
    std::vector<Monster*> monsters_;
};

}  // namespace arch
