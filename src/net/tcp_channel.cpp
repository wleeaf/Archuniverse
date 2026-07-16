#include "archuniverse/net/tcp_channel.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstddef>

namespace arch::net {
namespace {

bool write_all(int fd, const char* data, std::size_t length) {
    std::size_t sent = 0;
    while (sent < length) {
        const ssize_t n = ::send(fd, data + sent, length - sent, 0);
        if (n <= 0) return false;
        sent += static_cast<std::size_t>(n);
    }
    return true;
}

bool read_all(int fd, char* data, std::size_t length) {
    std::size_t got = 0;
    while (got < length) {
        const ssize_t n = ::recv(fd, data + got, length - got, 0);
        if (n <= 0) return false;
        got += static_cast<std::size_t>(n);
    }
    return true;
}

}  // namespace

TcpChannel::~TcpChannel() {
    if (fd_ >= 0) ::close(fd_);
}

TcpChannel& TcpChannel::operator=(TcpChannel&& other) noexcept {
    if (this != &other) {
        if (fd_ >= 0) ::close(fd_);
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

void TcpChannel::send(const std::string& message) {
    if (fd_ < 0) return;
    const std::uint32_t prefix = htonl(static_cast<std::uint32_t>(message.size()));
    if (!write_all(fd_, reinterpret_cast<const char*>(&prefix), sizeof(prefix))) return;
    write_all(fd_, message.data(), message.size());
}

std::optional<std::string> TcpChannel::receive() {
    if (fd_ < 0) return std::nullopt;
    std::uint32_t prefix = 0;
    if (!read_all(fd_, reinterpret_cast<char*>(&prefix), sizeof(prefix))) return std::nullopt;
    const std::uint32_t length = ntohl(prefix);
    std::string buffer(length, '\0');
    if (length > 0 && !read_all(fd_, buffer.data(), length)) return std::nullopt;
    return buffer;
}

std::optional<TcpChannel> TcpChannel::connect(const std::string& host, std::uint16_t port) {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return std::nullopt;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        ::close(fd);
        return std::nullopt;
    }
    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return std::nullopt;
    }
    return TcpChannel{fd};
}

TcpListener::TcpListener(std::uint16_t port) : fd_(::socket(AF_INET, SOCK_STREAM, 0)) {
    if (fd_ < 0) return;

    int yes = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd_);
        fd_ = -1;
        return;
    }
    if (::listen(fd_, 1) < 0) {
        ::close(fd_);
        fd_ = -1;
        return;
    }

    sockaddr_in resolved{};
    socklen_t len = sizeof(resolved);
    if (::getsockname(fd_, reinterpret_cast<sockaddr*>(&resolved), &len) == 0)
        port_ = ntohs(resolved.sin_port);
}

TcpListener::~TcpListener() {
    if (fd_ >= 0) ::close(fd_);
}

std::optional<TcpChannel> TcpListener::accept() {
    if (fd_ < 0) return std::nullopt;
    const int client = ::accept(fd_, nullptr, nullptr);
    if (client < 0) return std::nullopt;
    return TcpChannel{client};
}

}  // namespace arch::net
