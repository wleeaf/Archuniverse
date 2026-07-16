#pragma once

// Umbrella header: pulls in the whole public engine surface. Convenient for
// apps and tests; internal translation units should include only what they use.

#include "archuniverse/core/context.hpp"
#include "archuniverse/core/event_bus.hpp"
#include "archuniverse/core/events.hpp"
#include "archuniverse/core/fight_type.hpp"
#include "archuniverse/core/game_loop.hpp"
#include "archuniverse/core/id.hpp"
#include "archuniverse/core/result.hpp"
#include "archuniverse/core/rng.hpp"
#include "archuniverse/core/stat.hpp"
#include "archuniverse/core/tickable.hpp"

#include "archuniverse/entities/character.hpp"
#include "archuniverse/entities/living_entity.hpp"
#include "archuniverse/entities/monster.hpp"
#include "archuniverse/entities/status_effect.hpp"

#include "archuniverse/items/armor.hpp"
#include "archuniverse/items/effect.hpp"
#include "archuniverse/items/food.hpp"
#include "archuniverse/items/item.hpp"
#include "archuniverse/items/potion.hpp"
#include "archuniverse/items/special_effects.hpp"
#include "archuniverse/items/ware.hpp"
#include "archuniverse/items/weapon.hpp"

#include "archuniverse/skills/skill.hpp"
#include "archuniverse/skills/skill_tree.hpp"

#include "archuniverse/combat/bleed.hpp"
#include "archuniverse/combat/combat.hpp"
#include "archuniverse/combat/combat_manager.hpp"

#include "archuniverse/world/world.hpp"
