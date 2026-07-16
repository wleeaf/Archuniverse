#include "archuniverse/net/loopback.hpp"

namespace arch::net {

LoopbackPair::LoopbackPair() {
    client_.outbox = &client_to_server_;
    client_.inbox = &server_to_client_;
    server_.outbox = &server_to_client_;
    server_.inbox = &client_to_server_;
}

}  // namespace arch::net
