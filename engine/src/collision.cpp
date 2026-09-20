#include "kfighter/collision.hpp"
#include <algorithm>
#include <cmath>

namespace kfighter {

static WorldBox computeHitboxWorldBox(const Entity& e, const Hitbox& box) noexcept {
    WorldBox wb;
    wb.z = e.position.z;
    wb.zwidth = box.zwidth;
    wb.bottom = e.position.y + static_cast<float>(box.y);
    wb.top = wb.bottom + static_cast<float>(box.h);

    if (e.facing == Facing::Right) {
        wb.left = e.position.x + static_cast<float>(box.x);
        wb.right = wb.left + static_cast<float>(box.w);
    } else {
        wb.right = e.position.x - static_cast<float>(box.x);
        wb.left = wb.right - static_cast<float>(box.w);
    }
    return wb;
}

static WorldBox computeHurtboxWorldBox(const Entity& e, const Hurtbox& box) noexcept {
    WorldBox wb;
    wb.z = e.position.z;
    wb.zwidth = 14.0f; // Default character hurtbox depth tolerance
    wb.bottom = e.position.y + static_cast<float>(box.y);
    wb.top = wb.bottom + static_cast<float>(box.h);

    if (e.facing == Facing::Right) {
        wb.left = e.position.x + static_cast<float>(box.x);
        wb.right = wb.left + static_cast<float>(box.w);
    } else {
        wb.right = e.position.x - static_cast<float>(box.x);
        wb.left = wb.right - static_cast<float>(box.w);
    }
    return wb;
}

bool doesAttackConnect(
    const Entity& attacker,
    const Hitbox& hitbox,
    const Entity& victim,
    const Hurtbox& hurtbox
) noexcept {
    // 1. Cannot attack self or dead entities
    if (attacker.id == victim.id || attacker.isDead || victim.isDead) {
        return false;
    }

    // 2. Attacker on hit cooldown
    if (attacker.arestTimer > 0) {
        return false;
    }

    // 3. Team check (Friendly fire is disallowed for normal attacks)
    if (attacker.team == victim.team && attacker.team != Team::Independent) {
        if (hitbox.kind != AttackKind::Heal) {
            return false;
        }
    }

    // 4. Invincibility check
    if (victim.invincibleTimer > 0) {
        if (hitbox.kind != AttackKind::Heal && hitbox.kind != AttackKind::Stop) {
            return false;
        }
    }

    // 5. Victim state check (cannot repeatedly strike grounded lying targets with normal attacks)
    const Frame* victimFrame = victim.getCurrentFrame();
    if (victimFrame && victimFrame->state == ActionState::Lying) {
        return false;
    }

    // 6. 2.5D World AABB Intersection Check
    WorldBox attackBox = computeHitboxWorldBox(attacker, hitbox);
    WorldBox hurtBox = computeHurtboxWorldBox(victim, hurtbox);

    return attackBox.overlaps25D(hurtBox);
}

HitResult resolveCombatHit(
    Entity& attacker,
    Entity& victim,
    const Hitbox& hitbox,
    const Hurtbox& hurtbox
) {
    HitResult result;
    if (!doesAttackConnect(attacker, hitbox, victim, hurtbox)) {
        return result;
    }

    result.connected = true;
    result.effect = hitbox.effect;

    // Check if victim is guarding
    const Frame* victimFrame = victim.getCurrentFrame();
    bool isGuarding = victim.inputDefend || (victimFrame && victimFrame->state == ActionState::Defend);

    // To successfully block, the victim must be facing toward the attack
    bool facingAttacker = (victim.position.x < attacker.position.x && victim.facing == Facing::Right) ||
                          (victim.position.x > attacker.position.x && victim.facing == Facing::Left);

    float dir = (attacker.facing == Facing::Right) ? 1.0f : -1.0f;

    if (isGuarding && facingAttacker && hitbox.kind == AttackKind::Normal) {
        result.wasBlocked = true;
        result.damageDealt = std::max(1, hitbox.injury / 4);
        result.knockback = Vec3f(dir * (hitbox.dvx * 0.3f), 0.0f, 0.0f);
        result.fallApplied = 0;
        result.victimStunTicks = hitbox.vrest / 2;
        result.attackerCooldownTicks = hitbox.arest > 0 ? hitbox.arest : 12;

        victim.takeDamage(result.damageDealt, result.knockback, 0, result.victimStunTicks);
    } else {
        result.wasBlocked = false;
        result.damageDealt = hitbox.injury;
        result.knockback = Vec3f(dir * hitbox.dvx, hitbox.dvy, 0.0f);
        result.fallApplied = hitbox.fall;
        result.victimStunTicks = hitbox.vrest > 0 ? hitbox.vrest : 16;
        result.attackerCooldownTicks = hitbox.arest > 0 ? hitbox.arest : 15;

        victim.takeDamage(result.damageDealt, result.knockback, result.fallApplied, result.victimStunTicks);
    }

    attacker.arestTimer = result.attackerCooldownTicks;
    return result;
}

} // namespace kfighter
