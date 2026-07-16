#include "archuniverse/items/special_effects.hpp"

#include "archuniverse/entities/character.hpp"
#include "archuniverse/items/armor.hpp"
#include "archuniverse/items/potion.hpp"
#include "archuniverse/items/weapon.hpp"

namespace arch {

void Aggressiveness::apply(Item& item) {
    if (auto* weapon = dynamic_cast<Weapon*>(&item)) {
        handle_ = weapon->attack().add_modifier(ModOp::PercentAdd, percent_);
        applied_ = true;
    }
}

void Aggressiveness::revert(Item& item) {
    if (!applied_) return;
    if (auto* weapon = dynamic_cast<Weapon*>(&item)) weapon->attack().remove_modifier(handle_);
    applied_ = false;
}

void Solidness::apply(Item& item) {
    if (auto* armor = dynamic_cast<Armor*>(&item)) {
        handle_ = armor->defence().add_modifier(ModOp::PercentAdd, percent_);
        applied_ = true;
    }
}

void Solidness::revert(Item& item) {
    if (!applied_) return;
    if (auto* armor = dynamic_cast<Armor*>(&item)) armor->defence().remove_modifier(handle_);
    applied_ = false;
}

void HealthRegenBoost::apply(Item& item) {
    auto* potion = dynamic_cast<Potion*>(&item);
    if (potion == nullptr) return;
    if (auto* c = potion->consumer()) {
        handle_ = c->health_regen().add_modifier(ModOp::PercentAdd, 100);
        applied_ = true;
    }
}

void HealthRegenBoost::revert(Item& item) {
    if (!applied_) return;
    auto* potion = dynamic_cast<Potion*>(&item);
    if (potion == nullptr) return;
    if (auto* c = potion->consumer()) c->health_regen().remove_modifier(handle_);
    applied_ = false;
}

}  // namespace arch
