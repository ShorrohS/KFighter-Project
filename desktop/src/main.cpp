#include <SDL.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include "kfighter/world.hpp"

using namespace kfighter;

constexpr int WINDOW_WIDTH = 960;
constexpr int WINDOW_HEIGHT = 540;
constexpr float ARENA_CENTER_X = WINDOW_WIDTH * 0.5f;
constexpr float ARENA_BASE_Y = 380.0f; // Ground baseline in screen space

// Project 2.5D world position (x, y, z) into 2D screen coordinates
static SDL_Point worldToScreen(const Vec3f& pos) {
    // x: horizontal translation
    // z: depth translation (perspective shift)
    // y: vertical jump offset (upward is negative screen Y)
    int sx = static_cast<int>(ARENA_CENTER_X + pos.x);
    int sy = static_cast<int>(ARENA_BASE_Y + pos.z * 1.2f - pos.y * 1.5f);
    return {sx, sy};
}

static SDL_Point worldGroundToScreen(const Vec3f& pos) {
    int sx = static_cast<int>(ARENA_CENTER_X + pos.x);
    int sy = static_cast<int>(ARENA_BASE_Y + pos.z * 1.2f);
    return {sx, sy};
}

static void drawFilledCircle(SDL_Renderer* ren, int cx, int cy, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(ren, cx + dx, cy + dy);
            }
        }
    }
}

static void drawShadow(SDL_Renderer* ren, const Vec3f& pos) {
    SDL_Point gp = worldGroundToScreen(pos);
    // Shadow size shrinks slightly when jumping high
    float jumpFactor = std::max(0.2f, 1.0f - (pos.y / 200.0f));
    int rx = static_cast<int>(26 * jumpFactor);
    int ry = static_cast<int>(10 * jumpFactor);

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 15, 25, 20, 110);

    for (int dy = -ry; dy <= ry; ++dy) {
        int span = static_cast<int>(rx * std::sqrt(1.0f - (float)(dy * dy) / (float)(ry * ry)));
        SDL_RenderDrawLine(ren, gp.x - span, gp.y + dy, gp.x + span, gp.y + dy);
    }
}

static void drawEntity(SDL_Renderer* ren, const Entity& e, bool isPlayer) {
    SDL_Point sp = worldToScreen(e.position);

    // Invincibility flicker
    if (e.invincibleTimer > 0 && ((e.invincibleTimer / 3) % 2 == 0)) {
        return;
    }

    // 1. Draw Hurtboxes
    auto hurtboxes = e.getWorldHurtboxes();
    for (const auto& wb : hurtboxes) {
        // Convert world box to screen rect
        int left = static_cast<int>(ARENA_CENTER_X + wb.left);
        int top = static_cast<int>(ARENA_BASE_Y + wb.z * 1.2f - wb.top * 1.5f);
        int width = static_cast<int>(wb.right - wb.left);
        int height = static_cast<int>((wb.top - wb.bottom) * 1.5f);

        SDL_Rect rect{left, top, width, height};

        // Fill body
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        if (isPlayer) {
            SDL_SetRenderDrawColor(ren, 40, 160, 220, 180); // Blue for Player
        } else {
            SDL_SetRenderDrawColor(ren, 220, 120, 40, 180); // Orange for Dummy
        }
        SDL_RenderFillRect(ren, &rect);

        // Outline
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 230);
        SDL_RenderDrawRect(ren, &rect);

        // Head indicator
        int headR = width / 3;
        int headX = left + width / 2;
        int headY = top + headR;
        SDL_SetRenderDrawColor(ren, 255, 230, 180, 255);
        drawFilledCircle(ren, headX, headY, headR);

        // Facing eye indicator
        int eyeOffset = (e.facing == Facing::Right) ? headR / 2 : -headR / 2;
        SDL_SetRenderDrawColor(ren, 20, 20, 30, 255);
        drawFilledCircle(ren, headX + eyeOffset, headY - 2, 3);
    }

    // 2. Draw Active Attack Hitboxes
    auto hitboxes = e.getWorldHitboxes();
    for (const auto& wb : hitboxes) {
        int left = static_cast<int>(ARENA_CENTER_X + wb.left);
        int top = static_cast<int>(ARENA_BASE_Y + wb.z * 1.2f - wb.top * 1.5f);
        int width = static_cast<int>(wb.right - wb.left);
        int height = static_cast<int>((wb.top - wb.bottom) * 1.5f);

        SDL_Rect rect{left, top, width, height};

        // Translucent red active hit area
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 255, 40, 40, 160);
        SDL_RenderFillRect(ren, &rect);

        // Crisp red border
        SDL_SetRenderDrawColor(ren, 255, 230, 50, 255);
        SDL_RenderDrawRect(ren, &rect);
    }

    // 3. Mini Overhead HP Bar
    int hpBarW = 50;
    int hpBarH = 6;
    int hpX = sp.x - hpBarW / 2;
    int hpY = sp.y - 120;

    SDL_Rect bgRect{hpX, hpY, hpBarW, hpBarH};
    SDL_SetRenderDrawColor(ren, 40, 40, 40, 200);
    SDL_RenderFillRect(ren, &bgRect);

    float hpPct = std::clamp(static_cast<float>(e.hp) / static_cast<float>(e.maxHp), 0.0f, 1.0f);
    SDL_Rect fgRect{hpX, hpY, static_cast<int>(hpBarW * hpPct), hpBarH};
    if (hpPct > 0.5f) {
        SDL_SetRenderDrawColor(ren, 50, 220, 70, 255);
    } else if (hpPct > 0.25f) {
        SDL_SetRenderDrawColor(ren, 240, 200, 30, 255);
    } else {
        SDL_SetRenderDrawColor(ren, 240, 40, 40, 255);
    }
    SDL_RenderFillRect(ren, &fgRect);
}

