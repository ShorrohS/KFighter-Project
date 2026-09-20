#include "kfighter/world.hpp"
#include <algorithm>

namespace kfighter {

World::World() = default;

std::shared_ptr<Entity> World::spawnFighter(
    uint32_t id,
    const std::string& name,
    Team team,
    const Vec3f& position
) {
    auto fighter = std::make_shared<Entity>(id, name, team, position);
    entities.push_back(fighter);
    return fighter;
}

void World::tick(float dt) {
    recentEvents.clear();

    // 1. Advance individual entity simulations
    for (auto& entity : entities) {
        if (!entity) continue;
        entity->tick(dt);

        // Clamp entity position within arena boundary walls
        entity->position.x = std::clamp(entity->position.x, bounds.minX, bounds.maxX);
        entity->position.z = std::clamp(entity->position.z, bounds.minZ, bounds.maxZ);
    }

    // 2. Perform Pairwise Combat Collision Check
    const size_t count = entities.size();
    for (size_t i = 0; i < count; ++i) {
        auto& attacker = entities[i];
        if (!attacker || attacker->isDead || attacker->arestTimer > 0) continue;

        const Frame* attackerFrame = attacker->getCurrentFrame();
        if (!attackerFrame || attackerFrame->hitboxes.empty()) continue;

        for (size_t j = 0; j < count; ++j) {
            if (i == j) continue;
            auto& victim = entities[j];
            if (!victim || victim->isDead) continue;

            const Frame* victimFrame = victim->getCurrentFrame();
            if (!victimFrame || victimFrame->hurtboxes.empty()) continue;

            bool hitResolved = false;
            for (const auto& hitbox : attackerFrame->hitboxes) {
                if (hitResolved) break;

                for (const auto& hurtbox : victimFrame->hurtboxes) {
                    if (doesAttackConnect(*attacker, hitbox, *victim, hurtbox)) {
                        HitResult result = resolveCombatHit(*attacker, *victim, hitbox, hurtbox);
                        if (result.connected) {
                            CombatEvent evt;
                            evt.attackerId = attacker->id;
                            evt.victimId = victim->id;
                            evt.result = result;
                            evt.impactPosition = Vec3f(
                                (attacker->position.x + victim->position.x) * 0.5f,
                                (attacker->position.y + victim->position.y) * 0.5f + 30.0f,
                                (attacker->position.z + victim->position.z) * 0.5f
                            );
                            recentEvents.push_back(evt);
                            hitResolved = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    ++tickCount;
}

void World::cleanup() {
    entities.erase(
        std::remove_if(entities.begin(), entities.end(), [](const std::shared_ptr<Entity>& e) {
            return !e;
        }),
        entities.end()
    );
}

} // namespace kfighter
