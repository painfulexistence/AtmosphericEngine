#include "rpg_game.hpp"

#ifndef __EMSCRIPTEN__
#include <glad/glad.h>
#else
#include <GLES3/gl3.h>
#endif

#include <Atmospheric/graphics_server.hpp>
#include <Atmospheric/input.hpp>
#include <Atmospheric/audio_manager.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <fmt/format.h>

using glm::vec2;
using glm::vec4;

// ---------------------------------------------------------------------------
// Construction / AppConfig
// ---------------------------------------------------------------------------

RPGGame::RPGGame() : Application(AppConfig{
    .windowTitle    = "RPG Demo (C++ port of 2d-engine)",
    .windowWidth    = 800,
    .windowHeight   = 600,
    .enableGraphics3D = false,
    .enablePhysics3D  = false,
}) {}

// ---------------------------------------------------------------------------
// Helpers — OpenGL texture creation (no stb_image, fully procedural)
// ---------------------------------------------------------------------------

uint32_t RPGGame::CreateGLTexture(const uint8_t* pixels, int w, int h) {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (uint32_t)id;
}

uint32_t RPGGame::CreateSolidTexture(uint8_t r, uint8_t g, uint8_t b) {
    const uint8_t px[4] = { r, g, b, 255 };
    return CreateGLTexture(px, 1, 1);
}

// ---------------------------------------------------------------------------
// AnimClip builder (mirrors 2d-engine makeClip)
// ---------------------------------------------------------------------------

AnimClip RPGGame::MakeClip(int sheetCols, int frameRow,
                            std::vector<int> frameCols,
                            float frameDur, bool loop) {
    (void)sheetCols;
    AnimClip clip;
    clip.loop = loop;
    for (int c : frameCols)
        clip.frames.push_back({ c, frameRow, frameDur });
    return clip;
}

// ---------------------------------------------------------------------------
// OnInit — configure once-off state (pre-OpenGL)
// ---------------------------------------------------------------------------

void RPGGame::OnInit() {
    _screenW = 800;
    _screenH = 600;
}

// ---------------------------------------------------------------------------
// OnLoad — create textures, entities, tilemap
// ---------------------------------------------------------------------------

