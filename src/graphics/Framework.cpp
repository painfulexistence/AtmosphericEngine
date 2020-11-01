#include "Framework.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

void Framework::Init()
{
    glfwSetErrorCallback(OnError);
    if (!glfwInit())
        throw runtime_error("Failed to initialize glfw!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

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
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwSetCursorPosCallback(window, OnCursorMove);

    // Setup GL extension loader
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw runtime_error("Failed to initialize glew!");

    // Setup graphics
    glGenFramebuffers(1, &shadowFramebuffer);
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    
    vector<string> paths = {
        "./resources/beach.png",
        "./resources/starnight.jpg",
        "./resources/grass.png",
        "./resources/brick.jpg",
        "./resources/metal.jpg"
    };
    AddTextures(paths);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Dear ImGui platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void Framework::AddTextures(const vector<string>& paths)
{
    for (int i = 0; i < paths.size(); ++i)
    {   
        GLuint tex;
        glGenTextures(1, &tex);
        textures.push_back(tex);    
        
        glBindTexture(GL_TEXTURE_2D, tex);
        float border[] = {1.f, 1.f, 1.f, 1.f};
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);  
        
        int width, height, nChannels;
        unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            cout << string("Failed to load image ") + to_string(i) << endl;
        }
        stbi_image_free(data);
    }
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

void Framework::Terminate()
{
    glfwTerminate();
}

void Framework::CheckErrors()
{     
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode)
        {
            // Reference: https://learnopengl.com/In-Practice/Debugging
            case GL_INVALID_ENUM:                  
            error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 
            error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             
            error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                
            error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               
            error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 
            error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: 
            error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << "OpenGL Error " << error << endl;
    }
}