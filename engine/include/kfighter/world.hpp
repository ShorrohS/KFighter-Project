#pragma once

#include <vector>
#include <memory>
#include "entity.hpp"
#include "collision.hpp"

namespace kfighter {

/// Represents an arena combat event (hits, impacts, guards)
struct CombatEvent {
    uint32_t attackerId{0};
    uint32_t victimId{0};
    HitResult result;
    Vec3f impactPosition{0.0f, 0.0f, 0.0f};
};

/// 2.5D World boundaries
struct ArenaBounds {
    float minX{-300.0f};
    float maxX{300.0f};
    float minZ{-80.0f};
    float maxZ{80.0f};
    float floorY{0.0f};
};

/// Master World simulation manager
class World {
public:
    ArenaBounds bounds;
    float gravity{-0.75f};             ///< Downward acceleration per tick
    float groundFriction{0.88f};        ///< Horizontal ground damping per tick
    uint64_t tickCount{0};

    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<CombatEvent> recentEvents;

public:
    World();

    std::shared_ptr<Entity> spawnFighter(
        uint32_t id,
        const std::string& name,
        Team team,
        const Vec3f& position
    );

    /// Advances the entire simulation by one fixed tick
    void tick(float dt = 1.0f / 60.0f);

    /// Clears any dead entities or expired particles
    void cleanup();

    [[nodiscard]] const std::vector<CombatEvent>& getRecentEvents() const noexcept {
        return recentEvents;
    }
};

} // namespace kfighter