void RPGGame::OnLoad() {
    auto* gfx = GetGraphicsServer();

    // ─ Font ─────────────────────────────────────────────────────────────
    _fontID = gfx->LoadFont("assets/fonts/OpenSans-Regular.ttf", 20.0f);

    // ─ Tileset texture (procedural) ─────────────────────────────────────
    // 8 tile types arranged in a 4×2 grid, each 32×32 px
    constexpr int TS = 32, TCOLS = 4, TROWS = 2;
    const std::vector<std::array<uint8_t,3>> tileColors = {
        {80,120,60},    // 0 grass
        {60,50,40},     // 1 dirt
        {100,100,110},  // 2 stone
        {40,80,130},    // 3 water
        {120,90,50},    // 4 sand
        {80,80,80},     // 5 dark stone
        {60,100,50},    // 6 dark grass
        {180,160,120},  // 7 light stone
    };
    auto tilePixels = MakeColorSheetPixels(TS, TCOLS, TROWS, tileColors);
    _tilesetTex = CreateGLTexture(tilePixels.data(), TCOLS*TS, TROWS*TS);

    // ─ Tilemap data (matches TS RPG map) ───────────────────────────────
    const std::vector<std::string> MAP_STR = {
        "3333333333333333333333333",
        "3111111111111111111111113",
        "3111111111111111111111113",
        "3111221111111111113311113",
        "3111221111111111113311113",
        "3111111111111111111111113",
        "3111111111222111111111113",
        "3111111111222111111111113",
        "3133111111111111111331113",
        "3133111111111111111331113",
        "3111111111111111111111113",
        "3111111222211111122111113",
        "3111111222211111122111113",
        "3111111111111111111111113",
        "3111111111111111111111113",
        "3111441111111111114411113",
        "3111441111111111114411113",
        "3111111111111111111111113",
        "3111111111111111111111113",
        "3333333333333333333333333",
    };
    Tilemap2DData mapData;
    mapData.width      = (int)MAP_STR[0].size();
    mapData.height     = (int)MAP_STR.size();
    mapData.tileSize   = TS;
    mapData.tilesetCols = TCOLS;
    mapData.tilesetRows = TROWS;
    mapData.solid      = { 3, 4 };  // water + sand are solid
    for (const auto& row : MAP_STR)
        for (char ch : row)
            mapData.tiles.push_back(ch - '0');
    _tilemap = std::make_unique<Tilemap2D>(mapData, _tilesetTex);

    // ─ Player texture ─────────────────────────────────────────────
    // 4 walk frames + 4 attack frames, 2 rows (2 animation sets)
    constexpr int CS = 24, CCOLS = 4, CROWS = 2;
    auto playerPixels = MakeColorSheetPixels(CS, CCOLS, CROWS, {
        {70,120,200},{60,110,190},{80,130,210},{65,115,195},
        {50,100,180},{75,125,205},{85,140,215},{55,105,185},
    });
    _playerTex = CreateGLTexture(playerPixels.data(), CCOLS*CS, CROWS*CS);

    // ─ Enemy texture ─────────────────────────────────────────────
    auto enemyPixels = MakeColorSheetPixels(CS, CCOLS, CROWS, {
        {200,60,60},{180,50,50},{210,70,70},{190,55,55},
        {220,80,80},{170,45,45},{200,65,65},{185,55,55},
    });
    _enemyTex = CreateGLTexture(enemyPixels.data(), CCOLS*CS, CROWS*CS);

    // ─ NPC texture (solid green) ────────────────────────────────────
    _npcTex = CreateSolidTexture(180, 200, 80);

    // ─ Player entity + animations ──────────────────────────────────
    _player.x = 3 * TS;
    _player.y = 3 * TS;
    _player.w = _player.h = CS;
    _playerAnim.addClip("idle",   MakeClip(CCOLS, 0, {0,1},     0.4f));
    _playerAnim.addClip("walk",   MakeClip(CCOLS, 0, {0,1,2,3}, 0.12f));
    _playerAnim.addClip("attack", MakeClip(CCOLS, 1, {0,1,2},   0.08f, false));
    _playerAnim.play("idle");

    // ─ Enemies ─────────────────────────────────────────────────────
    const std::vector<std::pair<float,float>> enemySpawns = {
        {8*TS, 8*TS}, {15*TS, 12*TS}, {20*TS, 6*TS}, {12*TS, 16*TS}
    };
    for (auto [ex, ey] : enemySpawns) {
        Enemy e(ex, ey);
        e.w = e.h = CS;
        _enemies.push_back(e);
        SpriteAnimator anim;
        anim.addClip("idle", MakeClip(CCOLS, 0, {0,1},     0.6f));
        anim.addClip("walk", MakeClip(CCOLS, 0, {0,1,2,3}, 0.15f));
        anim.play("idle");
        _enemyAnims.push_back(std::move(anim));
    }

    // ─ NPCs ───────────────────────────────────────────────────────
    _npcs.push_back(NPC(6*TS, 6*TS, "Villager", {
        "Villager: Beware of the red creatures!",
        "Villager: Press Z to attack them.",
        "Villager: Good luck, adventurer!",
    }));
    _npcs.push_back(NPC(18*TS, 15*TS, "Merchant", {
        "Merchant: I have nothing to sell... yet.",
        "Merchant: Come back later!",
    }));
    _npcs[0].w = _npcs[0].h = CS;
    _npcs[1].w = _npcs[1].h = CS;

    // ─ Lighting ────────────────────────────────────────────────────
    _lighting.ambientR = 0.18f;
    _lighting.ambientG = 0.20f;
    _lighting.ambientB = 0.28f;
    _lighting.ambientA = 1.0f;
}

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------

void RPGGame::CameraFollow(float targetCX, float targetCY) {
    _camX = targetCX - _screenW * 0.5f;
    _camY = targetCY - _screenH * 0.5f;
}

