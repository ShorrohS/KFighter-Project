#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "types.hpp"
#include "hitbox.hpp"
#include "frame.hpp"

namespace kfighter {

/// Entity representation in 2.5D space (Fighter, Weapon, Projectile)
class Entity {
public:
    uint32_t id{0};
    std::string name{"Fighter"};
    Team team{Team::Team1};
    EntityType type{EntityType::Character};

    // 2.5D Spatial State
    Vec3f position{0.0f, 0.0f, 0.0f};  ///< x: horizontal, y: height above ground, z: depth
    Vec3f velocity{0.0f, 0.0f, 0.0f};  ///< Velocity vector
    Facing facing{Facing::Right};
    bool isGrounded{true};

    // Combat Attributes
    int32_t hp{500};
    int32_t maxHp{500};
    int32_t mp{500};
    int32_t maxMp{500};
    uint32_t invincibleTimer{0};        ///< Invincibility frames
    uint32_t arestTimer{0};             ///< Cooldown before next attack can connect
    uint32_t vrestTimer{0};             ///< Victim hitstun duration
    int32_t fallValue{0};               ///< Accumulated fall value (>= 60 triggers knockdown)
    uint32_t defenseShield{100};        ///< Guard gauge
    bool isDead{false};

    // Animation & State Tracking
    uint32_t currentFrameId{0};
    uint32_t waitTimer{0};
    std::vector<Frame> frames;

    // Input States
    bool inputUp{false};
    bool inputDown{false};
    bool inputLeft{false};
    bool inputRight{false};
    bool inputAttack{false};
    bool inputJump{false};
    bool inputDefend{false};

    // Input Trigger Pulses (one-tick triggers)
    bool triggerAttack{false};
    bool triggerJump{false};
    bool triggerDefend{false};

public:
    Entity() = default;
    Entity(uint32_t id_, std::string name_, Team team_, Vec3f pos);

    void setFrame(uint32_t frameId);
    [[nodiscard]] const Frame* getCurrentFrame() const;

    /// Advances the entity simulation by one fixed tick (typically 1/60th second)
    void tick(float dt = 1.0f / 60.0f);

    /// Applies an instant velocity impulse
    void applyImpulse(const Vec3f& impulse);

    /// Applies damage, knockback, and hitstun from an incoming attack
    void takeDamage(int32_t injury, const Vec3f& knockback, int32_t fall, uint32_t vrest);

    /// Computes world-space boxes for the current frame
    [[nodiscard]] std::vector<WorldBox> getWorldHitboxes() const;
    [[nodiscard]] std::vector<WorldBox> getWorldHurtboxes() const;
};

} // namespace kfighter
