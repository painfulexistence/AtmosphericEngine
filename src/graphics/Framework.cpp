#include "Framework.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;


Framework::Framework() {}

Framework::~Framework() {}

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

void Framework::Init()
{
    glfwSetErrorCallback(OnError);
    if (!glfwInit())
        throw runtime_error("Failed to initialize glfw!");

    const char* glsl_version = "#version 410";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow((int)SCREEN_W, (int)SCREEN_H, "Atmospheric", NULL, NULL);
    if (window == nullptr)
        throw runtime_error("Failed to initialize window!");

    glfwMakeContextCurrent(window);
    #ifdef VSYNC_ON
    glfwSwapInterval(1);
    #endif

    // Setup OpenGL extension loader
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw runtime_error("Failed to initialize glew!");

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
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void Framework::Textures(const vector<string>& paths)
{
    for (int i = 0; i < paths.size(); ++i)
    {
        GLuint tex;
        glGenTextures(1, &tex);
        int width, height, nChannels;
        unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nChannels, 0);
        if (data) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            throw runtime_error(string("Failed to load image ") + to_string(i));
        }
        stbi_image_free(data);
    }
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
