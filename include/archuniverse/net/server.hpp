#pragma once

#include <cstddef>
#include <string>

namespace arch {

class World;

namespace net {

class Channel;

// Authoritative side: applies commands from a client to its World and streams
// back save-snapshots for replication.
class GameServer {
public:
    GameServer(World& world, Channel& channel);

    // Handle exactly one incoming message. Returns false when none is available
    // (or the peer closed). Use with a blocking transport.
    bool serve_one();

    // Drain all currently-available messages. Returns how many were handled.
    // Intended for non-blocking transports (loopback).
    std::size_t poll();

    // Serialize the world and send it to the client.
    void broadcast_snapshot();

private:
    void handle(const std::string& raw);

    World& world_;
    Channel& channel_;
};

}  // namespace net
}  // namespace arch
