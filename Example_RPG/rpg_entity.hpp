#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

// ---------------------------------------------------------------------------
// Axis-aligned bounding box helpers
// ---------------------------------------------------------------------------

struct AABB {
    float x, y, w, h;
};

inline bool AABBOverlaps(const AABB& a, const AABB& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x &&
           a.y < b.y + b.h && a.y + a.h > b.y;
}

// ---------------------------------------------------------------------------
// Base entity
// ---------------------------------------------------------------------------

struct Entity {
    float x = 0, y = 0;
    float w = 24, h = 24;

    float cx() const { return x + w * 0.5f; }
    float cy() const { return y + h * 0.5f; }

    AABB aabb() const { return { x, y, w, h }; }
};

// ---------------------------------------------------------------------------
// Player
// ---------------------------------------------------------------------------

struct Player : Entity {
    float speed        = 90.0f;
    float hp           = 100.0f;
    float maxHp        = 100.0f;
    float attackDmg    = 25.0f;
    float attackRange  = 32.0f;
    float attackCooldown = 0.0f;
    float attackTimer  = 0.0f;
    bool  isAttacking  = false;
    float facing       = 1.0f;  // +1 right, -1 left

    void takeDamage(float dmg) { hp = std::max(0.0f, hp - dmg); }
    bool isDead() const { return hp <= 0.0f; }
};

// ---------------------------------------------------------------------------
// Enemy
// ---------------------------------------------------------------------------

struct Enemy : Entity {
    Enemy() = default;
    Enemy(float startX, float startY) { x = startX; y = startY; }

    float hp           = 60.0f;
    float maxHp        = 60.0f;
    float speed        = 35.0f;
    float aggroRange   = 160.0f;
    float attackRange  = 20.0f;
    float attackDmg    = 8.0f;
    float attackCooldown = 0.0f;
    bool  alive        = true;

    void takeDamage(float dmg) {
        hp = std::max(0.0f, hp - dmg);
        if (hp <= 0.0f) alive = false;
    }
};

// ---------------------------------------------------------------------------
// NPC
// ---------------------------------------------------------------------------

struct NPC : Entity {
    NPC() = default;
    NPC(float nx, float ny, std::string n, std::vector<std::string> lines)
        : name(std::move(n)), dialogue(std::move(lines)) {
        x = nx; y = ny;
    }
    std::string              name;
    std::vector<std::string> dialogue;
    int                      dialogueIdx = 0;
    float                    talkRadius  = 48.0f;
};

// ---------------------------------------------------------------------------
// Simple sprite animator (UV-offset, no engine component required)
// ---------------------------------------------------------------------------

struct AnimFrame {
    int col, row;  // tileset column and row
    float duration;
};

struct AnimClip {
    std::vector<AnimFrame> frames;
    bool loop = true;
};

struct SpriteAnimator {
    std::unordered_map<std::string, AnimClip> clips;
    std::string currentClip;
    int   frameIdx  = 0;
    float timer     = 0.0f;
    bool  playing   = false;

    void addClip(const std::string& name, AnimClip clip) {
        clips[name] = std::move(clip);
    }

    void play(const std::string& name) {
        if (currentClip == name && playing) return;
        currentClip = name;
        frameIdx    = 0;
        timer       = 0.0f;
        playing     = true;
    }

    void update(float dt) {
        if (!playing || clips.empty()) return;
        auto it = clips.find(currentClip);
        if (it == clips.end()) return;
        const auto& clip = it->second;
        if (clip.frames.empty()) return;

        timer += dt;
        if (timer >= clip.frames[frameIdx].duration) {
            timer -= clip.frames[frameIdx].duration;
            frameIdx++;
            if (frameIdx >= (int)clip.frames.size()) {
                if (clip.loop) frameIdx = 0;
                else { frameIdx = (int)clip.frames.size()-1; playing = false; }
            }
        }
    }

    // Returns {col, row} of the current frame (for UV lookup in DrawSprite2D)
    std::pair<int,int> currentFrame() const {
        auto it = clips.find(currentClip);
        if (it == clips.end() || it->second.frames.empty())
            return {0, 0};
        const auto& f = it->second.frames[frameIdx];
        return {f.col, f.row};
    }
};

// ---------------------------------------------------------------------------
// Procedural texture helpers (returns raw RGBA pixel buffer)
// ---------------------------------------------------------------------------

#include <cmath>
#include <random>

inline std::vector<uint8_t> MakeColorSheetPixels(
        int tileSize, int cols, int rows,
        const std::vector<std::array<uint8_t,3>>& colors) {
    int W = cols * tileSize, H = rows * tileSize;
    std::vector<uint8_t> px(W * H * 4);
    static std::mt19937 rng(42);
    std::uniform_int_distribution<int> noise(-5, 5);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            auto [cr, cg, cb] = (idx < (int)colors.size()) ? colors[idx]
                                : std::array<uint8_t,3>{128,128,128};
            for (int py = 0; py < tileSize; py++) {
                for (int px_ = 0; px_ < tileSize; px_++) {
                    int i = ((r*tileSize + py)*W + c*tileSize + px_) * 4;
                    bool edge = px_<2 || py<2 || px_>=tileSize-2 || py>=tileSize-2;
                    px[i  ] = (uint8_t)std::clamp((int)cr + (edge ? -30 : noise(rng)), 0, 255);
                    px[i+1] = (uint8_t)std::clamp((int)cg + (edge ? -30 : noise(rng)), 0, 255);
                    px[i+2] = (uint8_t)std::clamp((int)cb + (edge ? -30 : noise(rng)), 0, 255);
                    px[i+3] = 255;
                }
            }
        }
    }
    return px;
}
