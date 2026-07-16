#pragma once

#include "archuniverse/ai/behavior.hpp"
#include "archuniverse/core/vec3.hpp"

namespace arch {

// Does nothing; the default disposition.
class IdleBehavior : public Behavior {
public:
    void update(Monster&, World&, float) override {}
    [[nodiscard]] std::string_view name() const override { return "Idle"; }
};

// Random walk within a radius of a home point.
class WanderBehavior : public Behavior {
public:
    WanderBehavior(Vec3 home, double radius, double speed)
        : home_(home), radius_(radius), speed_(speed) {}

    void update(Monster& self, World& world, float dt) override;
    [[nodiscard]] std::string_view name() const override { return "Wander"; }

private:
    Vec3 home_;
    Vec3 target_{};
    double radius_;
    double speed_;
    bool has_target_ = false;
};

// Chase the nearest living character and open combat once within engage range.
class HunterBehavior : public Behavior {
public:
    HunterBehavior(double speed, double engage_range)
        : speed_(speed), engage_range_(engage_range) {}

    void update(Monster& self, World& world, float dt) override;
    [[nodiscard]] std::string_view name() const override { return "Hunter"; }
    [[nodiscard]] bool engaged() const noexcept { return engaged_; }

private:
    double speed_;
    double engage_range_;
    bool engaged_ = false;
};

}  // namespace arch
