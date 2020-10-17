#include "Framework.hpp"

Framework::Framework()
{
    

}

Framework::~Framework()
{

}

static void OnError(int errorCode, const char* msg)
{
    std::cout << "GLFW Error " << msg << std::endl;
}

static void OnCursorMove(GLFWwindow* winodw, double xPos, double yPos)
{
    //std::cout << "Cursor at: (" << xPos << ", " << yPos << ")" << std::endl;
}

static void OnCursorEnterLeave(GLFWwindow* winodw, int entered)
{
    //std::cout << "Cursor " << (entered ? "entered" : "left") << std::endl;
}

void Framework::Init()
{
    glfwSetErrorCallback(OnError);
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize glfw!");

    const char* glsl_version = "#version 410";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow((int)SCREEN_W, (int)SCREEN_H, "Atmospheric", NULL, NULL);
    if (window == nullptr)
        throw std::runtime_error("Failed to initialize glfw!");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup OpenGL extension loader
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize window!");

    // Setup input module
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwSetCursorPosCallback(window, OnCursorMove);
    

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}
    
void Framework::Swap()
{
    glfwSwapBuffers(window);
}

void Framework::Poll()
{
    glfwPollEvents();

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

bool Framework::KeyDown(int key)
{
    return (glfwGetKey(window, key) == GLFW_PRESS);
}

glm::vec2 Framework::CursorPosition()
{
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    return glm::vec2(xPos, yPos);
}

glm::vec2 Framework::CursorUV()
{
    glm::vec2 cursorPos = CursorPosition();
    return glm::vec2(cursorPos.x / SCREEN_W, cursorPos.y / SCREEN_H);
}
    
double Framework::GetTime()
{
    return glfwGetTime();
}

bool Framework::ShouldCloseWindow()
{
    return glfwWindowShouldClose(window) || shouldClose;
}

void Framework::CloseWindow()
{
    shouldClose = true;
}

void Framework::Terminate()
{
    glfwTerminate();
}
