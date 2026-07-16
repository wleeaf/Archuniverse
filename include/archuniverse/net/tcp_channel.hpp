#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "archuniverse/net/channel.hpp"

namespace arch::net {

// A length-framed (4-byte big-endian prefix) blocking TCP channel. receive()
// blocks for one whole message and returns nullopt on EOF or error.
class TcpChannel : public Channel {
public:
    explicit TcpChannel(int fd) noexcept : fd_(fd) {}
    ~TcpChannel() override;

    TcpChannel(TcpChannel&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    TcpChannel& operator=(TcpChannel&& other) noexcept;
    TcpChannel(const TcpChannel&) = delete;
    TcpChannel& operator=(const TcpChannel&) = delete;

    void send(const std::string& message) override;
    std::optional<std::string> receive() override;

    [[nodiscard]] bool valid() const noexcept { return fd_ >= 0; }

    static std::optional<TcpChannel> connect(const std::string& host, std::uint16_t port);

private:
    int fd_;
};

// A loopback-bound listening socket that accepts one client at a time.
class TcpListener {
public:
    explicit TcpListener(std::uint16_t port);
    ~TcpListener();

    TcpListener(const TcpListener&) = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    [[nodiscard]] bool valid() const noexcept { return fd_ >= 0; }
    [[nodiscard]] std::uint16_t port() const noexcept { return port_; }  // resolved if 0 requested

    std::optional<TcpChannel> accept();

private:
    int fd_;
    std::uint16_t port_ = 0;
};

}  // namespace arch::net
