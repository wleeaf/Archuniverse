#pragma once

#include <deque>
#include <optional>
#include <string>

#include "archuniverse/net/channel.hpp"

namespace arch::net {

// Two in-process channels wired mouth-to-ear: what the client sends, the server
// receives, and vice versa. For single-process play and deterministic tests.
class LoopbackPair {
public:
    LoopbackPair();

    LoopbackPair(const LoopbackPair&) = delete;
    LoopbackPair& operator=(const LoopbackPair&) = delete;

    [[nodiscard]] Channel& client() noexcept { return client_; }
    [[nodiscard]] Channel& server() noexcept { return server_; }

private:
    class Endpoint : public Channel {
    public:
        std::deque<std::string>* outbox = nullptr;
        std::deque<std::string>* inbox = nullptr;

        void send(const std::string& message) override { outbox->push_back(message); }
        std::optional<std::string> receive() override {
            if (inbox->empty()) return std::nullopt;
            std::string message = std::move(inbox->front());
            inbox->pop_front();
            return message;
        }
    };

    std::deque<std::string> client_to_server_;
    std::deque<std::string> server_to_client_;
    Endpoint client_;
    Endpoint server_;
};

}  // namespace arch::net
