#include "archuniverse/items/food.hpp"

#include <utility>

#include "archuniverse/entities/character.hpp"

namespace arch {

Food::Food(Id id, std::string name, Grade grade, int worth, int health_boost)
    : Item(id, std::move(name), Kind::Food, grade, worth), health_boost_(health_boost) {}

Result<> Food::on_use() {
    if (owner_ == nullptr) return fail(GameError::ItemNotOwned);
    if (!owner_->holds(*this)) return fail(GameError::ItemNotInInventory);

    owner_->health().heal(health_boost_);
    used_ = true;
    return owner_->remove_item(*this);
}

}  // namespace arch
