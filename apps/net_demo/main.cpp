#include <cstdint>
#include <iostream>
#include <thread>

#include "archuniverse/archuniverse.hpp"
#include "archuniverse/net/client.hpp"
#include "archuniverse/net/server.hpp"
#include "archuniverse/net/tcp_channel.hpp"

using namespace arch;

int main() {
    // Authoritative server world.
    World server_world{1};
    Character& hero =
        server_world.spawn_character("Izroth", Character::Sex::Male, 1, 500, 20, 20, 100, 100, 100, 1.0);
    hero.add_xp(2500);
    Weapon& sword = server_world.make_weapon("Longsword", Item::Grade::Rare, 400, 35, 5);
    hero.add_and_equip(sword);
    server_world.spawn_monster("Goblin", 3, 12, 8, 70, 40, 50, 0.9);

    net::TcpListener listener{0};  // ephemeral port
    if (!listener.valid()) {
        std::cerr << "server: failed to listen\n";
        return 1;
    }
    const std::uint16_t port = listener.port();
    std::cout << "server: listening on 127.0.0.1:" << port << "\n";

    // Client runs in its own thread and mirrors the server world.
    std::thread client_thread([port]() {
        auto channel = net::TcpChannel::connect("127.0.0.1", port);
        if (!channel) {
            std::cerr << "client: failed to connect\n";
            return;
        }
        World mirror{2};
        net::GameClient client(mirror, *channel);

        client.request_snapshot();
        client.apply_one();  // block for the snapshot and load it

        std::cout << "client: mirror has " << mirror.entity_count() << " entities, "
                  << mirror.item_count() << " items\n";
        for (const auto& e : mirror.entities()) {
            std::cout << "client:   " << e->name() << "  level " << e->level() << "  attack "
                      << e->melee_damage() << "\n";
        }
    });

    auto connection = listener.accept();
    if (!connection) {
        std::cerr << "server: accept failed\n";
        client_thread.join();
        return 1;
    }
    net::GameServer server(server_world, *connection);
    server.serve_one();  // handle the client's snapshot request

    client_thread.join();
    std::cout << "done\n";
    return 0;
}
