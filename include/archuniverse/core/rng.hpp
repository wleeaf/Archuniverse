#pragma once

#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>

namespace arch {

// Seedable random source. Seed it explicitly for deterministic tests/replays.
class Rng {
public:
    Rng() : engine_(std::random_device{}()) {}
    explicit Rng(std::uint64_t seed) : engine_(seed) {}

    void reseed(std::uint64_t seed) { engine_.seed(seed); }

    // Inclusive integer range [lo, hi].
    [[nodiscard]] int range(int lo, int hi) {
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(engine_);
    }

    // Real range [lo, hi).
    [[nodiscard]] float range(float lo, float hi) {
        std::uniform_real_distribution<float> dist(lo, hi);
        return dist(engine_);
    }

    // [0.0, 1.0)
    [[nodiscard]] double unit() {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(engine_);
    }

    // True with probability p.
    [[nodiscard]] bool chance(double p = 0.5) {
        if (p <= 0.0) return false;
        if (p >= 1.0) return true;
        return unit() < p;
    }

    template <class T>
    [[nodiscard]] const T& pick(std::span<const T> items) {
        if (items.empty()) throw std::out_of_range("Rng::pick on empty span");
        return items[static_cast<std::size_t>(range(0, static_cast<int>(items.size()) - 1))];
    }

private:
    std::mt19937_64 engine_;
};

}  // namespace arch
