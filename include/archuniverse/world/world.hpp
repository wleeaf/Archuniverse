#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "archuniverse/ai/ai_system.hpp"
#include "archuniverse/combat/combat_manager.hpp"
#include "archuniverse/core/context.hpp"
#include "archuniverse/core/event_bus.hpp"
#include "archuniverse/core/game_loop.hpp"
#include "archuniverse/core/id.hpp"
#include "archuniverse/core/rng.hpp"
#include "archuniverse/core/vec3.hpp"
#include "archuniverse/entities/character.hpp"
#include "archuniverse/entities/monster.hpp"
#include "archuniverse/items/armor.hpp"
#include "archuniverse/items/food.hpp"
#include "archuniverse/items/item.hpp"
#include "archuniverse/items/potion.hpp"
#include "archuniverse/items/ware.hpp"
#include "archuniverse/items/weapon.hpp"

namespace arch {

// The single owning context for a simulation. Bundles the shared services
// (loop, event bus, rng), hands out ids, and owns every entity and item. This
// replaces the original scattered SoulLedger / ItemManager / GameLoop
// singletons with one injectable object.
class World {
public:
    World() = default;
    explicit World(std::uint64_t seed) : rng_(seed) {}

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    [[nodiscard]] GameLoop& loop() noexcept { return loop_; }
    [[nodiscard]] EventBus& bus() noexcept { return bus_; }
    [[nodiscard]] Rng& rng() noexcept { return rng_; }
    [[nodiscard]] Context context() noexcept { return Context{loop_, bus_, rng_}; }
    [[nodiscard]] CombatManager& combat() noexcept { return combat_; }
    [[nodiscard]] AiSystem& ai() noexcept { return ai_; }

    // Entity factories (the World owns the returned reference).
    Character& spawn_character(std::string name, Character::Sex gender, int level, int gold,
                               int base_attack, int base_defence, int max_health = 100,
                               int max_mana = 100, int max_stamina = 100, double speed = 1.0);
    Monster& spawn_monster(std::string name, int level, int base_attack, int base_defence,
                           int max_health = 100, int max_mana = 100, int max_stamina = 100,
                           double speed = 1.0);

    // Item factories.
    Weapon& make_weapon(std::string name, Item::Grade grade, int worth, int attack, int defence);
    Armor& make_armor(std::string name, Item::Grade grade, int worth, int defence);
    Potion& make_potion(std::string name, Item::Grade grade, int worth, int health_boost,
                        int mana_boost, int stamina_boost, float effect_seconds);
    Food& make_food(std::string name, Item::Grade grade, int worth, int health_boost);
    Ware& make_ware(std::string name, Item::Grade grade, int worth);

    // Simulation control.
    void step(float dt) { loop_.step(dt); }
    void run_for(float seconds, float dt = GameLoop::kDefaultDt) { loop_.run_for(seconds, dt); }

    [[nodiscard]] std::size_t entity_count() const noexcept { return entities_.size(); }
    [[nodiscard]] std::size_t item_count() const noexcept { return items_.size(); }

    // Direct access to owned storage (iteration, spatial queries, persistence).
    [[nodiscard]] const std::vector<std::unique_ptr<LivingEntity>>& entities() const noexcept {
        return entities_;
    }
    [[nodiscard]] const std::vector<std::unique_ptr<Item>>& items() const noexcept {
        return items_;
    }

    [[nodiscard]] LivingEntity* find_entity(Id id) const {
        for (const auto& entity : entities_)
            if (entity->id() == id) return entity.get();
        return nullptr;
    }

    // Spatial queries.
    [[nodiscard]] std::vector<LivingEntity*> entities_within(const Vec3& center,
                                                             double radius) const;

    // Nearest living entity to a point satisfying pred (excludes the dead).
    template <class Pred>
    [[nodiscard]] LivingEntity* nearest(const Vec3& from, Pred pred) const {
        LivingEntity* best = nullptr;
        double best_dist = 0.0;
        for (const auto& entity : entities_) {
            if (entity->dead() || !pred(*entity)) continue;
            const double d = distance_squared(from, entity->position());
            if (best == nullptr || d < best_dist) {
                best = entity.get();
                best_dist = d;
            }
        }
        return best;
    }

private:
    GameLoop loop_;
    EventBus bus_;
    Rng rng_;
    IdGenerator ids_;
    CombatManager combat_{Context{loop_, bus_, rng_}};
    AiSystem ai_{*this};
    std::vector<std::unique_ptr<LivingEntity>> entities_;
    std::vector<std::unique_ptr<Item>> items_;
};

}  // namespace arch
