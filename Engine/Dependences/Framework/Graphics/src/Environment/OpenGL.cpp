//
// Created by Nikita on 18.11.2020.
//



#include <iostream>
#include "Environment/OpenGL.h"
#include <vector>
#include <string>
#include <Debug.h>
#include <Utils/StringUtils.h>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <ImGuizmo.h>

using namespace Framework::Helper;

unsigned int Framework::Graphics::OpenGL::CreateTexture(unsigned char *pixels, int w, int h, int components) {
    GLuint textureID;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, components, w, h, 0, (components == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    return (unsigned int)textureID;
}

bool Framework::Graphics::OpenGL::PreInitGUI(const std::string& fontPath) {
    Debug::Graph("OpenGL::InitGUI() : pre-initializing ImGUI library...");

    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Set font
        //ImFont* pFont = io.Fonts->AddFontFromFileTTF((Helper::ResourceManager::GetResourcesFolder() + "\\Fonts\\CalibriL.ttf").c_str(), 12.0f);
        ImFont* pFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 12.0f);

        //io.IniFilename = NULL;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        //io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigDockingWithShift       = true;
        io.ConfigWindowsResizeFromEdges = true;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        //io.ConfigDockingWithShift = true;
        // Setup Dear ImGui style
        //ImGui::StyleColorsDark();
    }

    return true;
}

bool Framework::Graphics::OpenGL::InitGUI() {
    Debug::Graph("OpenGL::InitGUI() : initializing ImGUI library...");

    ImGui_ImplGlfw_InitForOpenGL(this->m_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    return true;
}


bool Framework::Graphics::OpenGL::StopGUI() {
    Debug::Graph("OpenGL::StopGUI() : stopping ImGUI library...");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
    return true;
}

bool Framework::Graphics::OpenGL::PreInit(unsigned int smooth_samples) {
    Helper::Debug::Graph("OpenGL::PreInit() : initializing glfw...");

    if (!glfwInit()) {
        Helper::Debug::Error("OpenGL::PreInit() : failed initializing glfw!");
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, (int)smooth_samples); // 4x сглаживание
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    return true;
}

bool Framework::Graphics::OpenGL::MakeWindow(const char* winName, Types::WindowFormat* format, bool fullScreen) {
    this->m_winFormat = format;

    m_monitor = glfwGetPrimaryMonitor();
    m_vidMode = glfwGetVideoMode(m_monitor);

    m_window = glfwCreateWindow((int)m_winFormat->Width(), (int)m_winFormat->Height(), winName, fullScreen ? m_monitor : nullptr, nullptr);

    return m_window != nullptr;

}

bool Framework::Graphics::OpenGL::SetContextCurrent() {
    glfwMakeContextCurrent(m_window);

    return true;
}

bool Framework::Graphics::OpenGL::Init(int swapInterval) {
    this->m_screenSize = { this->m_vidMode->width, this->m_vidMode->height };
    glfwSwapInterval(swapInterval);

    glfwSetWindowFocusCallback(m_window, [](GLFWwindow* win, int focus) {
        g_callback(WinEvents::Focus, win, &focus, nullptr);
    });

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* win, int w, int h) {
        g_callback(WinEvents::Resize, win, &w, &h);
    });

    glfwSetWindowCloseCallback(m_window, (GLFWwindowclosefun)[](GLFWwindow* win) {
        g_callback(WinEvents::Close, win, nullptr, nullptr);
    });

    glfwSetWindowPosCallback(m_window, [](GLFWwindow* win, int x, int y){
         g_callback(WinEvents::Move, win, &x, &y);
    });

    glfwSetScrollCallback(m_window, [](GLFWwindow* win, double xoffset, double yoffset){
        for (const auto& a : g_scrollEvents)
            a(xoffset, yoffset);

        g_callback(WinEvents::Scroll, win, &xoffset, &yoffset);
    });

    Helper::Debug::Graph("OpenGL::Init() : initializing glew...");

    glewExperimental = TRUE;
    if (glewInit() != GLEW_OK) {
        Helper::Debug::Error("OpenGL::Init() : failed initializing glew!");
        return false;
    }

    return true;
}

