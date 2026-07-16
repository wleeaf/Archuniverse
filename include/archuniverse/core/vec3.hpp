#pragma once

#include <cmath>

namespace arch {

// A minimal 3D vector for world positions, movement, and range queries.
struct Vec3 {
    double x{0.0};
    double y{0.0};
    double z{0.0};

    friend constexpr Vec3 operator+(Vec3 a, Vec3 b) noexcept {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }
    friend constexpr Vec3 operator-(Vec3 a, Vec3 b) noexcept {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }
    friend constexpr Vec3 operator*(Vec3 v, double s) noexcept {
        return {v.x * s, v.y * s, v.z * s};
    }
    friend constexpr bool operator==(const Vec3&, const Vec3&) noexcept = default;

    [[nodiscard]] constexpr double length_squared() const noexcept {
        return x * x + y * y + z * z;
    }
    [[nodiscard]] double length() const noexcept { return std::sqrt(length_squared()); }

    [[nodiscard]] Vec3 normalized() const noexcept {
        const double len = length();
        return len > 0.0 ? Vec3{x / len, y / len, z / len} : Vec3{};
    }
};

[[nodiscard]] inline double distance(Vec3 a, Vec3 b) noexcept { return (a - b).length(); }
[[nodiscard]] constexpr double distance_squared(Vec3 a, Vec3 b) noexcept {
    return (a - b).length_squared();
}

}  // namespace arch
