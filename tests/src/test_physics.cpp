#include <cassert>
#include <iostream>
#include <cmath>
#include "kfighter/world.hpp"

using namespace kfighter;

void testGravityAndLanding() {
    std::cout << "[TEST] Gravity & Ground Landing... ";
    World world;
    auto fighter = world.spawnFighter(1, "Jumper", Team::Team1, Vec3f(0.0f, 50.0f, 0.0f));
    fighter->isGrounded = false;
    fighter->velocity.y = 0.0f;

    // Simulate 30 ticks of gravity falling
    for (int i = 0; i < 30; ++i) {
        world.tick();
    }

    assert(fighter->position.y == 0.0f && "Fighter should land exactly on ground plane y = 0");
    assert(fighter->isGrounded && "Fighter should be marked grounded");
    assert(fighter->velocity.y == 0.0f && "Vertical velocity should be stopped on landing");

    std::cout << "PASSED\n";
}

void testFrictionDeceleration() {
    std::cout << "[TEST] Ground Friction Deceleration... ";
    World world;
    auto fighter = world.spawnFighter(1, "Runner", Team::Team1, Vec3f(0.0f, 0.0f, 0.0f));
    fighter->velocity.x = 10.0f;

    for (int i = 0; i < 20; ++i) {
        world.tick();
    }

    assert(fighter->velocity.x < 0.2f && "Fighter horizontal velocity should decay toward 0 due to friction");
    std::cout << "PASSED\n";
}

void testBoundaryClamping() {
    std::cout << "[TEST] Arena Boundary Clamping... ";
    World world;
    world.bounds.maxX = 100.0f;
    world.bounds.minX = -100.0f;

    auto fighter = world.spawnFighter(1, "Dasher", Team::Team1, Vec3f(90.0f, 0.0f, 0.0f));
    fighter->velocity.x = 50.0f; // Excessive speed past boundary

    world.tick();

    assert(fighter->position.x <= world.bounds.maxX && "Entity must not exceed maxX arena boundary");
    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== Running KFighter Physics Unit Tests ===\n";
    testGravityAndLanding();
    testFrictionDeceleration();
    testBoundaryClamping();
    std::cout << "=== All 3 Physics Tests Passed Successfully! ===\n";
    return 0;
}
