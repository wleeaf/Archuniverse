#include "archuniverse/persistence/snapshot.hpp"

#include <charconv>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "archuniverse/entities/character.hpp"
#include "archuniverse/entities/monster.hpp"
#include "archuniverse/items/armor.hpp"
#include "archuniverse/items/food.hpp"
#include "archuniverse/items/potion.hpp"
#include "archuniverse/items/ware.hpp"
#include "archuniverse/items/weapon.hpp"
#include "archuniverse/skills/skill_tree.hpp"
#include "archuniverse/world/world.hpp"

namespace arch::persistence {
namespace {

std::vector<std::string> split_tabs(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t tab = line.find('\t', start);
        if (tab == std::string::npos) {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, tab - start));
        start = tab + 1;
    }
    return fields;
}

bool to_ll(const std::string& s, long long& out) {
    const auto* first = s.data();
    const auto* last = s.data() + s.size();
    return std::from_chars(first, last, out).ec == std::errc{};
}

bool to_double(const std::string& s, double& out) {
    const auto* first = s.data();
    const auto* last = s.data() + s.size();
    return std::from_chars(first, last, out).ec == std::errc{};
}

int as_int(const std::string& s) {
    long long v = 0;
    to_ll(s, v);
    return static_cast<int>(v);
}

std::uint64_t as_u64(const std::string& s) {
    long long v = 0;
    to_ll(s, v);
    return static_cast<std::uint64_t>(v);
}

double as_double(const std::string& s) {
    double v = 0.0;
    to_double(s, v);
    return v;
}

// Serialize the common LivingEntity fields shared by CHAR and MONSTER lines.
void write_entity_stats(std::ostream& out, LivingEntity& e) {
    out << '\t' << e.level() << '\t' << e.xp() << '\t' << e.health().current() << '\t'
        << e.health().max_attribute().base() << '\t' << e.mana().current() << '\t'
        << e.mana().max_attribute().base() << '\t' << e.stamina().current() << '\t'
        << e.stamina().max_attribute().base() << '\t' << e.health_regen().base() << '\t'
        << e.mana_regen().base() << '\t' << e.stamina_regen().base() << '\t'
        << e.base_attack().base() << '\t' << e.base_defence().base() << '\t'
        << e.skills().ability(SkillTree::Ability::Melee) << '\t'
        << e.skills().ability(SkillTree::Ability::Magic) << '\t' << e.position().x << '\t'
        << e.position().y << '\t' << e.position().z << '\t' << e.speed();
}

// Restore those fields (in the order that keeps modifier-based bonuses correct:
// bases first, then abilities/skills, and current vitals last, by the caller).
struct EntityRow {
    LivingEntity* entity;
    int hp_current;
    int mp_current;
    int sp_current;
};

}  // namespace

