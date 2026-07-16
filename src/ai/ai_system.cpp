#include "archuniverse/ai/ai_system.hpp"

#include "archuniverse/ai/behavior.hpp"
#include "archuniverse/core/game_loop.hpp"
#include "archuniverse/entities/monster.hpp"
#include "archuniverse/world/world.hpp"

namespace arch {

AiSystem::AiSystem(World& world) : world_(world) { world_.loop().add(this); }

AiSystem::~AiSystem() { world_.loop().remove(this); }

void AiSystem::register_monster(Monster& monster) { monsters_.push_back(&monster); }

void AiSystem::tick(float dt) {
    for (Monster* monster : monsters_) {
        if (monster->alive() && monster->behavior() != nullptr)
            monster->behavior()->update(*monster, world_, dt);
    }
}

}  // namespace arch
