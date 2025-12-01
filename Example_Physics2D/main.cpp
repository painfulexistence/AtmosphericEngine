#include "Atmospheric.hpp"
#include <random>

class Physics2DDemo : public Application {
    using Application::Application;

    std::vector<GameObject*> dynamicBodies;
    GameObject* ground;
    std::vector<GameObject*> walls;

    std::mt19937 rng;
    float spawnTimer = 0.0f;

    void OnLoad() override {
        rng.seed(42);

        // Set up orthographic camera for 2D view
        mainCamera->SetOrthographic(800.0f, 600.0f, -100.0f, 100.0f);
        mainCamera->gameObject->SetPosition(glm::vec3(400.0f, 300.0f, 0.0f));

        // Set gravity (positive Y = down in screen space)
        physics2D.SetGravity(glm::vec2(0.0f, 500.0f));

        // Create ground (static body)
        CreateGround();

        // Create walls
        CreateWalls();

        // Set up collision callbacks
        physics2D.SetBeginContactCallback([](Rigidbody2DComponent* a, Rigidbody2DComponent* b) {
            // Flash color on collision
            if (a->gameObject) {
                auto* sprite = a->gameObject->GetComponent<SpriteComponent>();
                if (sprite) {
                    glm::vec4 color = sprite->GetColor();
                    sprite->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, color.a));
                }
            }
        });

        script.Print("Physics2D Demo - Press SPACE to spawn shapes, R to reset");
    }

    void CreateGround() {
        ground = CreateGameObject(glm::vec2(400.0f, 550.0f));

        // Add sprite for visualization
        SpriteProps spriteProps;
        spriteProps.size = glm::vec2(700.0f, 30.0f);
        spriteProps.color = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
        spriteProps.layer = CanvasLayer::LAYER_WORLD;
        ground->AddComponent<SpriteComponent>(spriteProps);

        // Add static rigidbody
        Rigidbody2DProps rbProps;
        rbProps.type = BodyType2D::Static;
        rbProps.shape.type = ShapeType2D::Box;
        rbProps.shape.boxSize = glm::vec2(700.0f, 30.0f);
        ground->AddComponent<Rigidbody2DComponent>(rbProps);
    }

    void CreateWalls() {
        // Left wall
        auto leftWall = CreateGameObject(glm::vec2(30.0f, 300.0f));
        SpriteProps leftSpriteProps;
        leftSpriteProps.size = glm::vec2(20.0f, 500.0f);
        leftSpriteProps.color = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
        leftWall->AddComponent<SpriteComponent>(leftSpriteProps);

        Rigidbody2DProps leftRbProps;
        leftRbProps.type = BodyType2D::Static;
        leftRbProps.shape.type = ShapeType2D::Box;
        leftRbProps.shape.boxSize = glm::vec2(20.0f, 500.0f);
        leftWall->AddComponent<Rigidbody2DComponent>(leftRbProps);
        walls.push_back(leftWall);

        // Right wall
        auto rightWall = CreateGameObject(glm::vec2(770.0f, 300.0f));
        SpriteProps rightSpriteProps;
        rightSpriteProps.size = glm::vec2(20.0f, 500.0f);
        rightSpriteProps.color = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
        rightWall->AddComponent<SpriteComponent>(rightSpriteProps);

        Rigidbody2DProps rightRbProps;
        rightRbProps.type = BodyType2D::Static;
        rightRbProps.shape.type = ShapeType2D::Box;
        rightRbProps.shape.boxSize = glm::vec2(20.0f, 500.0f);
        rightWall->AddComponent<Rigidbody2DComponent>(rightRbProps);
        walls.push_back(rightWall);
    }

    void SpawnRandomShape(glm::vec2 position) {
        std::uniform_int_distribution<int> shapeDist(0, 2);
        std::uniform_real_distribution<float> colorDist(0.3f, 1.0f);
        std::uniform_real_distribution<float> sizeDist(20.0f, 50.0f);

        int shapeType = shapeDist(rng);
        glm::vec4 color(colorDist(rng), colorDist(rng), colorDist(rng), 1.0f);

        auto body = CreateGameObject(position);

        Rigidbody2DProps rbProps;
        rbProps.type = BodyType2D::Dynamic;
        rbProps.shape.density = 1.0f;
        rbProps.shape.friction = 0.3f;
        rbProps.shape.restitution = 0.4f;

        SpriteProps spriteProps;
        spriteProps.color = color;
        spriteProps.layer = CanvasLayer::LAYER_WORLD;

        switch (shapeType) {
            case 0: {
                // Box
                float size = sizeDist(rng);
                spriteProps.size = glm::vec2(size, size);
                rbProps.shape.type = ShapeType2D::Box;
                rbProps.shape.boxSize = glm::vec2(size, size);
                break;
            }
            case 1: {
                // Circle (rendered as sprite, physics as circle)
                float radius = sizeDist(rng) * 0.5f;
                spriteProps.size = glm::vec2(radius * 2.0f, radius * 2.0f);
                rbProps.shape.type = ShapeType2D::Circle;
                rbProps.shape.circleRadius = radius;
                break;
            }
            case 2: {
                // Polygon (triangle or pentagon)
                std::uniform_int_distribution<int> vertDist(3, 5);
                int numVerts = vertDist(rng);
                float size = sizeDist(rng);

                std::vector<glm::vec2> vertices;
                for (int i = 0; i < numVerts; ++i) {
                    float angle = (2.0f * glm::pi<float>() * i) / numVerts - glm::pi<float>() / 2.0f;
                    vertices.push_back(glm::vec2(std::cos(angle) * size * 0.5f, std::sin(angle) * size * 0.5f));
                }

                spriteProps.size = glm::vec2(size, size);
                rbProps.shape.type = ShapeType2D::Polygon;
                rbProps.shape.polygonVertices = vertices;
                break;
            }
        }

        body->AddComponent<SpriteComponent>(spriteProps);
        body->AddComponent<Rigidbody2DComponent>(rbProps);
        dynamicBodies.push_back(body);
    }

    void OnUpdate(float dt, float time) override {
        // Spawn shapes on space press
        if (input.IsKeyPressed(Key::SPACE)) {
            std::uniform_real_distribution<float> xDist(100.0f, 700.0f);
            SpawnRandomShape(glm::vec2(xDist(rng), 50.0f));
        }

        // Auto-spawn every second
        spawnTimer += dt;
        if (spawnTimer > 1.0f && dynamicBodies.size() < 50) {
            std::uniform_real_distribution<float> xDist(100.0f, 700.0f);
            SpawnRandomShape(glm::vec2(xDist(rng), 50.0f));
            spawnTimer = 0.0f;
        }

        // Reset colors back to original (fade effect)
        for (auto* body : dynamicBodies) {
            auto* sprite = body->GetComponent<SpriteComponent>();
            if (sprite) {
                glm::vec4 color = sprite->GetColor();
                // Fade white back to original color
                color.r = glm::mix(color.r, 0.7f, dt * 2.0f);
                color.g = glm::mix(color.g, 0.5f, dt * 2.0f);
                color.b = glm::mix(color.b, 0.8f, dt * 2.0f);
                sprite->SetColor(color);
            }
        }

        // Draw debug shapes using new shape API
        auto* renderer = graphics.renderer->GetBatchRenderer();

        // Draw circles around each circle body
        for (auto* body : dynamicBodies) {
            auto* rb = body->GetComponent<Rigidbody2DComponent>();
            if (rb && rb->GetShapeDef().type == ShapeType2D::Circle) {
                glm::vec2 pos = rb->GetPosition();
                float radius = rb->GetShapeDef().circleRadius;
                renderer->BeginScene(mainCamera->GetProjectionMatrix() * mainCamera->GetViewMatrix());
                renderer->DrawCircle(pos, radius + 2.0f, glm::vec4(1.0f, 1.0f, 0.0f, 0.5f), 16);
                renderer->EndScene();
            }
        }

        // Reset scene
        if (input.IsKeyPressed(Key::R)) {
            ReloadScene();
        }

        if (input.IsKeyDown(Key::ESCAPE)) {
            Quit();
        }
    }
};

int main(int argc, char* argv[]) {
    Physics2DDemo game({
      .windowTitle = "Physics2D Demo - Polygon Collision",
      .windowWidth = 800,
      .windowHeight = 600,
      .useDefaultTextures = true,
      .useDefaultShaders = true,
    });
    game.Run();
    return 0;
}