void RPGGame::CameraClamp(int mapW, int mapH) {
    _camX = std::clamp(_camX, 0.0f, (float)(mapW  - _screenW));
    _camY = std::clamp(_camY, 0.0f, (float)(mapH - _screenH));
}

// ---------------------------------------------------------------------------
// OnUpdate — game logic + draw calls
// ---------------------------------------------------------------------------

void RPGGame::OnUpdate(float dt, float /*time*/) {
    _time    += dt;
    _fadeIn   = std::max(0.0f, _fadeIn - dt * 1.5f);
    _hitFlash = std::max(0.0f, _hitFlash - dt);

    UpdatePlayer(dt);
    UpdateEnemies(dt);

    if (_dialogTimer > 0.0f) {
        _dialogTimer -= dt;
        if (_dialogTimer <= 0.0f) _dialogText.clear();
    }

    _playerAnim.update(dt);
    for (size_t i = 0; i < _enemies.size(); i++)
        if (_enemies[i].alive) _enemyAnims[i].update(dt);

    // Camera
    CameraFollow(_player.cx(), _player.cy());
    CameraClamp(_tilemap->GetPixelWidth(), _tilemap->GetPixelHeight());

    // Lighting: player + nearby alive enemies
    _lighting.lights.clear();
    _lighting.lights.push_back({
        _player.cx() - _camX,
        _player.cy() - _camY,
        1.0f, 0.9f, 0.7f, 180.0f, 1.2f
    });
    for (const auto& e : _enemies) {
        if (!e.alive) continue;
        float dx = e.cx() - _player.cx(), dy = e.cy() - _player.cy();
        if (dx*dx + dy*dy > 300.0f*300.0f) continue;
        _lighting.lights.push_back({
            e.cx() - _camX,
            e.cy() - _camY,
            1.0f, 0.3f, 0.2f, 80.0f, 0.7f
        });
    }

    DrawScene();
    DrawHUD();
    if (!_dialogText.empty()) DrawDialog();
}

// ---------------------------------------------------------------------------
// UpdatePlayer
// ---------------------------------------------------------------------------

void RPGGame::UpdatePlayer(float dt) {
    auto* inp = GetInput();
    auto* gfx = GetGraphicsServer();
    (void)gfx;

    float ax = 0, ay = 0;
    if (inp->IsKeyDown(Key::KEY_LEFT)  || inp->IsKeyDown(Key::KEY_A)) ax -= 1;
    if (inp->IsKeyDown(Key::KEY_RIGHT) || inp->IsKeyDown(Key::KEY_D)) ax += 1;
    if (inp->IsKeyDown(Key::KEY_UP)    || inp->IsKeyDown(Key::KEY_W)) ay -= 1;
    if (inp->IsKeyDown(Key::KEY_DOWN)  || inp->IsKeyDown(Key::KEY_S)) ay += 1;

    Player& p = _player;
    const auto& md = _tilemap->GetData();

    // Attack
    if (inp->IsKeyPressed(Key::KEY_Z) && p.attackCooldown <= 0) {
        p.isAttacking   = true;
        p.attackTimer   = 0.25f;
        p.attackCooldown = 0.4f;
        _playerAnim.play("attack");

        AABB hb = { p.x + (p.facing > 0 ? p.w : -p.attackRange),
                    p.y, p.attackRange, p.h };
        for (auto& e : _enemies) {
            if (!e.alive) continue;
            if (AABBOverlaps(hb, e.aabb())) {
                e.takeDamage(p.attackDmg);
                _hitFlash = 0.15f;
            }
        }
    }
    p.attackCooldown -= dt;
    if (p.attackTimer > 0) {
        p.attackTimer -= dt;
        if (p.attackTimer <= 0) p.isAttacking = false;
    }

    // Interact
    if (inp->IsKeyPressed(Key::KEY_E)) {
        for (auto& npc : _npcs) {
            float dx = npc.cx() - p.cx(), dy = npc.cy() - p.cy();
            if (std::sqrt(dx*dx + dy*dy) < npc.talkRadius) {
                _dialogText  = npc.dialogue[npc.dialogueIdx % (int)npc.dialogue.size()];
                npc.dialogueIdx++;
                _dialogTimer = 3.5f;
                break;
            }
        }
    }

    // Animation
    if (!p.isAttacking && p.attackTimer <= 0)
        _playerAnim.play((ax||ay) ? "walk" : "idle");

    // Movement
    if (ax || ay) {
        float len = std::sqrt(ax*ax + ay*ay);
        float nx = ax/len, ny = ay/len;
        float newX = p.x + nx * p.speed * dt;
        float newY = p.y + ny * p.speed * dt;

        auto solid = [&](float wx, float wy) {
            return _tilemap->IsSolidWorld(wx, wy);
        };
        if (!solid(newX, p.y) && !solid(newX+p.w-1, p.y) &&
            !solid(newX, p.y+p.h-1) && !solid(newX+p.w-1, p.y+p.h-1))
            p.x = newX;
        if (!solid(p.x, newY) && !solid(p.x+p.w-1, newY) &&
            !solid(p.x, newY+p.h-1) && !solid(p.x+p.w-1, newY+p.h-1))
            p.y = newY;

        if (ax) p.facing = (ax > 0) ? 1.0f : -1.0f;
    }
}

