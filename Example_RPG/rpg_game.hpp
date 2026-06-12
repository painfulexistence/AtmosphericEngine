#pragma once
#include <Atmospheric/application.hpp>
#include <Atmospheric/tilemap_2d.hpp>
#include <Atmospheric/lighting_2d.hpp>
#include "rpg_entity.hpp"

#include <memory>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// RPGGame — C++ port of 2d-engine/examples/rpg/game.ts
//
// Demonstrates Tilemap2D, LightingSystem2D, SpriteAnimator, combat, dialogue,
// camera follow/clamp, and animated fade-in — all ported from the TypeScript
// reference implementation.
// ---------------------------------------------------------------------------

class RPGGame : public Application {
public:
    RPGGame();

    void OnInit()  override;
    void OnLoad()  override;
    void OnUpdate(float dt, float time) override;

private:
    // ─ camera (world-pixel offsets) ───────────────────────────────────
    float _camX = 0, _camY = 0;
    int   _screenW = 0, _screenH = 0;

    void CameraFollow(float targetCX, float targetCY);
    void CameraClamp(int mapW, int mapH);

    // ─ resources ──────────────────────────────────────────────
    uint32_t _tilesetTex  = 0;
    uint32_t _playerTex   = 0;
    uint32_t _enemyTex    = 0;
    uint32_t _npcTex      = 0;  // solid-colour 1×1 texture
    FontID   _fontID      = 0;

    std::unique_ptr<Tilemap2D> _tilemap;
    LightingSystem2D           _lighting;

    // ─ entities ──────────────────────────────────────────────
    Player               _player;
    SpriteAnimator       _playerAnim;

    std::vector<Enemy>           _enemies;
    std::vector<SpriteAnimator>  _enemyAnims;
    std::vector<NPC>             _npcs;

    // ─ game state ─────────────────────────────────────────────
    float       _fadeIn    = 1.0f;
    float       _hitFlash  = 0.0f;
    float       _time      = 0.0f;
    std::string _dialogText;
    float       _dialogTimer = 0.0f;

    void UpdatePlayer(float dt);
    void UpdateEnemies(float dt);
    void DrawScene();
    void DrawHUD();
    void DrawDialog();
    void DrawHPBar(float x, float y, float w, float h, float hp, float maxHp);

    static AnimClip MakeClip(int sheetCols, int frameRow,
                             std::vector<int> frameCols,
                             float frameDur, bool loop = true);
};
