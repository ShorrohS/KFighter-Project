#pragma once

#include "entity.hpp"
#include "hitbox.hpp"

namespace kfighter {

/// Result of an attack collision check
struct HitResult {
    bool connected{false};
    bool wasBlocked{false};
    int32_t damageDealt{0};
    Vec3f knockback{0.0f, 0.0f, 0.0f};
    int32_t fallApplied{0};
    uint32_t victimStunTicks{0};
    uint32_t attackerCooldownTicks{0};
    HitEffect effect{HitEffect::Normal};
};

/// Evaluates whether an offensive hitbox from an attacker connects with a victim's hurtbox.
/// Directly implements the reverse-engineered rules from LF2's func_417400_does_attack_success.
[[nodiscard]] bool doesAttackConnect(
    const Entity& attacker,
    const Hitbox& hitbox,
    const Entity& victim,
    const Hurtbox& hurtbox
) noexcept;

/// Resolves the collision event, updating attacker cooldown, applying damage/knockback to victim,
/// and returning the calculated hit details.
HitResult resolveCombatHit(
    Entity& attacker,
    Entity& victim,
    const Hitbox& hitbox,
    const Hurtbox& hurtbox
);

} // namespace kfighter
