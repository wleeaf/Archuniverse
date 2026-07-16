#pragma once

#include <istream>
#include <ostream>

#include "archuniverse/core/result.hpp"

namespace arch {

class World;

namespace persistence {

// A plain-text, tab-separated save format. save() writes the world's entities,
// items, progression, equipment, and positions; load() rebuilds them into a
// (preferably empty) world, re-applying equipment and re-unlocking skills so
// modifier-based bonuses are reconstructed rather than double-counted.
//
// Not persisted yet: transient status effects (bleed, active potion buffs) and
// code-attached item enchantments. Documented limitation.
void save(const World& world, std::ostream& out);
Result<> load(World& world, std::istream& in);

}  // namespace persistence
}  // namespace arch
