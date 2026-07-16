#include "archuniverse/items/potion.hpp"

#include <utility>

#include "archuniverse/core/game_loop.hpp"
#include "archuniverse/entities/character.hpp"

namespace arch {

Potion::Potion(Id id, std::string name, Grade grade, int worth, int health_boost, int mana_boost,
               int stamina_boost, float effect_seconds)
    : Item(id, std::move(name), Kind::Potion, grade, worth),
      health_boost_(health_boost),
      mana_boost_(mana_boost),
      stamina_boost_(stamina_boost),
      effect_seconds_(effect_seconds) {}

Potion::~Potion() {
    if (loop_ != nullptr) loop_->remove(this);
}

Result<> Potion::on_use() {
    if (owner_ == nullptr) return fail(GameError::ItemNotOwned);
    if (!owner_->holds(*this)) return fail(GameError::ItemNotInInventory);

    consumer_ = owner_;
    owner_->health().heal(health_boost_);
    owner_->mana().heal(mana_boost_);
    owner_->stamina().heal(stamina_boost_);

    used_ = true;
    in_effect_ = true;
    apply_effects();  // timed buffs read consumer_

    (void)owner_->remove_item(*this);  // leaves the bag; stays alive in the World

    loop_ = &consumer_->context().loop;
    loop_->add(this);
    return ok();
}

void Potion::tick(float dt) {
    if (!in_effect_) return;
    elapsed_ += dt;
    if (elapsed_ >= effect_seconds_) {
        in_effect_ = false;
        revert_effects();
        if (loop_ != nullptr) {
            loop_->remove(this);
            loop_ = nullptr;
        }
    }
}

}  // namespace arch
