#pragma once

#include <cstdint>
#include "types.hpp"

namespace kfighter {

/// Attack interaction kinds (derived from LF2 itr kind)
enum class AttackKind : uint32_t {
    Normal = 0,
    Catch = 1,
    Heal = 2,
    SuperPunch = 3,
    Forcefield = 4,
    Flute = 5,
    Float = 6,
    Fly = 7,
    Freeze = 8,
    Stop = 9,
    Thrown = 10
};

/// Hit effect aesthetics & elemental properties
enum class HitEffect : uint32_t {
    Normal = 0,
    Blood = 1,
    Fire = 2,
    Burn = 3,
    Freeze = 4,
    FreezeColumn = 5,
    Shrafe = 6,
    Lightning = 7
};

/// Interaction box (Hitbox) - represents an offensive or interactive area
struct Hitbox {
    AttackKind kind{AttackKind::Normal};
    int32_t x{0};           ///< Offset X relative to entity center
    int32_t y{0};           ///< Offset Y relative to entity center
    int32_t w{0};           ///< Width of the attack box
    int32_t h{0};           ///< Height of the attack box
    float dvx{0.0f};        ///< Knockback impulse X applied to target
    float dvy{0.0f};        ///< Knockback impulse Y applied to target
    int32_t fall{0};        ///< Fall accumulation (leads to knockdown when threshold passed)
    uint32_t arest{0};      ///< Attacker rest frames (cooldown before hitting again)
    uint32_t vrest{0};      ///< Victim rest frames (hitstun duration)
    HitEffect effect{HitEffect::Normal};
    int32_t injury{0};      ///< Damage dealt
    float zwidth{12.0f};    ///< 2.5D depth tolerance (attacker and victim z distance must be <= zwidth)
    uint32_t bdefend{0};    ///< Guard-break penetration rating
};

/// Body box (Hurtbox) - represents a vulnerable or physical contact area
struct Hurtbox {
    uint32_t kind{0};
    int32_t x{0};           ///< Offset X relative to entity center
    int32_t y{0};           ///< Offset Y relative to entity center
    int32_t w{0};           ///< Width of the hurtbox
    int32_t h{0};           ///< Height of the hurtbox
};

/// Computes an Axis-Aligned Bounding Box (AABB) in world space for a hitbox,
/// factoring in entity facing direction.
struct WorldBox {
    float left{0.0f};
    float right{0.0f};
    float top{0.0f};
    float bottom{0.0f};
    float z{0.0f};
    float zwidth{0.0f};

    [[nodiscard]] bool overlaps2D(const WorldBox& o) const noexcept {
        return left < o.right && right > o.left &&
               bottom < o.top && top > o.bottom;
    }

    [[nodiscard]] bool overlaps25D(const WorldBox& o) const noexcept {
        if (!overlaps2D(o)) return false;
        return std::abs(z - o.z) <= (zwidth + o.zwidth) * 0.5f;
    }
};

} // namespace kfighter
