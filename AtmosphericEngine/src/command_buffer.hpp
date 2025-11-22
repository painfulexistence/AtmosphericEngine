#pragma once
#include "globals.hpp"

enum GLCmd_t
{
    SET_CLEAR_COLOR,
    CLEAR_COLOR_DEPTH,
    VIEWPORT_SHADOW,
    VIEWPORT_COLOR,
    DRAW
};

struct GLCommand
{
    GLCommand(GLCmd_t type, std::function<void(void)> func)
    {
        this->type = type;
        this->func = func;
    };
    GLCmd_t type;
    std::function<void(void)> func;
};

class CommandBuffer
{
public:
    CommandBuffer();

    ~CommandBuffer();

    void Record(std::function<void(void)> drawProcess);

    void QueueSubmit();

private:
    std::queue<GLCommand> _queue;
};