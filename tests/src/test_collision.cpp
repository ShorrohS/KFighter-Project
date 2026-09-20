#include <cassert>
#include <iostream>
#include "kfighter/types.hpp"
#include "kfighter/hitbox.hpp"
#include "kfighter/entity.hpp"
#include "kfighter/collision.hpp"
#include "kfighter/world.hpp"

using namespace kfighter;

void testDepthFiltering() {
    std::cout << "[TEST] 2.5D Depth Filtering... ";
    Entity attacker(1, "Attacker", Team::Team1, Vec3f(0.0f, 0.0f, 0.0f));
    Entity victimInDepth(2, "VictimNear", Team::Team2, Vec3f(20.0f, 0.0f, 5.0f));
    Entity victimFarDepth(3, "VictimFar", Team::Team2, Vec3f(20.0f, 0.0f, 35.0f));

    // Put attacker into attack frame (frame 3)
    attacker.setFrame(3);
    const Frame* f = attacker.getCurrentFrame();
    assert(f != nullptr && !f->hitboxes.empty());
    const Hitbox& hb = f->hitboxes[0];

    const Frame* vf = victimInDepth.getCurrentFrame();
    assert(vf != nullptr && !vf->hurtboxes.empty());
    const Hurtbox& hurtbox = vf->hurtboxes[0];

    // Near victim in depth should connect
    bool nearConnects = doesAttackConnect(attacker, hb, victimInDepth, hurtbox);
    assert(nearConnects && "Attack should connect when z-distance is within tolerance");

    // Far victim in depth should NOT connect
    bool farConnects = doesAttackConnect(attacker, hb, victimFarDepth, hurtbox);
    assert(!farConnects && "Attack must NOT connect when z-distance exceeds zwidth tolerance");

    std::cout << "PASSED\n";
}

void testFriendlyFire() {
    std::cout << "[TEST] Friendly Fire Check... ";
    Entity p1(1, "Player1", Team::Team1, Vec3f(0.0f, 0.0f, 0.0f));
    Entity ally(2, "Ally", Team::Team1, Vec3f(20.0f, 0.0f, 0.0f));
    Entity enemy(3, "Enemy", Team::Team2, Vec3f(20.0f, 0.0f, 0.0f));

    p1.setFrame(3);
    const Hitbox& hb = p1.getCurrentFrame()->hitboxes[0];
    const Hurtbox& hurt = ally.getCurrentFrame()->hurtboxes[0];

    // Same team: should not connect
    assert(!doesAttackConnect(p1, hb, ally, hurt) && "Normal attack must not hit teammates");

    // Enemy team: connects
    assert(doesAttackConnect(p1, hb, enemy, hurt) && "Normal attack must connect with enemy");

    std::cout << "PASSED\n";
}

void testInvulnerability() {
    std::cout << "[TEST] Invulnerability Frame Immunity... ";
    Entity p1(1, "Player1", Team::Team1, Vec3f(0.0f, 0.0f, 0.0f));
    Entity enemy(2, "Enemy", Team::Team2, Vec3f(20.0f, 0.0f, 0.0f));
    enemy.invincibleTimer = 30; // 30 ticks of invincibility

    p1.setFrame(3);
    const Hitbox& hb = p1.getCurrentFrame()->hitboxes[0];
    const Hurtbox& hurt = enemy.getCurrentFrame()->hurtboxes[0];

    assert(!doesAttackConnect(p1, hb, enemy, hurt) && "Invulnerable target must ignore normal attack");

    std::cout << "PASSED\n";
}

void testDamageAndKnockback() {
    std::cout << "[TEST] Damage & Knockback Resolution... ";
    Entity p1(1, "Player1", Team::Team1, Vec3f(0.0f, 0.0f, 0.0f));
    Entity enemy(2, "Enemy", Team::Team2, Vec3f(20.0f, 0.0f, 0.0f));
    int32_t startingHp = enemy.hp;

    p1.setFrame(3);
    const Hitbox& hb = p1.getCurrentFrame()->hitboxes[0];
    const Hurtbox& hurt = enemy.getCurrentFrame()->hurtboxes[0];

    HitResult res = resolveCombatHit(p1, enemy, hb, hurt);
    assert(res.connected && "Hit must be resolved");
    assert(!res.wasBlocked && "Hit was unblocked");
    assert(enemy.hp == startingHp - hb.injury && "HP must be deducted precisely");
    assert(enemy.velocity.x > 0.0f && "Enemy must receive positive X knockback from right-facing attacker");
    assert(p1.arestTimer > 0 && "Attacker must receive cooldown arest timer");

    std::cout << "PASSED\n";
}

void testBlockingMechanics() {
    std::cout << "[TEST] Guard & Block Mechanics... ";
    Entity p1(1, "Player1", Team::Team1, Vec3f(0.0f, 0.0f, 0.0f));
    Entity enemy(2, "Enemy", Team::Team2, Vec3f(20.0f, 0.0f, 0.0f));
    int32_t startingHp = enemy.hp;

    // Enemy faces attacker (facing Left) and holds guard
    enemy.facing = Facing::Left;
    enemy.inputDefend = true;

    p1.setFrame(3);
    const Hitbox& hb = p1.getCurrentFrame()->hitboxes[0];
    const Hurtbox& hurt = enemy.getCurrentFrame()->hurtboxes[0];

    HitResult res = resolveCombatHit(p1, enemy, hb, hurt);
    assert(res.connected && res.wasBlocked && "Attack should be marked as blocked");
    assert(res.damageDealt < hb.injury && "Blocked damage should be reduced");
    assert(enemy.hp == startingHp - res.damageDealt && "HP must deduct reduced blocked damage");
    assert(res.fallApplied == 0 && "Blocking must prevent knockdown fall accumulation");

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== Running KFighter Collision Unit Tests ===\n";
    testDepthFiltering();
    testFriendlyFire();
    testInvulnerability();
    testDamageAndKnockback();
    testBlockingMechanics();
    std::cout << "=== All 5 Collision Tests Passed Successfully! ===\n";
    return 0;
}
