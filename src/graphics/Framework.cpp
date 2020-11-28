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

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true); // platform binding
    ImGui_ImplOpenGL3_Init("#version 410"); // renderer binding

    // Configure graphics     
    glPrimitiveRestartIndex(0xFFFF);
    glCullFace(GL_BACK);
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Initialize framebuffers
    glGenFramebuffers(1, &hdrFramebuffer);
    glGenTextures(1, &hdrColorMap);
    glBindTexture(GL_TEXTURE_2D, hdrColorMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_W, SCREEN_H, 0, GL_RGBA, GL_FLOAT, NULL); 
    glGenTextures(1, &hdrDepthMap);
    glBindTexture(GL_TEXTURE_2D, hdrDepthMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCREEN_W, SCREEN_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); 

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorMap, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, hdrDepthMap, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        runtime_error("HDR Framebuffer is incomplete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &shadowFramebuffer);
    for (int i = 0; i < 1; ++i)
    {
        GLuint map;
        glGenTextures(1, &map);
        glBindTexture(GL_TEXTURE_2D, map);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        uniShadowMaps.push_back(map);
    }
    for (int i = 0; i < 4; ++i)
    {
        GLuint map;
        glGenTextures(1, &map);
        glBindTexture(GL_TEXTURE_CUBE_MAP, map);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        for (int f = 0; f < 6; ++f)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); 
        }
        omniShadowMaps.push_back(map);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    for (int i = 0; i < (int)uniShadowMaps.size(); ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, uniShadowMaps[0], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    for (int i = 0; i < (int)omniShadowMaps.size(); ++i)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, omniShadowMaps[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    vector<GLfloat> verts = {
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
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
            cout << string("Failed to load texture ") + to_string(i) << endl;
        }
        stbi_image_free(data);
    }
}

void Framework::Blit()
{
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
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