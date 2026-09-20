#pragma once

#include <cstdint>
#include <cmath>

namespace kfighter {

/// 2D integer vector for discrete coordinate math and box dimensions
struct Vec2i {
    int32_t x{0};
    int32_t y{0};

    constexpr Vec2i() = default;
    constexpr Vec2i(int32_t x_, int32_t y_) : x(x_), y(y_) {}

    constexpr Vec2i operator+(const Vec2i& o) const noexcept { return {x + o.x, y + o.y}; }
    constexpr Vec2i operator-(const Vec2i& o) const noexcept { return {x - o.x, y - o.y}; }
};

/// 2D floating-point vector for subpixel calculation
struct Vec2f {
    float x{0.0f};
    float y{0.0f};

    constexpr Vec2f() = default;
    constexpr Vec2f(float x_, float y_) : x(x_), y(y_) {}

    constexpr Vec2f operator+(const Vec2f& o) const noexcept { return {x + o.x, y + o.y}; }
    constexpr Vec2f operator-(const Vec2f& o) const noexcept { return {x - o.x, y - o.y}; }
    constexpr Vec2f operator*(float s) const noexcept { return {x * s, y * s}; }
};

/// 2.5D floating-point vector:
/// - x: Horizontal screen position
/// - y: Height above the ground (vertical / jump axis, positive is upward)
/// - z: Depth plane along the stage floor (positive is further into foreground)
struct Vec3f {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};

    constexpr Vec3f() = default;
    constexpr Vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    constexpr Vec3f operator+(const Vec3f& o) const noexcept { return {x + o.x, y + o.y, z + o.z}; }
    constexpr Vec3f operator-(const Vec3f& o) const noexcept { return {x - o.x, y - o.y, z - o.z}; }
    constexpr Vec3f operator*(float s) const noexcept { return {x * s, y * s, z * s}; }

    Vec3f& operator+=(const Vec3f& o) noexcept {
        x += o.x; y += o.y; z += o.z;
        return *this;
    }
};

/// Facing orientation: Facing::Right (1) or Facing::Left (-1)
enum class Facing : int8_t {
    Left = -1,
    Right = 1
};

/// Combat team allegiance
enum class Team : uint8_t {
    Independent = 0,
    Team1 = 1,
    Team2 = 2,
    Team3 = 3,
    Team4 = 4
};

/// Entity type classification (derived from LF2 object types)
enum class EntityType : uint8_t {
    Character = 0,
    LightWeapon = 1,
    HeavyWeapon = 2,
    Projectile = 3,
    ThrownItem = 4,
    Consumable = 5,
    Criminal = 6
};

} // namespace kfighter
