#pragma once

#include <optional>
#include <string>

namespace arch::net {

// A bidirectional message pipe. Each send()/receive() moves one whole message
// (framing is the transport's concern). receive() returns nullopt when no
// message is available (loopback) or the peer has closed (TCP).
class Channel {
public:
    virtual ~Channel() = default;
    virtual void send(const std::string& message) = 0;
    virtual std::optional<std::string> receive() = 0;
};

}  // namespace arch::net
