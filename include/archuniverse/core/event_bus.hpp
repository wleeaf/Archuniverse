#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace arch {

// A minimal typed publish/subscribe bus. Subscribe with a handler for an event
// struct type E; publish an E and every matching handler runs synchronously.
//
//   bus.subscribe<EntityDied>([](const EntityDied& e) { ... });
//   bus.publish(EntityDied{&victim, &killer});
class EventBus {
public:
    template <class E>
    void subscribe(std::function<void(const E&)> handler) {
        handlers_[std::type_index(typeid(E))].emplace_back(
            [callback = std::move(handler)](const void* payload) {
                callback(*static_cast<const E*>(payload));
            });
    }

    template <class E>
    void publish(const E& event) const {
        const auto it = handlers_.find(std::type_index(typeid(E)));
        if (it == handlers_.end()) return;
        for (const auto& handler : it->second) handler(&event);
    }

    void clear() { handlers_.clear(); }

private:
    using ErasedHandler = std::function<void(const void*)>;
    std::unordered_map<std::type_index, std::vector<ErasedHandler>> handlers_;
};

}  // namespace arch
