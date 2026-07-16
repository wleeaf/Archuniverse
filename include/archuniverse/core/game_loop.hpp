#pragma once

#include <cstddef>
#include <vector>

namespace arch {

class Tickable;

// Fixed-timestep scheduler. Every registered Tickable is advanced by the same
// dt each step. Registration is safe during a tick: additions are queued and
// applied on the next step, removals are tombstoned and compacted, so an entity
// dying mid-tick (and unregistering itself) never invalidates the iteration.
class GameLoop {
public:
    static constexpr float kDefaultDt = 1.0f / 60.0f;

    void add(Tickable* t);
    void remove(Tickable* t);
    [[nodiscard]] bool contains(Tickable* t) const;
    [[nodiscard]] std::size_t size() const noexcept { return live_count_; }

    // Advance every tickable by dt exactly once.
    void step(float dt);

    // Deterministic headless run: floor(seconds / fixed_dt) steps, no sleeping.
    void run_for(float seconds, float fixed_dt = kDefaultDt);

    // Real-time run using a steady clock and an accumulator; blocks until stop().
    void run(float fixed_dt = kDefaultDt);
    void stop() noexcept { running_ = false; }

private:
    void integrate_pending();

    std::vector<Tickable*> tickables_;
    std::vector<Tickable*> pending_add_;
    std::size_t live_count_{0};
    bool running_{false};
};

}  // namespace arch
