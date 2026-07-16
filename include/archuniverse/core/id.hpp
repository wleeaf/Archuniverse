#pragma once

#include <cstdint>
#include <functional>

namespace arch {

// A lightweight, monotonically assigned identity for any world object.
struct Id {
    std::uint64_t value{0};

    [[nodiscard]] constexpr bool valid() const noexcept { return value != 0; }
    friend constexpr auto operator<=>(const Id&, const Id&) = default;
};

inline constexpr Id kInvalidId{0};

// Hands out unique ids. One instance lives in the World.
class IdGenerator {
public:
    [[nodiscard]] Id next() noexcept { return Id{++counter_}; }

private:
    std::uint64_t counter_{0};
};

}  // namespace arch

template <>
struct std::hash<arch::Id> {
    std::size_t operator()(const arch::Id& id) const noexcept {
        return std::hash<std::uint64_t>{}(id.value);
    }
};