void save(const World& world, std::ostream& out) {
    out << "ARCHUNIVERSE\t2\n";

    for (const auto& item_ptr : world.items()) {
        Item& item = *item_ptr;
        out << "ITEM\t" << item.id().value << '\t' << static_cast<int>(item.kind()) << '\t'
            << static_cast<int>(item.grade()) << '\t' << item.worth() << '\t' << item.name();
        switch (item.kind()) {
            case Item::Kind::Weapon: {
                auto& w = static_cast<Weapon&>(item);
                out << '\t' << w.attack_base() << '\t' << w.defence_base();
                break;
            }
            case Item::Kind::Armor: {
                auto& a = static_cast<Armor&>(item);
                out << '\t' << a.defence_base();
                break;
            }
            case Item::Kind::Potion: {
                auto& p = static_cast<Potion&>(item);
                out << '\t' << p.health_boost() << '\t' << p.mana_boost() << '\t'
                    << p.stamina_boost() << '\t' << p.effect_seconds();
                break;
            }
            case Item::Kind::Food: {
                auto& f = static_cast<Food&>(item);
                out << '\t' << f.health_boost();
                break;
            }
            case Item::Kind::Ware:
                break;
        }
        out << '\n';
    }

    for (const auto& ent_ptr : world.entities()) {
        LivingEntity& e = *ent_ptr;
        if (auto* c = dynamic_cast<Character*>(&e)) {
            out << "CHAR\t" << c->id().value << '\t' << c->name() << '\t'
                << static_cast<int>(c->gender()) << '\t' << c->gold();
            write_entity_stats(out, *c);
            out << '\n';
        } else {
            out << "MONSTER\t" << e.id().value << '\t' << e.name();
            write_entity_stats(out, e);
            out << '\n';
        }
    }

    // Progression: unused points and unlocked skills.
    for (const auto& ent_ptr : world.entities()) {
        LivingEntity& e = *ent_ptr;
        out << "POINTS\t" << e.id().value << '\t' << e.skills().unused_points() << '\n';
        for (const auto& skill : e.skills().skills()) {
            if (skill->unlocked()) out << "SKILL\t" << e.id().value << '\t' << skill->name() << '\n';
        }
    }

    // Inventory and equipment (characters only).
    for (const auto& ent_ptr : world.entities()) {
        auto* c = dynamic_cast<Character*>(ent_ptr.get());
        if (c == nullptr) continue;
        out << "INV\t" << c->id().value;
        for (const Item* item : c->inventory()) out << '\t' << item->id().value;
        out << '\n';
        out << "EQUIP\t" << c->id().value << '\t'
            << (c->equipped_weapon() ? c->equipped_weapon()->id().value : 0) << '\t'
            << (c->equipped_armor() ? c->equipped_armor()->id().value : 0) << '\n';
    }

    out << "END\n";
}

