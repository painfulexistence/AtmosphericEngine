#pragma once
#include "globals.hpp"
#include "server.hpp"
#include "window.hpp"

class Input : public Server
{
private:
    static Input* _instance;

public:
    static Input* Get()
    {
        return _instance;
    }

    Input();

    ~Input();

    void Process(float dt) override;

    bool GetKeyDown(Key key);

    bool GetKeyUp(Key key);

    bool IsKeyDown(Key key);

    bool IsKeyUp(Key key);

    glm::vec2 GetMousePosition();

private:
    std::unordered_map<Key, KeyState> _keyStates;
};