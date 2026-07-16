#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "archuniverse/core/context.hpp"

namespace arch {

class Combat;
class LivingEntity;

// Owns the lifetime of active duels and awards XP when they end (subscribed to
// CombatEnded on the bus, so the reward rule lives in one place).
class CombatManager {
public:
    explicit CombatManager(Context ctx);
    ~CombatManager();  // defined where Combat is complete (owns unique_ptr<Combat>)

    CombatManager(const CombatManager&) = delete;
    CombatManager& operator=(const CombatManager&) = delete;

    Combat& start(LivingEntity& a, LivingEntity& b);

    [[nodiscard]] std::size_t active_count() const noexcept;
    [[nodiscard]] std::size_t total_count() const noexcept { return combats_.size(); }

private:
    Context ctx_;
    std::vector<std::unique_ptr<Combat>> combats_;
};

}  // namespace arch
