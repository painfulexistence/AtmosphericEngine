#include "Window/Framework.hpp"

using namespace std;


static void OnError(int errorCode, const char* msg)
{
    cout << "GLFW Error " << msg << endl;
}

static void OnCursorMove(GLFWwindow* winodw, double xPos, double yPos)
{
    //cout << "Cursor at: (" << xPos << ", " << yPos << ")" << endl;
}

static void OnCursorEnterLeave(GLFWwindow* winodw, int entered)
{
    //cout << "Cursor " << (entered ? "entered" : "left") << endl;
}

Framework::Framework()
{
    glfwSetErrorCallback(OnError);
    if (!glfwInit())
        throw runtime_error("Failed to initialize glfw!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow((int)SCREEN_W, (int)SCREEN_H, "Atmospheric", NULL, NULL);
    if (window == nullptr)
        throw runtime_error("Failed to initialize window!");

    glfwMakeContextCurrent(window);
    #if VSYNC_ON
    glfwSwapInterval(1);
    #endif

    // Setup input module
    if (glfwRawMouseMotionSupported())
    {
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwSetCursorPosCallback(window, OnCursorMove);
}

Framework::~Framework()
{
    glfwTerminate();
}

    
void Framework::SwapBuffers()
{
    glfwSwapBuffers(window);
}

void Framework::PollEvents()
{
    glfwPollEvents();

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        CloseWindow();
}

void Framework::CloseWindow()
{
    glfwSetWindowShouldClose(window, GL_TRUE);
}