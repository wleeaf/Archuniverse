#include "archuniverse/combat/combat_manager.hpp"

#include <algorithm>

#include "archuniverse/combat/combat.hpp"
#include "archuniverse/core/event_bus.hpp"
#include "archuniverse/core/events.hpp"
#include "archuniverse/entities/living_entity.hpp"

namespace arch {

CombatManager::CombatManager(Context ctx) : ctx_(ctx) {
    ctx_.bus.subscribe<CombatEnded>([](const CombatEnded& event) {
        if (event.winner != nullptr && event.loser != nullptr)
            event.winner->add_xp(200 * event.loser->level());
    });
}

CombatManager::~CombatManager() = default;

Combat& CombatManager::start(LivingEntity& a, LivingEntity& b) {
    auto combat = std::make_unique<Combat>(ctx_, a, b);
    Combat& ref = *combat;
    combats_.push_back(std::move(combat));
    return ref;
}

std::size_t CombatManager::active_count() const noexcept {
    return static_cast<std::size_t>(
        std::ranges::count_if(combats_, [](const std::unique_ptr<Combat>& c) {
            return !c->over();
        }));
}

}  // namespace arch
