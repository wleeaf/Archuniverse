#pragma once

#include <cstddef>
#include <string>

#include "archuniverse/core/id.hpp"

namespace arch {

class World;

namespace net {

class Channel;

// Client side: issues commands to the server and applies replicated snapshots
// into a local mirror World.
class GameClient {
public:
    GameClient(World& mirror, Channel& channel);

    void request_snapshot();
    void step(float seconds);
    void start_combat(Id a, Id b);

    // Apply one incoming message; false if none available. Blocking transport.
    bool apply_one();
    // Apply all available messages; returns how many. Non-blocking transport.
    std::size_t poll();

private:
    void apply(const std::string& raw);

    World& mirror_;
    Channel& channel_;
};

}  // namespace net
}  // namespace arch
