#include "archuniverse/entities/character.hpp"

#include <algorithm>
#include <utility>

namespace arch {

Character::Character(Context ctx, Id id, std::string name, Sex gender, int level, int gold,
                     int base_attack, int base_defence, int max_health, int max_mana,
                     int max_stamina, double speed)
    : LivingEntity(ctx, id, std::move(name), level, base_attack, base_defence, max_health,
                   max_mana, max_stamina, speed),
      gender_(gender),
      gold_(gold) {}

bool Character::holds(const Item& item) const {
    return std::ranges::find(inventory_, &item) != inventory_.end();
}

Result<> Character::add_item(Item& item) {
    if (holds(item)) return fail(GameError::ItemAlreadyOwned);
    if (inventory_full()) return fail(GameError::InventoryFull);
    inventory_.push_back(&item);
    item.set_owner(this);
    return ok();
}

Result<> Character::remove_item(Item& item) {
    if (item.owner() != this) return fail(GameError::ItemNotOwned);
    const auto it = std::ranges::find(inventory_, &item);
    if (it == inventory_.end()) return fail(GameError::ItemNotInInventory);
    inventory_.erase(it);
    item.set_owner(nullptr);
    return ok();
}

Result<> Character::equip(Item& item) {
    if (item.owner() != this) return fail(GameError::ItemNotOwned);
    if (!holds(item)) return fail(GameError::ItemNotInInventory);
    if (!item.equippable()) return fail(GameError::NotEquippable);

    if (item.kind() == Item::Kind::Weapon) {
        if (equipped_weapon_ != nullptr) (void)unequip(*equipped_weapon_);
        std::erase(inventory_, &item);
        equipped_weapon_ = static_cast<Weapon*>(&item);
    } else if (item.kind() == Item::Kind::Armor) {
        if (equipped_armor_ != nullptr) (void)unequip(*equipped_armor_);
        std::erase(inventory_, &item);
        equipped_armor_ = static_cast<Armor*>(&item);
    } else {
        return fail(GameError::NotEquippable);
    }

    item.set_equipped(true);
    item.apply_effects();  // installs enchantment modifiers
    return ok();
}

Result<> Character::unequip(Item& item) {
    if (item.owner() != this) return fail(GameError::ItemNotOwned);
    if (!item.equipped()) return fail(GameError::Cancelled);

    if (equipped_weapon_ == &item)
        equipped_weapon_ = nullptr;
    else if (equipped_armor_ == &item)
        equipped_armor_ = nullptr;
    else
        return fail(GameError::Cancelled);

    item.revert_effects();  // removes exactly the modifiers it installed
    item.set_equipped(false);
    inventory_.push_back(&item);
    return ok();
}

Result<> Character::use(Item& item) {
    if (item.owner() != this) return fail(GameError::ItemNotOwned);
    if (!holds(item)) return fail(GameError::ItemNotInInventory);
    return item.on_use();
}

Result<> Character::add_and_equip(Item& item) {
    if (auto r = add_item(item); !r) return r;
    return equip(item);
}

Result<> Character::transfer_item(Item& item, Character& other) {
    if (item.owner() != this) return fail(GameError::ItemNotOwned);
    if (!holds(item)) return fail(GameError::ItemNotInInventory);
    if (other.inventory_full()) return fail(GameError::InventoryFull);
    std::erase(inventory_, &item);
    item.set_owner(nullptr);
    return other.add_item(item);
}

Result<> Character::transfer_gold(int amount, Character& other) {
    if (amount < 0) return fail(GameError::Cancelled);
    if (gold_ < amount) return fail(GameError::InsufficientGold);
    gold_ -= amount;
    other.gold_ += amount;
    return ok();
}

Result<> Character::sell_item(Item& item, int price, Character& buyer) {
    if (item.owner() != this) return fail(GameError::ItemNotOwned);
    if (!holds(item)) return fail(GameError::ItemNotInInventory);
    if (buyer.gold_ < price) return fail(GameError::InsufficientGold);
    if (buyer.inventory_full()) return fail(GameError::InventoryFull);
    if (auto r = buyer.transfer_gold(price, *this); !r) return r;
    std::erase(inventory_, &item);
    item.set_owner(nullptr);
    return buyer.add_item(item);
}

Result<> Character::buy_item(Item& item, int price, Character& seller) {
    return seller.sell_item(item, price, *this);
}

int Character::melee_damage() const {
    int total = LivingEntity::melee_damage();
    if (equipped_weapon_ != nullptr) total += equipped_weapon_->attack_value();
    return total;
}

int Character::magic_damage() const {
    int total = LivingEntity::magic_damage();
    if (equipped_weapon_ != nullptr) total += equipped_weapon_->attack_value();
    return total;
}

int Character::melee_defence() const {
    int total = LivingEntity::melee_defence();
    if (equipped_armor_ != nullptr) total += equipped_armor_->defence_value();
    return total;
}

int Character::magic_defence() const {
    int total = LivingEntity::magic_defence();
    if (equipped_armor_ != nullptr) total += equipped_armor_->defence_value();
    return total;
}

}  // namespace arch
