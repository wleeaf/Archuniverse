#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace arch::net {

// Wire messages are text. The first line is a tab-separated header (verb + args);
// anything after the first newline is an opaque payload (used to carry a save
// snapshot). This keeps commands trivially printable and lets a snapshot's own
// tabs/newlines ride along untouched.
struct Message {
    std::vector<std::string> header;
    std::string payload;

    [[nodiscard]] std::string_view verb() const {
        return header.empty() ? std::string_view{} : std::string_view{header[0]};
    }
};

inline Message parse_message(const std::string& raw) {
    Message msg;
    const std::size_t newline = raw.find('\n');
    const std::string head = (newline == std::string::npos) ? raw : raw.substr(0, newline);
    if (newline != std::string::npos) msg.payload = raw.substr(newline + 1);

    std::size_t start = 0;
    while (true) {
        const std::size_t tab = head.find('\t', start);
        if (tab == std::string::npos) {
            msg.header.push_back(head.substr(start));
            break;
        }
        msg.header.push_back(head.substr(start, tab - start));
        start = tab + 1;
    }
    return msg;
}

// Verb constants shared by client and server.
namespace verbs {
inline constexpr std::string_view kSnapshotRequest = "SNAPSHOT_REQUEST";
inline constexpr std::string_view kSnapshot = "SNAPSHOT";
inline constexpr std::string_view kStep = "STEP";
inline constexpr std::string_view kStartCombat = "START_COMBAT";
}  // namespace verbs

}  // namespace arch::net
