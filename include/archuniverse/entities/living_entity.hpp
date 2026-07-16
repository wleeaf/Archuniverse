#pragma once

#include <memory>
#include <string>
#include <vector>

#include "archuniverse/core/context.hpp"
#include "archuniverse/core/fight_type.hpp"
#include "archuniverse/core/id.hpp"
#include "archuniverse/core/result.hpp"
#include "archuniverse/core/stat.hpp"
#include "archuniverse/core/tickable.hpp"
#include "archuniverse/entities/status_effect.hpp"

namespace arch {

class SkillTree;

// The base for everything that lives, fights, regenerates, and levels up.
// Vitals and combat stats are Attributes/Vitals, so buffs and gear bonuses are
// applied as removable modifiers rather than hand-managed integer arithmetic.
class LivingEntity : public Tickable {
public:
    struct Action {
        FightType type;
        int amount;
    };

    LivingEntity(Context ctx, Id id, std::string name, int level, int base_attack,
                 int base_defence, int max_health = 100, int max_mana = 100,
                 int max_stamina = 100, double speed = 1.0);
    ~LivingEntity() override;

    LivingEntity(const LivingEntity&) = delete;
    LivingEntity& operator=(const LivingEntity&) = delete;

    // Identity
    [[nodiscard]] Id id() const noexcept { return id_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    // Vitals
    [[nodiscard]] Vital& health() noexcept { return health_; }
    [[nodiscard]] Vital& mana() noexcept { return mana_; }
    [[nodiscard]] Vital& stamina() noexcept { return stamina_; }
    [[nodiscard]] const Vital& health() const noexcept { return health_; }
    [[nodiscard]] const Vital& mana() const noexcept { return mana_; }
    [[nodiscard]] const Vital& stamina() const noexcept { return stamina_; }

    [[nodiscard]] Attribute& health_regen() noexcept { return health_regen_; }
    [[nodiscard]] Attribute& mana_regen() noexcept { return mana_regen_; }
    [[nodiscard]] Attribute& stamina_regen() noexcept { return stamina_regen_; }

    // Combat attributes
    [[nodiscard]] Attribute& base_attack() noexcept { return base_attack_; }
    [[nodiscard]] Attribute& base_defence() noexcept { return base_defence_; }
    [[nodiscard]] Attribute& melee() noexcept { return melee_; }
    [[nodiscard]] Attribute& magic() noexcept { return magic_; }

    // State
    [[nodiscard]] bool alive() const noexcept { return health_.current() > 0; }
    [[nodiscard]] bool dead() const noexcept { return !alive(); }
    [[nodiscard]] int level() const noexcept { return level_; }
    [[nodiscard]] int xp() const noexcept { return xp_; }
    [[nodiscard]] double speed() const noexcept { return speed_; }
    void set_speed(double s) noexcept { speed_ = s; }

    [[nodiscard]] SkillTree& skills() noexcept { return *skills_; }
    [[nodiscard]] Context& context() noexcept { return ctx_; }

    // Progression
    void add_xp(int amount);
    [[nodiscard]] static int xp_for_level(int level) noexcept;

    // Spend the resource a channel uses (Melee -> stamina, Magic -> mana).
    Status spend_for(FightType channel, int amount);

    // Damage / defence values. Virtual so Character can fold in gear.
    [[nodiscard]] virtual int melee_damage() const;
    [[nodiscard]] virtual int magic_damage() const;
    [[nodiscard]] virtual int melee_defence() const;
    [[nodiscard]] virtual int magic_defence() const;

    // Resource costs: monotonically cheaper with level, floored, never negative.
    [[nodiscard]] int attack_cost() const noexcept;
    [[nodiscard]] int defend_cost() const noexcept;

    // Pick the strongest affordable attack / defence given current resources.
    [[nodiscard]] virtual Action plan_attack() const;
    [[nodiscard]] virtual Action plan_defence() const;

    // Apply already-computed damage to health. Returns the amount actually lost.
    int apply_damage(int amount);

    void add_status(std::unique_ptr<StatusEffect> effect);
    [[nodiscard]] std::size_t status_count() const noexcept { return statuses_.size(); }

    void tick(float dt) override;

protected:
    void level_up();
    void level_up_from_xp();

    Context ctx_;
    Id id_;
    std::string name_;

    Vital health_;
    Vital mana_;
    Vital stamina_;
    Attribute health_regen_{1};
    Attribute mana_regen_{1};
    Attribute stamina_regen_{1};

    Attribute base_attack_{10};
    Attribute base_defence_{10};
    Attribute melee_{0};
    Attribute magic_{0};

    int xp_{0};
    int level_{1};
    double speed_{1.0};
    double x_{0.0};
    double y_{0.0};
    double z_{0.0};

    std::unique_ptr<SkillTree> skills_;
    std::vector<std::unique_ptr<StatusEffect>> statuses_;
    float regen_timer_{0.0f};
};

}  // namespace arch