// ---------------------------------------------------------------------------
// UpdateEnemies
// ---------------------------------------------------------------------------

void RPGGame::UpdateEnemies(float dt) {
    for (size_t i = 0; i < _enemies.size(); i++) {
        Enemy& e = _enemies[i];
        if (!e.alive) continue;

        float dx = _player.cx() - e.cx();
        float dy = _player.cy() - e.cy();
        float dist = std::sqrt(dx*dx + dy*dy);
        e.attackCooldown -= dt;

        if (dist < e.aggroRange) {
            float nx = dx/dist, ny = dy/dist;
            float newX = e.x + nx * e.speed * dt;
            float newY = e.y + ny * e.speed * dt;

            auto solid = [&](float wx, float wy) {
                return _tilemap->IsSolidWorld(wx, wy);
            };
            if (!solid(newX,e.y)&&!solid(newX+e.w-1,e.y)&&
                !solid(newX,e.y+e.h-1)&&!solid(newX+e.w-1,e.y+e.h-1)) e.x = newX;
            if (!solid(e.x,newY)&&!solid(e.x+e.w-1,newY)&&
                !solid(e.x,newY+e.h-1)&&!solid(e.x+e.w-1,newY+e.h-1)) e.y = newY;

            _enemyAnims[i].play("walk");

            if (dist < e.attackRange && e.attackCooldown <= 0) {
                _player.takeDamage(e.attackDmg);
                e.attackCooldown = 1.2f;
                _hitFlash = 0.1f;
            }
        } else {
            _enemyAnims[i].play("idle");
        }
    }
}

// ---------------------------------------------------------------------------
// DrawScene
// ---------------------------------------------------------------------------

void RPGGame::DrawScene() {
    auto* gfx = GetGraphicsServer();
    const float camX = _camX, camY = _camY;

    // — Tilemap
    _tilemap->Draw(gfx, camX, camY, _screenW, _screenH);

    // — NPCs
    for (const auto& npc : _npcs) {
        float sx = npc.x - camX, sy = npc.y - camY;
        gfx->DrawTexturedQuad(sx + npc.w*0.5f, sy + npc.h*0.5f,
                               npc.w, npc.h, 0.0f,
                               _npcTex, vec4(1.0f));
    }

    // — Enemies
    constexpr int CS = 24, CCOLS = 4, CROWS = 2;
    for (size_t i = 0; i < _enemies.size(); i++) {
        if (!_enemies[i].alive) continue;
        const Enemy& e = _enemies[i];
        auto [fc, fr] = _enemyAnims[i].currentFrame();
        float sx = e.x - camX, sy = e.y - camY;
        vec2 uvMin = vec2((float)fc/(float)CCOLS, (float)fr/(float)CROWS);
        vec2 uvMax = vec2((float)(fc+1)/(float)CCOLS, (float)(fr+1)/(float)CROWS);
        gfx->DrawSprite2D(sx, sy, e.w, e.h, _enemyTex, uvMin, uvMax);
    }

    // — Player
    {
        auto [fc, fr] = _playerAnim.currentFrame();
        float sx = _player.x - camX, sy = _player.y - camY;
        vec2 uvMin = vec2((float)fc/(float)CCOLS, (float)fr/(float)CROWS);
        vec2 uvMax = vec2((float)(fc+1)/(float)CCOLS, (float)(fr+1)/(float)CROWS);
        vec4 tint = (_hitFlash > 0) ? vec4(1,0.3f,0.3f,1) : vec4(1);
        gfx->DrawSprite2D(sx, sy, _player.w, _player.h, _playerTex, uvMin, uvMax, tint);
    }

    // — Lighting overlay (dark ambient + light circles)
    _lighting.Apply(gfx, _screenW, _screenH);
}