bool Framework::Graphics::OpenGL::PostInit() {

    glEnable(GL_BLEND); // Прозрачность стекла
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glBlendEquation(GL_FUNC_ADD);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_MULTISAMPLE);

    ///\%info ПРОВЕРЯЕМ ГЛУБИНУ, ЧТОБЫ ИЗБАВИТЬСЯ ОТ "ЭФФЕКТИА ПЕРЕКРЫТИЯ" ДАЛЬНИМИ ОБЪЕКТАМИ
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // Отсечение граней

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Действительно хорошие вычисления перспективы

    return true;
}

bool Framework::Graphics::OpenGL::CloseWindow() {

    return true;
}

bool Framework::Graphics::OpenGL::IsWindowOpen() {
    return !glfwWindowShouldClose(m_window);
}

void Framework::Graphics::OpenGL::PoolEvents() {
    if (Helper::Debug::Profile()) {
        EASY_FUNCTION(profiler::colors::Green);
    }

    glfwPollEvents();
}

void Framework::Graphics::OpenGL::SetWindowSize(float ratio, unsigned int w, unsigned int h) {
    if (m_winFormat->GetValue() != Types::WindowFormat::Free) {
        w = m_winFormat->Width();
        h = m_winFormat->Height();
    }

    if (Debug::GetLevel() >= Debug::Level::High)
        Debug::Log("OpenGL::SetWindowSize() : width = "+std::to_string(w) + "; height = "+ std::to_string(h));

    glfwSetWindowSize(m_window, w, h);
    glViewport(0, 0, w, h);// определ¤ем окно просмотра

    //glMatrixMode(GL_PROJECTION);// используем матрицу проекции
    //glLoadIdentity();// Reset матрицы
    //glViewport(0, 0, w, h);// определ¤ем окно просмотра
    //gluPerspective(45, ratio, 0.1, 8000);// установить корректную перспективу.
    //glMatrixMode(GL_MODELVIEW);// вернутьс¤ к модели
}

void Framework::Graphics::OpenGL::SetWindowPosition(int x, int y) {
    glfwSetWindowPos(m_window, x, y);
}

bool Framework::Graphics::OpenGL::FreeMesh(unsigned int VAO) noexcept {
    if (Helper::Debug::GetLevel() >= Helper::Debug::Level::High)
        Helper::Debug::Log("OpenGL::FreeMesh() : free mesh \""+std::to_string(VAO) + "\" VAO...");

    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        return true;
    }else{
        Helper::Debug::Error("OpenGL::FreeMesh() : VAO is zero! Something went wrong...");
        return false;
    }
}

