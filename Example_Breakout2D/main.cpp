#include "Atmospheric.hpp"

int level_1[10][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

int level_2[10][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

class Breakout : public Application {
    using Application::Application;

    int windowWidth, windowHeight;
    GameObject* bg;
    GameObject* paddle;
    GameObject* ball;
    std::vector<GameObject*> bricks;
    glm::vec2 paddleSize = glm::vec2(100.0f, 20.0f);
    glm::vec2 ballPos = glm::vec2(0.0f);
    glm::vec2 ballSize = glm::vec2(30.0f, 30.0f);
    glm::vec2 paddlePos = glm::vec2(0.0f);
    glm::vec2 ballVel = glm::vec2(0.0f);

    void OnLoad() override {
        srand(time(NULL));

        auto windowSize = Window::Get()->GetFramebufferSize();
        windowWidth = windowSize.width;
        windowHeight = windowSize.height;

        LoadScene({
            .textures = {
                "assets/textures/paddle.png",
                "assets/textures/block.png",
                "assets/textures/block_solid.png",
                "assets/textures/ball.jpeg",
                "assets/textures/fractal.png"
            },
        });

        bg = CreateGameObject(glm::vec2(0, 0));
        bg->AddDrawable2D({
            .size = glm::vec2(windowWidth, windowHeight),
            .color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
            .textureID = 4
        });

        paddle = CreateGameObject(glm::vec2(windowWidth * 0.5f - paddleSize.x * 0.5f, windowHeight - 50.0f));
        paddle->AddDrawable2D({
            .size = paddleSize,
            .color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
            .textureID = 0
        });

        ball = CreateGameObject(glm::vec2(windowWidth * 0.5f - ballSize.x * 0.5f, windowHeight - 80.0f));
        ball->AddDrawable2D({
            .size = ballSize,
            .color = glm::vec4(1.0f),
            .textureID = 3
        });

        LoadLevel(1);
    }

    void OnUpdate(float dt, float time) override {
        if (input.IsKeyDown(Key::LEFT)) {
            auto pos = paddle->GetPosition();
            paddle->SetPosition(glm::vec3(std::max(pos.x - 300.0f * dt, 0.0f), pos.y, pos.z));
        }
        if (input.IsKeyDown(Key::RIGHT)) {
            auto pos = paddle->GetPosition();
            paddle->SetPosition(glm::vec3(std::min(pos.x + 300.0f * dt, windowWidth - paddleSize.x), pos.y, pos.z));
        }
        if (input.IsKeyDown(Key::ESCAPE)) {
            Quit();
        }

        auto ballPos = glm::vec2(ball->GetPosition());
        ballPos += ballVel * dt;
        if (ballPos.x < 0.0f) {
            ballPos.x = 0.0f;
            ballVel.x *= -1.0f;
        }
        if (ballPos.x > windowWidth - ballSize.x) {
            ballPos.x = windowWidth - ballSize.x;
            ballVel.x *= -1.0f;
        }
        if (ballPos.y < 0.0f) {
            ballPos.y = 0.0f;
            ballVel.y *= -1.0f;
        }
        if (ballPos.y > windowHeight - ballSize.y) {
            ballPos.y = windowHeight - ballSize.y;
            ballVel.y *= -1.0f;
        }
        ball->SetPosition(glm::vec3(ballPos.x, ballPos.y, 0.0f));
    }

    void LoadLevel(int levelID) {
        for (auto brick : bricks) {
            delete brick;
        }
        bricks.clear();

        ballVel = 200.0f * glm::normalize(glm::vec2(rand() % 100 - 50, -(rand() % 50)));

        int rows = 5;
        int cols = 8;
        float brickWidth = 80.0f;
        float brickHeight = 30.0f;
        float padding = 10.0f;
        glm::vec3 color = glm::vec3(1.0f, 0.5f, 0.5f);

        switch (levelID) {
        case 1:
            rows = 5;
            cols = 8;
            brickWidth = 80.0f;
            brickHeight = 30.0f;
            padding = 10.0f;
            break;
        default:
            break;
        }

        float startX = (windowWidth - (cols * (brickWidth + padding))) / 2.0f;
        float startY = 100.0f + rows * (brickHeight + padding);

        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                auto brick = CreateGameObject();
                brick->AddDrawable2D({
                    .size = glm::vec2(brickWidth, brickHeight),
                    .textureID = 1
                });

                float x = startX + col * (brickWidth + padding);
                float y = startY - row * (brickHeight + padding);

                brick->SetPosition(glm::vec3(x, y, 0.0f));
                bricks.push_back(brick);
            }
        }
    }
};

int main(int argc, char* argv[]) {
    Breakout game({
        .windowWidth = 480,
        .windowHeight = 720,
        .windowFloating = true,
        .useDefaultTextures = false,
        .enablePhysics3D = false
    });
    game.Run();
    return 0;
}