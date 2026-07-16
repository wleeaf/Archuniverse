#pragma once

namespace arch {

// Tolerance for float time accumulation when firing periodic (per-second)
// effects. Summing a dt like 1/60f across a whole second drifts a hair under
// 1.0, so an exact ">= 1.0" test would drop the tick at the boundary. This
// absorbs that drift without introducing any long-term timing error.
inline constexpr float kTickEpsilon = 1e-4f;

// Anything that advances with the game clock. Driven by GameLoop.
class Tickable {
public:
    virtual ~Tickable() = default;
    virtual void tick(float dt) = 0;
};

}  // namespace arch