static void drawHUD(SDL_Renderer* ren, const Entity& p1, const Entity& p2, bool dummyGuarding) {
    // HUD Header panel
    SDL_Rect headerRect{20, 15, WINDOW_WIDTH - 40, 60};
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 15, 20, 30, 210);
    SDL_RenderFillRect(ren, &headerRect);
    SDL_SetRenderDrawColor(ren, 200, 160, 60, 255);
    SDL_RenderDrawRect(ren, &headerRect);

    // Player 1 HP Bar (Left)
    int barW = 320;
    int barH = 18;
    SDL_Rect p1Bg{40, 30, barW, barH};
    SDL_SetRenderDrawColor(ren, 40, 40, 40, 255);
    SDL_RenderFillRect(ren, &p1Bg);

    float p1HpPct = std::clamp(static_cast<float>(p1.hp) / static_cast<float>(p1.maxHp), 0.0f, 1.0f);
    SDL_Rect p1Fg{40, 30, static_cast<int>(barW * p1HpPct), barH};
    SDL_SetRenderDrawColor(ren, 40, 180, 240, 255);
    SDL_RenderFillRect(ren, &p1Fg);

    // Player 2 HP Bar (Right)
    int p2X = WINDOW_WIDTH - 40 - barW;
    SDL_Rect p2Bg{p2X, 30, barW, barH};
    SDL_SetRenderDrawColor(ren, 40, 40, 40, 255);
    SDL_RenderFillRect(ren, &p2Bg);

    float p2HpPct = std::clamp(static_cast<float>(p2.hp) / static_cast<float>(p2.maxHp), 0.0f, 1.0f);
    int p2FillW = static_cast<int>(barW * p2HpPct);
    SDL_Rect p2Fg{p2X + (barW - p2FillW), 30, p2FillW, barH};
    SDL_SetRenderDrawColor(ren, 240, 120, 40, 255);
    SDL_RenderFillRect(ren, &p2Fg);

    // Guard indicator
    if (dummyGuarding) {
        SDL_Rect gRect{p2X, 52, 90, 14};
        SDL_SetRenderDrawColor(ren, 240, 200, 40, 200);
        SDL_RenderFillRect(ren, &gRect);
    }
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    std::cout << "Starting KFighter Desktop Harness (C++20 Engine)...\n";

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow(
        "KFighter - Kerala Combat Engine (C++20 & LF2 Physics)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!win) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    }

    // Initialize 2.5D Combat World
    World world;
    auto player = world.spawnFighter(1, "Aromal (P1)", Team::Team1, Vec3f(-120.0f, 0.0f, 0.0f));
    auto dummy = world.spawnFighter(2, "Othenan (P2)", Team::Team2, Vec3f(120.0f, 0.0f, 0.0f));
    dummy->facing = Facing::Left;

    bool running = true;
    bool dummyGuarding = false;
    SDL_Event ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = false;
            } else if (ev.type == SDL_KEYDOWN && !ev.key.repeat) {
                switch (ev.key.keysym.sym) {
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_LEFT:  player->inputLeft = true; break;
                    case SDLK_RIGHT: player->inputRight = true; break;
                    case SDLK_UP:    player->inputUp = true; break;
                    case SDLK_DOWN:  player->inputDown = true; break;
                    case SDLK_z:
                    case SDLK_j:
                        player->inputAttack = true;
                        player->triggerAttack = true;
                        break;
                    case SDLK_x:
                    case SDLK_k:
                        player->inputJump = true;
                        player->triggerJump = true;
                        break;
                    case SDLK_c:
                    case SDLK_l:
                        player->inputDefend = true;
                        break;
                    case SDLK_b:
                        dummyGuarding = !dummyGuarding;
                        dummy->inputDefend = dummyGuarding;
                        break;
                    case SDLK_r:
                        // Reset simulation
                        player->position = Vec3f(-120.0f, 0.0f, 0.0f);
                        player->velocity = Vec3f(0.0f, 0.0f, 0.0f);
                        player->hp = player->maxHp;
                        player->isDead = false;
                        player->setFrame(0);

                        dummy->position = Vec3f(120.0f, 0.0f, 0.0f);
                        dummy->velocity = Vec3f(0.0f, 0.0f, 0.0f);
                        dummy->hp = dummy->maxHp;
                        dummy->isDead = false;
                        dummy->facing = Facing::Left;
                        dummy->setFrame(0);
                        break;
                }
            } else if (ev.type == SDL_KEYUP) {
                switch (ev.key.keysym.sym) {
                    case SDLK_LEFT:  player->inputLeft = false; break;
                    case SDLK_RIGHT: player->inputRight = false; break;
                    case SDLK_UP:    player->inputUp = false; break;
                    case SDLK_DOWN:  player->inputDown = false; break;
                    case SDLK_z:
                    case SDLK_j:     player->inputAttack = false; break;
                    case SDLK_x:
                    case SDLK_k:     player->inputJump = false; break;
                    case SDLK_c:
                    case SDLK_l:     player->inputDefend = false; break;
                }
            }
        }

        // Advance Game Simulation by 1 tick (60 Hz)
        world.tick(1.0f / 60.0f);

        // Render Scene
        // 1. Background: Kerala Temple/Backwaters Theme
        // Sky gradient
        SDL_SetRenderDrawColor(ren, 25, 45, 65, 255);
        SDL_RenderClear(ren);

        // Ground stage: Kalaripayattu Gurukulam Clay Floor
        SDL_Rect stageRect{0, static_cast<int>(ARENA_BASE_Y - 40), WINDOW_WIDTH, WINDOW_HEIGHT - static_cast<int>(ARENA_BASE_Y - 40)};
        SDL_SetRenderDrawColor(ren, 85, 45, 30, 255); // Red laterite clay
        SDL_RenderFillRect(ren, &stageRect);

        // 2.5D Depth Floor Lines (Perspective Grid)
        SDL_SetRenderDrawColor(ren, 110, 60, 40, 180);
        for (float z = -80.0f; z <= 80.0f; z += 20.0f) {
            int lineY = static_cast<int>(ARENA_BASE_Y + z * 1.2f);
            SDL_RenderDrawLine(ren, 40, lineY, WINDOW_WIDTH - 40, lineY);
        }

        // Draw Depth Shadows on the floor first
        drawShadow(ren, player->position);
        drawShadow(ren, dummy->position);

        // Sort entities by Z-depth for correct draw order
        std::vector<std::shared_ptr<Entity>> sortedEntities = world.entities;
        std::sort(sortedEntities.begin(), sortedEntities.end(), [](const auto& a, const auto& b) {
            return a->position.z < b->position.z;
        });

        // Draw Entities
        for (const auto& ent : sortedEntities) {
            if (!ent) continue;
            drawEntity(ren, *ent, ent->id == player->id);
        }

        // Draw Hit Sparks from recent combat events
        for (const auto& evt : world.getRecentEvents()) {
            SDL_Point sp = worldToScreen(evt.impactPosition);
            SDL_SetRenderDrawColor(ren, 255, 240, 100, 255);
            drawFilledCircle(ren, sp.x, sp.y, 8);
            SDL_SetRenderDrawColor(ren, 255, 80, 40, 255);
            drawFilledCircle(ren, sp.x, sp.y, 4);
        }

        // Draw HUD
        drawHUD(ren, *player, *dummy, dummyGuarding);

        SDL_RenderPresent(ren);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    std::cout << "KFighter Desktop closed cleanly.\n";
    return 0;
}
