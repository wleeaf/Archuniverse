#include "archuniverse/core/game_loop.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

#include "archuniverse/core/tickable.hpp"

namespace arch {

void GameLoop::add(Tickable* t) {
    if (t != nullptr) pending_add_.push_back(t);
}

void GameLoop::remove(Tickable* t) {
    if (t == nullptr) return;
    for (auto& slot : tickables_) {
        if (slot == t) {
            slot = nullptr;  // tombstone; compacted after the current step
            if (live_count_ > 0) --live_count_;
        }
    }
    std::erase(pending_add_, t);  // cancel a not-yet-integrated add
}

bool GameLoop::contains(Tickable* t) const {
    if (t == nullptr) return false;
    if (std::ranges::find(tickables_, t) != tickables_.end()) return true;
    return std::ranges::find(pending_add_, t) != pending_add_.end();
}

void GameLoop::integrate_pending() {
    for (Tickable* t : pending_add_) {
        tickables_.push_back(t);
        ++live_count_;
    }
    pending_add_.clear();
}

void GameLoop::step(float dt) {
    integrate_pending();
    // Index-based: entries added during the tick land in pending_add_ (next
    // step), and removals only null existing slots, so this never invalidates.
    for (std::size_t i = 0; i < tickables_.size(); ++i) {
        if (Tickable* t = tickables_[i]) t->tick(dt);
    }
    std::erase(tickables_, nullptr);
}

void GameLoop::run_for(float seconds, float fixed_dt) {
    if (fixed_dt <= 0.0f || seconds <= 0.0f) return;
    // Round rather than truncate: 3.0f / (1.0f/60.0f) lands at 179.9999, and a
    // truncated 179 steps would silently drop the final tick of the interval.
    const long steps = std::lround(seconds / fixed_dt);
    for (long i = 0; i < steps; ++i) step(fixed_dt);
}

void GameLoop::run(float fixed_dt) {
    if (fixed_dt <= 0.0f) return;
    using clock = std::chrono::steady_clock;
    running_ = true;
    auto previous = clock::now();
    float accumulator = 0.0f;
    while (running_) {
        const auto now = clock::now();
        accumulator += std::chrono::duration<float>(now - previous).count();
        previous = now;
        while (accumulator >= fixed_dt) {
            step(fixed_dt);
            accumulator -= fixed_dt;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

}  // namespace arch
