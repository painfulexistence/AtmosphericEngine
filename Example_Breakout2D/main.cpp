#include "Atmospheric.hpp"

class Breakout : public Application {
    int windowWidth, windowHeight;
    std::vector<GameObject*> sprites;

    void OnLoad() override {
        auto windowSize = Window::Get()->GetFramebufferSize();
        windowWidth = windowSize.width;
        windowHeight = windowSize.height;

        LoadScene({
            .textures = {
                "assets/textures/default_diff.jpg",
                "assets/textures/default_norm.jpg",
                "assets/textures/default_ao.jpg",
                "assets/textures/default_rough.jpg",
                "assets/textures/default_metallic.jpg"
            },
            .shaders = {
                {
                    "color", {
                        .vert = "assets/shaders/tbn.vert",
                        .frag = "assets/shaders/pbr.frag"
                    },
                },
                {
                    "debug_line", {
                        .vert = "assets/shaders/debug.vert",
                        .frag = "assets/shaders/flat.frag",
                    }
                },
                {
                    "depth", {
                        .vert = "assets/shaders/depth_simple.vert",
                        .frag = "assets/shaders/depth_simple.frag"
                    },
                },
                {
                    "depth_cubemap", {
                        .vert = "assets/shaders/depth_cubemap.vert",
                        .frag = "assets/shaders/depth_cubemap.frag"
                    },
                },
                {
                    "hdr", {
                        .vert = "assets/shaders/hdr.vert",
                        .frag = "assets/shaders/hdr_ca.frag"
                    },
                },
                {
                    "terrain", {
                        .vert = "assets/shaders/terrain.vert",
                        .frag = "assets/shaders/terrain.frag",
                        .tesc = "assets/shaders/terrain.tesc",
                        .tese = "assets/shaders/terrain.tese"
                    },
                },
                {
                    "canvas", {
                        .vert = "assets/shaders/canvas.vert",
                        .frag = "assets/shaders/canvas.frag",
                    }
                }
            },
            .materials = {
                {
                    .baseMap = 0,
                    .normalMap = 1,
                    .aoMap = 2,
                    .roughnessMap = 3,
                    .diffuse = {1., 1., 1.},
                    .specular = {.296648, .296648, .296648},
                    .ambient = {.25, .20725, .20725},
                    .shininess = 0.088
                }
            },
            .gameObjects = {}
        });

        for (int i = 0; i < 500; i++) {
            auto sprite = CreateGameObject();
            sprite->AddDrawable2D();
            sprite->SetPosition(glm::vec3(rand() % windowWidth, rand() % windowHeight, 0.0f));
            sprites.push_back(sprite);
        }
    }

    void OnUpdate(float dt, float time) override {
        for (auto sprite : sprites) {
            sprite->SetRotation(glm::vec3(0.0, 0.0, time * 1.0));
        }

        if (input.IsKeyDown(Key::R)) {
            graphics.ReloadShaders();
        }
        if (input.IsKeyDown(Key::ESCAPE)) {
            Quit();
        }
    }
};

int main(int argc, char* argv[]) {
    Breakout game;
    game.Run();
    return 0;
}