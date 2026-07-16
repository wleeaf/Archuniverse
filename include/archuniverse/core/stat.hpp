#pragma once

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <vector>

namespace arch {

// How a modifier folds into an attribute's final value.
enum class ModOp : std::uint8_t {
    Flat,        // add a flat amount
    PercentAdd,  // add this percent of the base (percents stack additively)
};

// Opaque handle returned when a modifier is added; used to remove it later.
struct ModifierId {
    std::uint64_t value{0};
    [[nodiscard]] constexpr bool valid() const noexcept { return value != 0; }
    friend constexpr bool operator==(ModifierId, ModifierId) noexcept = default;
};

// A numeric attribute: a base value plus a set of removable modifiers.
//
//   value = (base + sum(Flat)) * (1 + sum(PercentAdd) / 100)
//
// This is the spine of the whole engine: buffs, gear bonuses, and skill
// upgrades are all modifiers. Because each modifier has a handle, removing a
// bonus is exact, no bookkeeping drift when gear is swapped or a buff expires.
class Attribute {
public:
    Attribute() = default;
    constexpr explicit Attribute(int base) noexcept : base_{base} {}

    [[nodiscard]] int base() const noexcept { return base_; }
    void set_base(int value) noexcept {
        base_ = value;
        dirty_ = true;
    }

    [[nodiscard]] int value() const noexcept {
        if (dirty_) recompute();
        return cached_;
    }

    ModifierId add_modifier(ModOp op, double magnitude) {
        const ModifierId id{next_id_++};
        modifiers_.push_back({id, op, magnitude});
        dirty_ = true;
        return id;
    }

    bool remove_modifier(ModifierId id) {
        const auto it = std::ranges::find(modifiers_, id, &Modifier::id);
        if (it == modifiers_.end()) return false;
        modifiers_.erase(it);
        dirty_ = true;
        return true;
    }

    [[nodiscard]] std::size_t modifier_count() const noexcept { return modifiers_.size(); }

private:
    struct Modifier {
        ModifierId id;
        ModOp op;
        double magnitude;
    };

    void recompute() const noexcept {
        double flat = static_cast<double>(base_);
        double percent = 0.0;
        for (const auto& m : modifiers_) {
            if (m.op == ModOp::Flat)
                flat += m.magnitude;
            else
                percent += m.magnitude;
        }
        cached_ = static_cast<int>(flat * (1.0 + percent / 100.0));
        dirty_ = false;
    }

    int base_{0};
    std::vector<Modifier> modifiers_;
    mutable int cached_{0};
    mutable bool dirty_{true};
    std::uint64_t next_id_{1};
};

// A depletable pool (health / mana / stamina): a current value bounded by a
// modifiable maximum. Current is always clamped to [0, max]; because max is an
// Attribute, a temporary +max buff naturally restores headroom when it expires.
class Vital {
public:
    Vital() = default;
    Vital(int current, int max_value) : max_{max_value} {
        current_ = std::clamp(current, 0, max_value);
    }
    explicit Vital(int max_value) : Vital(max_value, max_value) {}

    [[nodiscard]] int current() const noexcept { return std::clamp(current_, 0, max()); }
    [[nodiscard]] int max() const noexcept { return max_.value(); }
    [[nodiscard]] Attribute& max_attribute() noexcept { return max_; }
    [[nodiscard]] const Attribute& max_attribute() const noexcept { return max_; }

    void set_current(int value) noexcept { current_ = std::clamp(value, 0, max()); }
    void damage(int amount) noexcept { set_current(current() - std::max(0, amount)); }
    void heal(int amount) noexcept { set_current(current() + std::max(0, amount)); }
    void refill() noexcept { current_ = max(); }
    void deplete() noexcept { current_ = 0; }

    [[nodiscard]] bool has_at_least(int amount) const noexcept { return current() >= amount; }
    [[nodiscard]] bool empty() const noexcept { return current() <= 0; }
    [[nodiscard]] bool full() const noexcept { return current() >= max(); }
    [[nodiscard]] double fraction() const noexcept {
        return max() > 0 ? static_cast<double>(current()) / static_cast<double>(max()) : 0.0;
    }

private:
    Attribute max_{100};
    int current_{100};
};

}  // namespace arch
