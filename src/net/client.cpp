#include "archuniverse/net/client.hpp"

#include <sstream>
#include <string>

#include "archuniverse/net/channel.hpp"
#include "archuniverse/net/protocol.hpp"
#include "archuniverse/persistence/snapshot.hpp"
#include "archuniverse/world/world.hpp"

namespace arch::net {

GameClient::GameClient(World& mirror, Channel& channel) : mirror_(mirror), channel_(channel) {}

void GameClient::request_snapshot() { channel_.send(std::string(verbs::kSnapshotRequest)); }

void GameClient::step(float seconds) {
    channel_.send(std::string(verbs::kStep) + "\t" + std::to_string(seconds));
}

void GameClient::start_combat(Id a, Id b) {
    channel_.send(std::string(verbs::kStartCombat) + "\t" + std::to_string(a.value) + "\t" +
                  std::to_string(b.value));
}

void GameClient::apply(const std::string& raw) {
    const Message msg = parse_message(raw);
    if (msg.verb() == verbs::kSnapshot) {
        std::istringstream in(msg.payload);
        (void)persistence::load(mirror_, in);
    }
}

bool GameClient::apply_one() {
    auto message = channel_.receive();
    if (!message) return false;
    apply(*message);
    return true;
}

std::size_t GameClient::poll() {
    std::size_t applied = 0;
    while (auto message = channel_.receive()) {
        apply(*message);
        ++applied;
    }
    return applied;
}

}  // namespace arch::net
