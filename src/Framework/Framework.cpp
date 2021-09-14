#include "Framework/Framework.hpp"
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
    _window = glfwCreateWindow((int)SCREEN_W, (int)SCREEN_H, "Atmospheric", NULL, NULL);
    if (_window == nullptr)
        throw runtime_error("Failed to initialize window!");
    glfwMakeContextCurrent(_window);
    
    #if VSYNC_ON
    glfwSwapInterval(1);
    #endif

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(_window, true); // platform binding
    ImGui_ImplOpenGL3_Init("#version 410"); // renderer binding

    // Configure Dear ImGui
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup input module
    if (glfwRawMouseMotionSupported())
    {
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwSetCursorPosCallback(_window, OnCursorMove);
}

Framework::~Framework()
{
    glfwTerminate();
}

bool Framework::Run()
{
    glfwPollEvents(); // Snapshot the keyboard
    _time = this->GetTime();
    return glfwWindowShouldClose(_window);
}

void Framework::Draw()
{
    glfwSwapBuffers(_window);
}

float Framework::GetTime()
{
    return (float)glfwGetTime();
}

float Framework::GetFrameTime()
{
    return (float)this->_time;
}

glm::vec2 Framework::GetCursor() 
{
    double xPos, yPos;
    glfwGetCursorPos(_window, &xPos, &yPos);
    return glm::vec2(xPos, yPos);
};

bool Framework::IsKeyDown(int key)
{
    return (glfwGetKey(_window, key) == GLFW_PRESS);
};

bool Framework::IsKeyUp(int key)
{
    return (glfwGetKey(_window, key) == GLFW_RELEASE);
};