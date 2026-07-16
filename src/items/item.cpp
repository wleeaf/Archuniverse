#include "archuniverse/items/item.hpp"

#include <utility>

namespace arch {

Item::Item(Id id, std::string name, Kind kind, Grade grade, int worth)
    : id_(id), name_(std::move(name)), kind_(kind), grade_(grade), worth_(worth) {}

Item::~Item() = default;

void Item::add_effect(std::unique_ptr<Effect> effect) {
    if (effect) effects_.push_back(std::move(effect));
}

void Item::apply_effects() {
    for (auto& effect : effects_) effect->apply(*this);
}

void Item::revert_effects() {
    for (auto& effect : effects_) effect->revert(*this);
}

}  // namespace arch
