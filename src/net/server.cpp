#include "archuniverse/net/server.hpp"

#include <charconv>
#include <sstream>

#include "archuniverse/combat/combat_manager.hpp"
#include "archuniverse/core/id.hpp"
#include "archuniverse/net/channel.hpp"
#include "archuniverse/net/protocol.hpp"
#include "archuniverse/persistence/snapshot.hpp"
#include "archuniverse/world/world.hpp"

namespace arch::net {
namespace {

bool parse_u64(const std::string& s, std::uint64_t& out) {
    return std::from_chars(s.data(), s.data() + s.size(), out).ec == std::errc{};
}

bool parse_float(const std::string& s, float& out) {
    return std::from_chars(s.data(), s.data() + s.size(), out).ec == std::errc{};
}

}  // namespace

GameServer::GameServer(World& world, Channel& channel) : world_(world), channel_(channel) {}

void GameServer::broadcast_snapshot() {
    std::ostringstream out;
    persistence::save(world_, out);
    channel_.send(std::string(verbs::kSnapshot) + "\n" + out.str());
}

void GameServer::handle(const std::string& raw) {
    const Message msg = parse_message(raw);
    const std::string_view verb = msg.verb();

    if (verb == verbs::kSnapshotRequest) {
        broadcast_snapshot();
    } else if (verb == verbs::kStep && msg.header.size() >= 2) {
        float seconds = 0.0f;
        if (parse_float(msg.header[1], seconds)) world_.run_for(seconds);
    } else if (verb == verbs::kStartCombat && msg.header.size() >= 3) {
        std::uint64_t a = 0;
        std::uint64_t b = 0;
        if (parse_u64(msg.header[1], a) && parse_u64(msg.header[2], b)) {
            LivingEntity* fa = world_.find_entity(Id{a});
            LivingEntity* fb = world_.find_entity(Id{b});
            if (fa != nullptr && fb != nullptr) world_.combat().start(*fa, *fb);
        }
    }
}

bool GameServer::serve_one() {
    auto message = channel_.receive();
    if (!message) return false;
    handle(*message);
    return true;
}

std::size_t GameServer::poll() {
    std::size_t handled = 0;
    while (auto message = channel_.receive()) {
        handle(*message);
        ++handled;
    }
    return handled;
}

}  // namespace arch::net
