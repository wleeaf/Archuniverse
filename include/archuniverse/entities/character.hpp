#pragma once

#include <vector>

#include "archuniverse/entities/living_entity.hpp"
#include "archuniverse/items/armor.hpp"
#include "archuniverse/items/item.hpp"
#include "archuniverse/items/weapon.hpp"

namespace arch {

// A playable being: a LivingEntity with inventory, equipment, gold, and trade.
class Character : public LivingEntity {
public:
    enum class Sex { Female, Male };

    Character(Context ctx, Id id, std::string name, Sex gender, int level, int gold,
              int base_attack, int base_defence, int max_health = 100, int max_mana = 100,
              int max_stamina = 100, double speed = 1.0);

    [[nodiscard]] Sex gender() const noexcept { return gender_; }
    void set_gender(Sex g) noexcept { gender_ = g; }
    [[nodiscard]] int gold() const noexcept { return gold_; }

    [[nodiscard]] const std::vector<Item*>& inventory() const noexcept { return inventory_; }
    [[nodiscard]] Weapon* equipped_weapon() const noexcept { return equipped_weapon_; }
    [[nodiscard]] Armor* equipped_armor() const noexcept { return equipped_armor_; }

    [[nodiscard]] int inventory_capacity() const noexcept { return inventory_capacity_; }
    void set_inventory_capacity(int capacity) noexcept { inventory_capacity_ = capacity; }
    [[nodiscard]] bool inventory_full() const noexcept {
        return static_cast<int>(inventory_.size()) >= inventory_capacity_;
    }
    [[nodiscard]] bool holds(const Item& item) const;

    // Inventory / equipment
    Result<> add_item(Item& item);
    Result<> remove_item(Item& item);
    Result<> equip(Item& item);
    Result<> unequip(Item& item);
    Result<> use(Item& item);
    Result<> add_and_equip(Item& item);

    // Economy
    Result<> transfer_item(Item& item, Character& other);
    Result<> transfer_gold(int amount, Character& other);
    Result<> sell_item(Item& item, int price, Character& buyer);
    Result<> buy_item(Item& item, int price, Character& seller);

    // Gear folds into the base combat values.
    [[nodiscard]] int melee_damage() const override;
    [[nodiscard]] int magic_damage() const override;
    [[nodiscard]] int melee_defence() const override;
    [[nodiscard]] int magic_defence() const override;

private:
    Sex gender_;
    int gold_;
    std::vector<Item*> inventory_;
    Weapon* equipped_weapon_ = nullptr;
    Armor* equipped_armor_ = nullptr;
    int inventory_capacity_ = 30;
};

}  // namespace arch