Result<> load(World& world, std::istream& in) {
    std::unordered_map<std::uint64_t, Item*> items_by_id;
    std::unordered_map<std::uint64_t, Character*> chars_by_id;
    std::unordered_map<std::uint64_t, LivingEntity*> entities_by_id;
    std::vector<EntityRow> pending_vitals;

    std::string line;
    if (!std::getline(in, line)) return fail(GameError::Cancelled);
    const auto header = split_tabs(line);
    if (header.size() < 2 || header[0] != "ARCHUNIVERSE") return fail(GameError::Cancelled);

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        const auto f = split_tabs(line);
        const std::string& tag = f[0];

        if (tag == "END") {
            break;
        } else if (tag == "ITEM" && f.size() >= 6) {
            const std::uint64_t old_id = as_u64(f[1]);
            const auto kind = static_cast<Item::Kind>(as_int(f[2]));
            const auto grade = static_cast<Item::Grade>(as_int(f[3]));
            const int worth = as_int(f[4]);
            const std::string& name = f[5];
            Item* created = nullptr;
            switch (kind) {
                case Item::Kind::Weapon:
                    if (f.size() >= 8)
                        created = &world.make_weapon(name, grade, worth, as_int(f[6]), as_int(f[7]));
                    break;
                case Item::Kind::Armor:
                    if (f.size() >= 7)
                        created = &world.make_armor(name, grade, worth, as_int(f[6]));
                    break;
                case Item::Kind::Potion:
                    if (f.size() >= 10)
                        created = &world.make_potion(name, grade, worth, as_int(f[6]), as_int(f[7]),
                                                     as_int(f[8]),
                                                     static_cast<float>(as_double(f[9])));
                    break;
                case Item::Kind::Food:
                    if (f.size() >= 7)
                        created = &world.make_food(name, grade, worth, as_int(f[6]));
                    break;
                case Item::Kind::Ware:
                    created = &world.make_ware(name, grade, worth);
                    break;
            }
            if (created == nullptr) return fail(GameError::Cancelled);
            items_by_id[old_id] = created;
        } else if (tag == "CHAR" && f.size() >= 24) {
            const std::uint64_t old_id = as_u64(f[1]);
            const std::string& name = f[2];
            const auto gender = static_cast<Character::Sex>(as_int(f[3]));
            const int gold = as_int(f[4]);
            // f[5..]: level xp hpcur hpbase mpcur mpbase spcur spbase hpreg mpreg
            //         spreg atk def melee magic x y z speed
            Character& c = world.spawn_character(name, gender, as_int(f[5]), gold, as_int(f[16]),
                                                 as_int(f[17]), as_int(f[8]), as_int(f[10]),
                                                 as_int(f[12]), as_double(f[23]));
            c.set_xp(as_int(f[6]));
            c.health_regen().set_base(as_int(f[13]));
            c.mana_regen().set_base(as_int(f[14]));
            c.stamina_regen().set_base(as_int(f[15]));
            c.skills().set_ability(SkillTree::Ability::Melee, as_int(f[18]));
            c.skills().set_ability(SkillTree::Ability::Magic, as_int(f[19]));
            c.set_position(Vec3{as_double(f[20]), as_double(f[21]), as_double(f[22])});
            chars_by_id[old_id] = &c;
            entities_by_id[old_id] = &c;
            pending_vitals.push_back({&c, as_int(f[7]), as_int(f[9]), as_int(f[11])});
        } else if (tag == "MONSTER" && f.size() >= 22) {
            const std::uint64_t old_id = as_u64(f[1]);
            const std::string& name = f[2];
            // f[3..]: level xp hpcur hpbase mpcur mpbase spcur spbase hpreg mpreg
            //         spreg atk def melee magic x y z speed
            Monster& m = world.spawn_monster(name, as_int(f[3]), as_int(f[14]), as_int(f[15]),
                                             as_int(f[6]), as_int(f[8]), as_int(f[10]),
                                             as_double(f[21]));
            m.set_xp(as_int(f[4]));
            m.health_regen().set_base(as_int(f[11]));
            m.mana_regen().set_base(as_int(f[12]));
            m.stamina_regen().set_base(as_int(f[13]));
            m.skills().set_ability(SkillTree::Ability::Melee, as_int(f[16]));
            m.skills().set_ability(SkillTree::Ability::Magic, as_int(f[17]));
            m.set_position(Vec3{as_double(f[18]), as_double(f[19]), as_double(f[20])});
            entities_by_id[old_id] = &m;
            pending_vitals.push_back({&m, as_int(f[5]), as_int(f[7]), as_int(f[9])});
        } else if (tag == "POINTS" && f.size() >= 3) {
            if (auto it = entities_by_id.find(as_u64(f[1])); it != entities_by_id.end())
                it->second->skills().set_unused_points(as_int(f[2]));
        } else if (tag == "SKILL" && f.size() >= 3) {
            if (auto it = entities_by_id.find(as_u64(f[1])); it != entities_by_id.end())
                (void)it->second->skills().force_unlock(f[2]);
        } else if (tag == "INV" && f.size() >= 2) {
            if (auto it = chars_by_id.find(as_u64(f[1])); it != chars_by_id.end()) {
                for (std::size_t i = 2; i < f.size(); ++i) {
                    if (auto item_it = items_by_id.find(as_u64(f[i])); item_it != items_by_id.end())
                        (void)it->second->add_item(*item_it->second);
                }
            }
        } else if (tag == "EQUIP" && f.size() >= 4) {
            auto it = chars_by_id.find(as_u64(f[1]));
            if (it == chars_by_id.end()) continue;
            Character& c = *it->second;
            for (const std::string& field : {f[2], f[3]}) {
                const std::uint64_t item_id = as_u64(field);
                if (item_id == 0) continue;
                if (auto item_it = items_by_id.find(item_id); item_it != items_by_id.end())
                    (void)c.add_and_equip(*item_it->second);
            }
        }
    }

    // Current vitals applied last, once max-boosting skills are re-unlocked.
    for (const EntityRow& row : pending_vitals) {
        row.entity->health().set_current(row.hp_current);
        row.entity->mana().set_current(row.mp_current);
        row.entity->stamina().set_current(row.sp_current);
    }

    return ok();
}

}  // namespace arch::persistence
