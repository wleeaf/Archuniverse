#include "doctest.h"

#include <string>

#include "archuniverse/archuniverse.hpp"

using namespace arch;

namespace {

Character* find_character(World& world, const std::string& name) {
    for (const auto& e : world.entities())
        if (auto* c = dynamic_cast<Character*>(e.get()); c != nullptr && c->name() == name) return c;
    return nullptr;
}

}  // namespace

TEST_CASE("protocol splits a header line and keeps the payload intact") {
    const net::Message msg = net::parse_message("SNAPSHOT\nline1\tcol\nline2\n");
    CHECK(msg.verb() == "SNAPSHOT");
    CHECK(msg.payload == "line1\tcol\nline2\n");

    const net::Message cmd = net::parse_message("START_COMBAT\t3\t7");
    CHECK(cmd.verb() == "START_COMBAT");
    REQUIRE(cmd.header.size() == 3);
    CHECK(cmd.header[1] == "3");
    CHECK(cmd.header[2] == "7");
    CHECK(cmd.payload.empty());
}

TEST_CASE("client mirrors the server world over a loopback channel") {
    World server_world{1};
    Character& hero =
        server_world.spawn_character("Izroth", Character::Sex::Male, 1, 500, 20, 20, 100, 100, 100, 1.0);
    hero.add_xp(2500);
    Weapon& sword = server_world.make_weapon("Longsword", Item::Grade::Rare, 400, 35, 5);
    hero.add_and_equip(sword);
    server_world.spawn_monster("Goblin", 3, 12, 8, 70, 40, 50, 0.9);

    net::LoopbackPair link;
    net::GameServer server(server_world, link.server());
    World mirror{2};
    net::GameClient client(mirror, link.client());

    client.request_snapshot();
    CHECK(server.poll() == 1);   // request handled -> snapshot enqueued
    CHECK(client.poll() == 1);   // snapshot applied

    CHECK(mirror.entity_count() == server_world.entity_count());
    CHECK(mirror.item_count() == server_world.item_count());

    Character* mirrored = find_character(mirror, "Izroth");
    REQUIRE(mirrored != nullptr);
    CHECK(mirrored->level() == hero.level());
    CHECK(mirrored->melee_damage() == hero.melee_damage());
    REQUIRE(mirrored->equipped_weapon() != nullptr);
    CHECK(mirrored->equipped_weapon()->name() == "Longsword");
}

TEST_CASE("a client command is applied on the server") {
    World server_world{1};
    Character& a =
        server_world.spawn_character("A", Character::Sex::Male, 3, 0, 30, 10, 100, 100, 100, 1.0);
    Character& b =
        server_world.spawn_character("B", Character::Sex::Male, 1, 0, 10, 5, 60, 100, 100, 1.0);

    net::LoopbackPair link;
    net::GameServer server(server_world, link.server());
    World mirror{2};
    net::GameClient client(mirror, link.client());

    CHECK(server_world.combat().total_count() == 0);
    client.start_combat(a.id(), b.id());
    server.poll();
    CHECK(server_world.combat().total_count() == 1);

    // A STEP command advances the authoritative simulation.
    client.step(60.0f);
    server.poll();
    CHECK(server_world.combat().active_count() == 0);  // the fight resolved
}
