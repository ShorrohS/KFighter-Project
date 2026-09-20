#include "kfighter/entity.hpp"
#include <algorithm>
#include <cmath>

namespace kfighter {

static std::vector<Frame> createDefaultFighterFrames() {
    std::vector<Frame> frames;
    frames.resize(10);

    // Frame 0: Idle / Standing
    frames[0].id = 0;
    frames[0].state = ActionState::Standing;
    frames[0].wait = 8;
    frames[0].next = 0;
    frames[0].hit_a = 2; // Attack
    frames[0].hit_j = 4; // Jump
    frames[0].hit_d = 5; // Defend
    // Body hurtbox (covers head to feet)
    frames[0].hurtboxes.push_back({0, -18, 0, 36, 68});

    // Frame 1: Walking
    frames[1].id = 1;
    frames[1].state = ActionState::Walking;
    frames[1].wait = 6;
    frames[1].next = 0;
    frames[1].hit_a = 2;
    frames[1].hit_j = 4;
    frames[1].hit_d = 5;
    frames[1].hurtboxes.push_back({0, -18, 0, 36, 68});

    // Frame 2: Attack Anticipation (Punch prep)
    frames[2].id = 2;
    frames[2].state = ActionState::NormalAttack;
    frames[2].wait = 4;
    frames[2].next = 3;
    frames[2].hurtboxes.push_back({0, -18, 0, 36, 68});

    // Frame 3: Attack Active (Punch strike)
    frames[3].id = 3;
    frames[3].state = ActionState::NormalAttack;
    frames[3].wait = 6;
    frames[3].next = 0; // Return to idle
    frames[3].dvx = 1.5f; // Step forward slightly
    frames[3].hurtboxes.push_back({0, -18, 0, 36, 68});
    // Active attack hitbox in front of fighter
    {
        Hitbox hb;
        hb.kind = AttackKind::Normal;
        hb.x = 8;
        hb.y = 30;
        hb.w = 34;
        hb.h = 24;
        hb.dvx = 4.2f;
        hb.dvy = 1.8f;
        hb.fall = 25;
        hb.injury = 35;
        hb.arest = 14;
        hb.vrest = 16;
        hb.zwidth = 16.0f;
        frames[3].hitboxes.push_back(hb);
    }

    // Frame 4: Jump (In the air)
    frames[4].id = 4;
    frames[4].state = ActionState::Jumping;
    frames[4].wait = 6;
    frames[4].next = 4;
    frames[4].hurtboxes.push_back({0, -16, 10, 32, 50});

    // Frame 5: Defend / Guard
    frames[5].id = 5;
    frames[5].state = ActionState::Defend;
    frames[5].wait = 4;
    frames[5].next = 5;
    frames[5].hurtboxes.push_back({0, -16, 0, 32, 65});

    // Frame 6: Injured / Hitstun
    frames[6].id = 6;
    frames[6].state = ActionState::Injured;
    frames[6].wait = 12;
    frames[6].next = 0;
    frames[6].hurtboxes.push_back({0, -20, 0, 40, 65});

    // Frame 7: Falling (Knocked back into air)
    frames[7].id = 7;
    frames[7].state = ActionState::Falling;
    frames[7].wait = 8;
    frames[7].next = 7;
    frames[7].hurtboxes.push_back({0, -24, 0, 48, 40});

    // Frame 8: Lying on ground
    frames[8].id = 8;
    frames[8].state = ActionState::Lying;
    frames[8].wait = 30;
    frames[8].next = 0; // Get back up
    frames[8].hurtboxes.push_back({0, -30, 0, 60, 20});

    return frames;
}

Entity::Entity(uint32_t id_, std::string name_, Team team_, Vec3f pos)
    : id(id_), name(std::move(name_)), team(team_), position(pos) {
    frames = createDefaultFighterFrames();
    setFrame(0);
}

void Entity::setFrame(uint32_t frameId) {
    if (frameId < frames.size()) {
        currentFrameId = frameId;
        waitTimer = frames[frameId].wait;
    }
}

const Frame* Entity::getCurrentFrame() const {
    if (currentFrameId < frames.size()) {
        return &frames[currentFrameId];
    }
    return nullptr;
}

void Entity::applyImpulse(const Vec3f& impulse) {
    velocity += impulse;
    if (std::abs(velocity.y) > 0.1f) {
        isGrounded = false;
    }
}

void Entity::takeDamage(int32_t injury, const Vec3f& knockback, int32_t fall, uint32_t vrest) {
    hp = std::max(0, hp - injury);
    if (hp == 0) {
        isDead = true;
    }

    applyImpulse(knockback);
    fallValue += fall;
    vrestTimer = vrest;

    if (fallValue >= 60 || knockback.y > 2.0f) {
        setFrame(7); // Falling
    } else {
        setFrame(6); // Injured
    }
}

void Entity::tick([[maybe_unused]] float dt) {
    if (isDead && isGrounded) {
        setFrame(8); // Lying
        return;
    }

    // 1. Decrement status timers
    if (invincibleTimer > 0) --invincibleTimer;
    if (arestTimer > 0) --arestTimer;
    if (vrestTimer > 0) {
        --vrestTimer;
        // In hitstun: do not process directional or attack inputs
    } else {
        // 2. State & Input handling when not stunned
        const Frame* curFrame = getCurrentFrame();
        bool canAct = curFrame && (curFrame->state == ActionState::Standing || curFrame->state == ActionState::Walking);

        if (canAct) {
            // Check Attack
            if (triggerAttack && curFrame->hit_a >= 0) {
                setFrame(static_cast<uint32_t>(curFrame->hit_a));
            }
            // Check Jump
            else if (triggerJump && isGrounded && curFrame->hit_j >= 0) {
                velocity.y = 9.2f;
                isGrounded = false;
                setFrame(static_cast<uint32_t>(curFrame->hit_j));
            }
            // Check Defend
            else if (inputDefend && curFrame->hit_d >= 0) {
                setFrame(static_cast<uint32_t>(curFrame->hit_d));
            }
            // Movement in 2.5D space
            else {
                float moveSpeedX = 3.2f;
                float moveSpeedZ = 2.0f;
                bool isMoving = false;

                if (inputLeft) {
                    velocity.x = -moveSpeedX;
                    facing = Facing::Left;
                    isMoving = true;
                } else if (inputRight) {
                    velocity.x = moveSpeedX;
                    facing = Facing::Right;
                    isMoving = true;
                }

                if (inputUp) {
                    velocity.z = -moveSpeedZ;
                    isMoving = true;
                } else if (inputDown) {
                    velocity.z = moveSpeedZ;
                    isMoving = true;
                }

                if (isMoving && curFrame->state == ActionState::Standing) {
                    setFrame(1); // Walk
                } else if (!isMoving && curFrame->state == ActionState::Walking) {
                    setFrame(0); // Idle
                }
            }
        }
    }

    // 3. Frame Timer & Transitions
    if (waitTimer > 0) {
        --waitTimer;
    } else {
        const Frame* curFrame = getCurrentFrame();
        if (curFrame && curFrame->next >= 0) {
            setFrame(static_cast<uint32_t>(curFrame->next));
        }
    }

    // 4. Physics Integration
    position += velocity;

    // Gravity
    if (!isGrounded) {
        velocity.y -= 0.65f; // Gravity
        if (position.y <= 0.0f) {
            position.y = 0.0f;
            velocity.y = 0.0f;
            isGrounded = true;

            const Frame* curFrame = getCurrentFrame();
            if (curFrame && curFrame->state == ActionState::Falling) {
                setFrame(8); // Lying down upon impact
                fallValue = 0;
            } else if (curFrame && curFrame->state == ActionState::Jumping) {
                setFrame(0); // Land into standing
            }
        }
    }

    // Ground Friction
    if (isGrounded) {
        velocity.x *= 0.82f;
        velocity.z *= 0.82f;
        if (std::abs(velocity.x) < 0.05f) velocity.x = 0.0f;
        if (std::abs(velocity.z) < 0.05f) velocity.z = 0.0f;
    }

    // Reset single-frame pulse triggers
    triggerAttack = false;
    triggerJump = false;
    triggerDefend = false;
}

std::vector<WorldBox> Entity::getWorldHitboxes() const {
    std::vector<WorldBox> boxes;
    const Frame* f = getCurrentFrame();
    if (!f) return boxes;

    for (const auto& hb : f->hitboxes) {
        WorldBox wb;
        wb.z = position.z;
        wb.zwidth = hb.zwidth;
        wb.bottom = position.y + static_cast<float>(hb.y);
        wb.top = wb.bottom + static_cast<float>(hb.h);

        if (facing == Facing::Right) {
            wb.left = position.x + static_cast<float>(hb.x);
            wb.right = wb.left + static_cast<float>(hb.w);
        } else {
            wb.right = position.x - static_cast<float>(hb.x);
            wb.left = wb.right - static_cast<float>(hb.w);
        }
        boxes.push_back(wb);
    }
    return boxes;
}

std::vector<WorldBox> Entity::getWorldHurtboxes() const {
    std::vector<WorldBox> boxes;
    const Frame* f = getCurrentFrame();
    if (!f) return boxes;

    for (const auto& hb : f->hurtboxes) {
        WorldBox wb;
        wb.z = position.z;
        wb.zwidth = 14.0f;
        wb.bottom = position.y + static_cast<float>(hb.y);
        wb.top = wb.bottom + static_cast<float>(hb.h);

        if (facing == Facing::Right) {
            wb.left = position.x + static_cast<float>(hb.x);
            wb.right = wb.left + static_cast<float>(hb.w);
        } else {
            wb.right = position.x - static_cast<float>(hb.x);
            wb.left = wb.right - static_cast<float>(hb.w);
        }
        boxes.push_back(wb);
    }
    return boxes;
}

} // namespace kfighter
