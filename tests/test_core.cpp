#include "doctest.h"

#include "archuniverse/core/event_bus.hpp"
#include "archuniverse/core/game_loop.hpp"
#include "archuniverse/core/rng.hpp"
#include "archuniverse/core/stat.hpp"
#include "archuniverse/core/tickable.hpp"

using namespace arch;

TEST_CASE("Attribute folds flat and percent modifiers") {
    Attribute atk{100};
    CHECK(atk.value() == 100);

    const ModifierId flat = atk.add_modifier(ModOp::Flat, 20);   // 120
    const ModifierId pct = atk.add_modifier(ModOp::PercentAdd, 50);  // *1.5 -> 180
    CHECK(atk.value() == 180);

    CHECK(atk.remove_modifier(pct));
    CHECK(atk.value() == 120);

    CHECK(atk.remove_modifier(flat));
    CHECK(atk.value() == 100);

    CHECK_FALSE(atk.remove_modifier(flat));  // already gone
}

TEST_CASE("Vital clamps to [0, max] and tracks a modifiable maximum") {
    Vital hp{100};
    CHECK(hp.full());

    hp.damage(30);
    CHECK(hp.current() == 70);

    hp.heal(1000);  // clamps at max
    CHECK(hp.current() == 100);

    hp.damage(1000);  // clamps at 0
    CHECK(hp.empty());

    // A +max buff restores headroom; removing it clamps back down.
    hp.refill();
    const ModifierId buff = hp.max_attribute().add_modifier(ModOp::Flat, 50);
    CHECK(hp.max() == 150);
    hp.refill();
    CHECK(hp.current() == 150);
    hp.max_attribute().remove_modifier(buff);
    CHECK(hp.current() == 100);  // clamped by the restored max
}

namespace {
struct Counter : Tickable {
    int ticks = 0;
    void tick(float) override { ++ticks; }
};
}  // namespace

TEST_CASE("GameLoop advances registered tickables deterministically") {
    GameLoop loop;
    Counter a;
    Counter b;
    loop.add(&a);
    loop.add(&b);

    loop.run_for(1.0f, 0.1f);  // 10 steps
    CHECK(a.ticks == 10);
    CHECK(b.ticks == 10);
    CHECK(loop.size() == 2);

    loop.remove(&b);
    loop.step(0.1f);
    CHECK(a.ticks == 11);
    CHECK(b.ticks == 10);  // no longer ticked
    CHECK(loop.size() == 1);
}

TEST_CASE("EventBus delivers to typed subscribers") {
    struct Ping {
        int n;
    };
    EventBus bus;
    int total = 0;
    bus.subscribe<Ping>([&](const Ping& p) { total += p.n; });
    bus.subscribe<Ping>([&](const Ping& p) { total += p.n; });
    bus.publish(Ping{5});
    CHECK(total == 10);
}

TEST_CASE("Rng is reproducible for a fixed seed") {
    Rng a{42};
    Rng b{42};
    for (int i = 0; i < 100; ++i) CHECK(a.range(0, 1000) == b.range(0, 1000));
}
