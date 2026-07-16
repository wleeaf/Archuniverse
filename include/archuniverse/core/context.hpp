#pragma once

namespace arch {

class GameLoop;
class EventBus;
class Rng;

// The shared services every live object needs, passed by reference instead of
// reached through globals. The World owns the backing instances.
struct Context {
    GameLoop& loop;
    EventBus& bus;
    Rng& rng;
};

}  // namespace arch