bool Framework::Graphics::OpenGL::CompileShader(std::string path, unsigned int *fragment, unsigned int *vertex) {
    std::string vertex_path = path + "_vertex.glsl";
    std::string fragment_path = path + "_fragment.glsl";;

    int	  InfoLogLength	= 0;
    GLint Result		= GL_FALSE;

    //! создаем шейдеры
    *vertex   =   glCreateShader(GL_VERTEX_SHADER);
    *fragment =   glCreateShader(GL_FRAGMENT_SHADER);

    //! читаем вершинный шейдер из файла
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::string Line = "";
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    //! читаем фрагментный шейдер из файла
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::string Line = "";
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    std::string error;

    {
        // Компилируем вершинный шейдер
        char const* VertexSourcePointer = VertexShaderCode.c_str();
        glShaderSource(*vertex, 1, &VertexSourcePointer, NULL);
        glCompileShader(*vertex);

        // Устанавливаем параметры
        glGetShaderiv(*vertex, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(*vertex, GL_INFO_LOG_LENGTH, &InfoLogLength);

        //?===========================================[ ERRORS ]==============================================
        if (InfoLogLength != 0) {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength);
            glGetShaderInfoLog(*vertex, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
            if (VertexShaderErrorMessage.size() > 10)
                Debug::Error("OpenGL::CompileShader : Failed compiling vertex shader!\n\tReason : " + std::string(VertexShaderErrorMessage.data()));
        }
        //?===================================================================================================
    }

    {
        // Компилируем фрагментный шейдер
        char const* FragmentSourcePointer = FragmentShaderCode.c_str();
        glShaderSource(*fragment, 1, &FragmentSourcePointer, nullptr);
        glCompileShader(*fragment);

        // Устанавливаем параметры
        glGetShaderiv(*fragment, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(*fragment, GL_INFO_LOG_LENGTH, &InfoLogLength);

        //?===========================================[ ERRORS ]==============================================
        if (InfoLogLength != 0) {
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
            glGetShaderInfoLog(*fragment, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
            if (FragmentShaderErrorMessage.size() > 10) {
                bool isError = StringUtils::Contains(std::string(FragmentShaderErrorMessage.data()), "error");

                if (isError)
                    Debug::Error("OpenGL::CompileShader() : Failed compiling fragment shader!\n\tReason : " +
                                 std::string(FragmentShaderErrorMessage.data()));
                else
                    Debug::Warn("OpenGL::CompileShader() : There are warnings in the shader!\n\tReason: " +
                                 std::string(FragmentShaderErrorMessage.data()));
            }
        }
        //?===================================================================================================
    }

    return true;
}

unsigned int Framework::Graphics::OpenGL::LinkShader(unsigned int *fragment, unsigned int *vertex) {
    unsigned int ProgramID = glCreateProgram();
    glAttachShader(ProgramID, *vertex);
    glAttachShader(ProgramID, *fragment);
    glLinkProgram(ProgramID);

    int	  InfoLogLength	= 0;
    GLint Result		= GL_FALSE;

    // Устанавливаем параметры
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    //?===========================================[ ERRORS ]==============================================
    if (InfoLogLength != 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::string error = StringUtils::FromCharVector(ProgramErrorMessage);
        if (!error.empty()) {
            size_t index = error.find("error");
            if (index == 18446744073709551615) {
                Debug::Warn("OpenGL::LinkShader : Warning linking program! Reason : " + error);
            }
            else {
                Debug::Error("OpenGL::LinkShader : Failed linking program! Reason : " + error);
                return 0;
            }
        }
    }
    //?===================================================================================================

    glDeleteShader(*vertex);
    glDeleteShader(*fragment);

    return ProgramID;
}

bool Framework::Graphics::OpenGL::CreateHDRFrameBufferObject(glm::vec2 size, unsigned int& rboDepth, unsigned int &hdrFBO, std::vector<unsigned int>& colorBuffers) {
    bool isNew = !((bool)hdrFBO);

    if (size.x == 0 || size.y == 0){
        Helper::Debug::Error("OpenGL::CreateHDRFrameBufferObject() : frame buffer has incorrect size!");
        return false;
    }

    if (isNew){
        Helper::Debug::Log("OpenGL::CreateHDRFrameBufferObject() : creating new frame buffers...");
    }

    if (isNew)
        glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create n floating point color buffers (1 for normal rendering, other for brightness threshold values)

    if (isNew)
        glGenTextures(colorBuffers.size(), colorBuffers.data());

    for (unsigned int i = 0; i < colorBuffers.size(); i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    if (isNew)
        glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    //unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    std::vector<unsigned int> attachments;
    for (unsigned int i = 0; i < colorBuffers.size(); i++)
    {
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    glDrawBuffers(attachments.size(), attachments.data());

    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Debug::Error("OpenGL::CreateHDRFrameBufferObject() : frame buffer is not complete!");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (isNew){
        std::string colorBfs = "\n\tColorBuffers: ";
        for (size_t t = 0; t < colorBuffers.size(); t++)
            colorBfs += std::to_string(colorBuffers[t]) + (t+1 == colorBuffers.size() ? "" : ",");

        Debug::Log("OpenGL::CreateHDRFrameBufferObject() : successful!\n\tRBODepth: "+std::to_string(rboDepth)+
            "\n\tHDR_FBO: "+std::to_string(hdrFBO) + colorBfs);
    }

    return true;
}

bool Framework::Graphics::OpenGL::CreatePingPongFrameBufferObject(
        glm::vec2 size,
        std::vector<unsigned int> &pingpongFBO,
        std::vector<unsigned int>& pingpongColorBuffers)
{
    bool isNew = pingpongFBO[0] == 0;

    if (isNew) {
        Debug::Log("OpenGL::CreatePingPongFrameBufferObject() : creating ping-pong frame buffers...");
        glGenFramebuffers(pingpongFBO.size(), pingpongFBO.data());
        glGenTextures(pingpongColorBuffers.size(), pingpongColorBuffers.data());
    }

    for (unsigned int i = 0; i < pingpongFBO.size(); i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorBuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Debug::Error("OpenGL::CreatePingPongFrameBufferObject() : frame buffer is not complete!");
            return false;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (isNew){
        std::string PingPongFBOBufs = "\n\tPingPongFBO: ";
        for (size_t t = 0; t < pingpongFBO.size(); t++)
            PingPongFBOBufs += std::to_string(pingpongFBO[t]) + (t+1 == pingpongFBO.size() ? "" : ",");

        std::string PingPongColorBufs = "\n\tPingPongColorBuffers: ";
        for (size_t t = 0; t < pingpongColorBuffers.size(); t++)
            PingPongColorBufs += std::to_string(pingpongColorBuffers[t]) + (t+1 == pingpongColorBuffers.size() ? "" : ",");

        Debug::Log("OpenGL::CreateHDRFrameBufferObject() : successful!"+PingPongFBOBufs+PingPongColorBufs);
    }

    return true;
}

unsigned int Framework::Graphics::OpenGL::CalculateTexture(
        unsigned char *data, int format, unsigned int w, unsigned int h,
        Framework::Graphics::TextureFilter filter, bool alpha
) {
    //glActiveTexture(GL_TEXTURE0);

    unsigned int id;
    glGenTextures(1, &id);

    GLuint gl_filter = 0;
    switch(filter){
        case TextureFilter::NEAREST:
            gl_filter = GL_NEAREST; break;
        case TextureFilter::LINEAR:
            gl_filter = GL_LINEAR; break;
        case TextureFilter::NEAREST_MIPMAP_NEAREST:
            gl_filter = GL_NEAREST_MIPMAP_NEAREST; break;
        case TextureFilter::LINEAR_MIPMAP_NEAREST:
            gl_filter = GL_LINEAR_MIPMAP_NEAREST; break;
        case TextureFilter::NEAREST_MIPMAP_LINEAR:
            gl_filter = GL_NEAREST_MIPMAP_LINEAR; break;
        case TextureFilter::LINEAR_MIPMAP_LINEAR:
            gl_filter = GL_LINEAR_MIPMAP_LINEAR; break;
    }

    Helper::Debug::Log("OpenGL::CalculateTexture() : calculating (ID "+std::to_string(id)+") texture...");

    glBindTexture(GL_TEXTURE_2D, id);

    GLuint compress = format == 4 ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT; //GL_RGBA16F
    glTexImage2D(GL_TEXTURE_2D, 0, compress, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);

    GLuint texParam = GL_CLAMP_TO_BORDER; // GL_REPEAT, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParam); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParam);

    /*
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParam);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);

    //gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, m_image->m_width, m_image->m_height, m_image->m_format,
    //	GL_UNSIGNED_BYTE, (unsigned int*)m_image->m_data);

    if(!alpha)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, w, h,
                     0, format, GL_UNSIGNED_BYTE, (unsigned int*)data);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, w, h,
                     0, format, GL_UNSIGNED_BYTE, (unsigned int*)data);


       // GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT - black-white
       // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
       // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
       // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
       // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // What it need?
    */

    glBindTexture(GL_TEXTURE_2D, 0);

    return id;
}

unsigned int Framework::Graphics::OpenGL::CalculateCubeMap(unsigned int w, unsigned int h, std::vector<unsigned char*> data) {
    unsigned int cubemap = 0;

    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    for (unsigned char c = 0; c < 6; c++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + c,
                     0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data[c]
        );
        delete data[c];
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return cubemap;
}

unsigned int Framework::Graphics::OpenGL::CalculateSkybox() noexcept {
    const float skyboxVertices[36 * 3] = {
            // positions
            -10.0f,  10.0f, -10.0f,
            -10.0f, -10.0f, -10.0f,
            10.0f, -10.0f, -10.0f,
            10.0f, -10.0f, -10.0f,
            10.0f,  10.0f, -10.0f,
            -10.0f,  10.0f, -10.0f,

            -10.0f, -10.0f,  10.0f,
            -10.0f, -10.0f, -10.0f,
            -10.0f,  10.0f, -10.0f,
            -10.0f,  10.0f, -10.0f,
            -10.0f,  10.0f,  10.0f,
            -10.0f, -10.0f,  10.0f,

            10.0f, -10.0f, -10.0f,
            10.0f, -10.0f,  10.0f,
            10.0f,  10.0f,  10.0f,
            10.0f,  10.0f,  10.0f,
            10.0f,  10.0f, -10.0f,
            10.0f, -10.0f, -10.0f,

            -10.0f, -10.0f,  10.0f,
            -10.0f,  10.0f,  10.0f,
            10.0f,  10.0f,  10.0f,
            10.0f,  10.0f,  10.0f,
            10.0f, -10.0f,  10.0f,
            -10.0f, -10.0f,  10.0f,

            -10.0f,  10.0f, -10.0f,
            10.0f,  10.0f, -10.0f,
            10.0f,  10.0f,  10.0f,
            10.0f,  10.0f,  10.0f,
            -10.0f,  10.0f,  10.0f,
            -10.0f,  10.0f, -10.0f,

            -10.0f, -10.0f, -10.0f,
            -10.0f, -10.0f,  10.0f,
            10.0f, -10.0f, -10.0f,
            10.0f, -10.0f, -10.0f,
            -10.0f, -10.0f,  10.0f,
            10.0f, -10.0f,  10.0f
    };

    unsigned int VAO = 0;
    unsigned int VBO = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);

    return VAO;
}

void Framework::Graphics::OpenGL::DrawSkybox(const unsigned int&  VAO, unsigned int CubeMap) const noexcept {
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    // ... задание видовой и проекционной матриц
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS); // set depth function back to default
}

void Framework::Graphics::OpenGL::BeginDrawGUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(true);

    ImGui::GetStyle().WindowRounding = 0.0f;
}

void Framework::Graphics::OpenGL::EndDrawGUI() {
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(this->m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Framework::Graphics::OpenGL::CreateSingleHDRFrameBO(glm::vec2 size, unsigned int &rboDepth, unsigned int &hdrFBO, unsigned int &colorBuffer) {
    bool isNew = !((bool)hdrFBO);

    if (isNew)
        Debug::Graph("OpenGL::CreateSingleHDRFrameBO() : creating new single frame buffer object...");

    if (isNew)
        glGenFramebuffers(1, &hdrFBO);

    if (isNew)
        glGenTextures(1, &colorBuffer);

    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    //================================================================================================

    if (colorBuffer && hdrFBO) {
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        // присоедиение текстуры к объекту текущего кадрового буфера
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    }
    else {
        Debug::Error("OpenGL::CreateSingleHDRFrameBO() : color buffer or FBO is not created!");
        return false;
    }

    if (isNew)
        glGenRenderbuffers(1, &rboDepth);

    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Debug::Error("OpenGL::CreateSingleHDRFrameBO() : frame buffer is not complete!");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (isNew){
        Debug::Log("OpenGL::CreateSingleHDRFrameBO() : successful!\n\tRBODepth: "+std::to_string(rboDepth)+
                   "\n\tHDR_FBO: "+std::to_string(hdrFBO) + "\n\tColor buffer: "+std::to_string(colorBuffer));
    }

    return true;
}

void Framework::Graphics::OpenGL::SetDepthTestEnabled(bool value) {
    if (value)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}



