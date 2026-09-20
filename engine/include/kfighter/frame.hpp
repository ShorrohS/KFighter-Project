#pragma once

#include <cstdint>
#include <vector>
#include "types.hpp"
#include "hitbox.hpp"

namespace kfighter {

/// Action state classification (derived from LF2 frame states)
enum class ActionState : uint32_t {
    Standing = 0,
    Walking = 1,
    Running = 2,
    HeavyAttack = 3,
    NormalAttack = 4,
    Jumping = 5,
    Dash = 6,
    Defend = 7,
    BrokenDefend = 8,
    Catching = 9,
    Caught = 10,
    Injured = 11,
    Falling = 12,
    Ice = 13,
    Fire = 14,
    BurnRun = 15,
    Lying = 16
};

/// Object / projectile spawn anchor
struct ObjectPoint {
    uint32_t kind{0};
    int32_t x{0};
    int32_t y{0};
    uint32_t action{0};
    float dvx{0.0f};
    float dvy{0.0f};
    uint32_t oid{0};
    int32_t facing{0};
};

/// Blood / impact spark emitter anchor
struct BloodPoint {
    int32_t x{0};
    int32_t y{0};
};

/// Weapon socket anchor
struct WeaponPoint {
    uint32_t kind{0};
    int32_t x{0};
    int32_t y{0};
    uint32_t weaponact{0};
    bool attacking{false};
    uint32_t cover{0};
};

/// Animation frame specification
struct Frame {
    uint32_t id{0};
    uint32_t picIndex{0};
    ActionState state{ActionState::Standing};
    uint32_t wait{4};             ///< Duration of this frame in ticks (e.g. 4 ticks = ~66ms at 60Hz)
    int32_t next{0};              ///< Frame ID to transition to when wait expires (-1 or 0 for loop/idle)
    
    // Inherent velocities for this frame
    float dvx{0.0f};
    float dvy{0.0f};
    float dvz{0.0f};

    // Frame origin anchor points
    int32_t centerx{40};
    int32_t centery{75};

    // Input branch targets (transition to these frames when respective inputs are triggered)
    int32_t hit_a{-1};            ///< Attack button pressed
    int32_t hit_d{-1};            ///< Defend button pressed
    int32_t hit_j{-1};            ///< Jump button pressed
    int32_t hit_Fa{-1};           ///< Forward + Attack
    int32_t hit_Ua{-1};           ///< Up + Attack
    int32_t hit_Da{-1};           ///< Down + Attack
    int32_t hit_Fj{-1};           ///< Forward + Jump
    int32_t hit_Uj{-1};           ///< Up + Jump
    int32_t hit_Dj{-1};           ///< Down + Jump
    int32_t hit_ja{-1};           ///< Jump + Attack (mid-air strike)

    // Attached collision geometry
    std::vector<Hitbox> hitboxes;
    std::vector<Hurtbox> hurtboxes;

    ObjectPoint opoint{};
    BloodPoint bpoint{};
    WeaponPoint wpoint{};
};

} // namespace kfighter