// ---------------------------------------------------------------------------
// DrawHUD
// ---------------------------------------------------------------------------

void RPGGame::DrawHUD() {
    auto* gfx = GetGraphicsServer();

    DrawHPBar(12, 12, 120, 12, _player.hp, _player.maxHp);

    // Enemy HP labels
    const float camX = _camX, camY = _camY;
    for (const auto& e : _enemies) {
        if (!e.alive || e.hp == e.maxHp) continue;
        float sx = e.x - camX, sy = e.y - camY - 12;
        if (sx < -40 || sx > _screenW || sy < -20 || sy > _screenH) continue;
        auto label = fmt::format("{}/{}", (int)e.hp, (int)e.maxHp);
        gfx->DrawText(_fontID, label, sx, sy, 0.6f, vec4(1, 0.4f, 0.4f, 1));
    }

    // Controls hint
    gfx->DrawText(_fontID, "WASD/Arrows:move  Z:attack  E:talk",
                  12, (float)_screenH - 24, 0.5f, vec4(0.8f, 0.8f, 0.8f, 0.7f));

    // Fade-in overlay
    if (_fadeIn > 0.01f) {
        gfx->DrawQuad(0, 0, (float)_screenW, (float)_screenH, 0.0f,
                       vec4(0, 0, 0, _fadeIn));
    }
}

// ---------------------------------------------------------------------------
// DrawHPBar
// ---------------------------------------------------------------------------

void RPGGame::DrawHPBar(float x, float y, float w, float h, float hp, float maxHp) {
    auto* gfx = GetGraphicsServer();
    // Background
    gfx->DrawQuad(x + w*0.5f, y + h*0.5f, w, h, 0.0f, vec4(0.1f, 0.1f, 0.1f, 0.85f));
    // Fill (green → red based on HP ratio)
    float ratio = hp / maxHp;
    float fillW = (w - 2) * ratio;
    if (fillW > 0)
        gfx->DrawQuad(x+1 + fillW*0.5f, y+1 + (h-2)*0.5f,
                       fillW, h-2, 0.0f,
                       vec4(1-ratio, ratio*0.8f, 0.1f, 1));
}

// ---------------------------------------------------------------------------
// DrawDialog
// ---------------------------------------------------------------------------

void RPGGame::DrawDialog() {
    auto* gfx = GetGraphicsServer();
    const float W = (float)_screenW, H = (float)_screenH;
    const float bx = 40, bh = 70, by = H - bh - 20, bw = W - 80;

    // Box background
    gfx->DrawQuad(bx + bw*0.5f, by + bh*0.5f, bw, bh,
                   0.0f, vec4(0.05f, 0.05f, 0.1f, 0.92f));
    // Top accent bar
    gfx->DrawQuad(bx + bw*0.5f, by + 1.5f, bw, 3.0f,
                   0.0f, vec4(0.5f, 0.8f, 1.0f, 1.0f));

    if (_fontID)
        gfx->DrawText(_fontID, _dialogText,
                       bx + 12, by + 14, 0.7f,
                       vec4(0.9f, 0.9f, 1.0f, 1.0f));
}
