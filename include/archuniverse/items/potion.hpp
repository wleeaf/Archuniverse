#pragma once

#include "archuniverse/core/tickable.hpp"
#include "archuniverse/items/item.hpp"

namespace arch {

class GameLoop;

// A consumable that grants an immediate vitals boost and keeps any timed
// special effects active for effect_seconds before reverting them.
class Potion : public Item, public Tickable {
public:
    Potion(Id id, std::string name, Grade grade, int worth, int health_boost,
           int mana_boost, int stamina_boost, float effect_seconds);
    ~Potion() override;

    Result<> on_use() override;
    void tick(float dt) override;

    [[nodiscard]] int health_boost() const noexcept { return health_boost_; }
    [[nodiscard]] int mana_boost() const noexcept { return mana_boost_; }
    [[nodiscard]] int stamina_boost() const noexcept { return stamina_boost_; }
    [[nodiscard]] float effect_seconds() const noexcept { return effect_seconds_; }
    [[nodiscard]] bool in_effect() const noexcept { return in_effect_; }

    // Who drank it. Captured on use and kept for the buff's lifetime, even after
    // the potion leaves the inventory (which clears the plain owner pointer).
    [[nodiscard]] Character* consumer() const noexcept { return consumer_; }

private:
    int health_boost_;
    int mana_boost_;
    int stamina_boost_;
    float effect_seconds_;
    bool in_effect_ = false;
    float elapsed_ = 0.0f;
    Character* consumer_ = nullptr;
    GameLoop* loop_ = nullptr;  // set while ticking, for self-deregistration
};

}  // namespace arch
