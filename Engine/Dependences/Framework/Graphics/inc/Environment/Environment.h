//
// Created by Nikita on 18.11.2020.
//

#ifndef GAMEENGINE_ENVIRONMENT_H
#define GAMEENGINE_ENVIRONMENT_H

#include <Debug.h>
#include <vector>
#include <Types/WindowFormat.h>
#include <functional>
#include <glm/glm.hpp>
#include <Environment/Vertex.h>
#include <Environment/TextureFilters.h>

namespace Framework::Graphics {
    struct Vertex;

    class Environment {
    public:
        enum class WinEvents{
            Close, Move, Resize, LeftClick, RightClick, Focus, Scroll
        };
    private:
        Environment(Environment&) = delete;
    protected:
        inline static std::vector<std::function<void(double x, double y)>> g_scrollEvents = std::vector<std::function<void(double x, double y)>>();
        Types::WindowFormat* m_winFormat = nullptr;
        glm::vec2 m_screenSize = glm::vec2(0, 0);
        static inline std::mutex g_mutex = std::mutex();
    protected:
        Environment() = default;
        ~Environment() = default;
    private:
        inline static Environment* g_environment = nullptr;
    public:
        inline static std::function<void(WinEvents, void* win, void* arg1, void* arg2)> g_callback = std::function<void(WinEvents, void* win, void* arg1, void* arg2)>();
    public:
        inline static void RegisterScrollEvent(std::function<void(double, double)> fun){
            g_mutex.lock();
            g_scrollEvents.push_back(fun);
            g_mutex.unlock();
        }

        static bool Set(Environment* env) {
            if (g_environment != nullptr)
                return false;
            else {
                g_environment = env;
                return true;
            }
        }
        static Environment* Get() noexcept {
            if (g_environment == nullptr){
                Framework::Helper::Debug::Error("Environment::Get() : environment is not set!");
                return nullptr;
            }
            else
                return g_environment;
        }
    public:
        virtual bool PreInitGUI(const std::string& fontPath) = 0;
        virtual bool InitGUI() = 0;
        virtual bool StopGUI() = 0;
        virtual void BeginDrawGUI() = 0;
        virtual void EndDrawGUI() = 0;

        virtual inline std::string GetPipeLineName() const noexcept = 0;

        virtual unsigned int CreateTexture(unsigned char* pixels, int w, int h, int components) = 0;

        // ============================= [ WINDOW METHODS ] =============================

        /* create window instance */
        virtual bool MakeWindow(const char* winName, Types::WindowFormat* format, bool fullScreen) = 0;

        inline void SetWinCallBack(std::function<void(WinEvents, void* win, void* arg1, void* arg2)> callback) { g_callback = callback; }

        [[nodiscard]] virtual inline bool IsFullScreen() const noexcept = 0;
        virtual inline void SetFullScreen(bool value) = 0;
        virtual void SetDepthTestEnabled(bool value) = 0;

        virtual bool PreInit(unsigned int smooth_samples) = 0;

        /* set current opengl/vulkan/directx context */
        virtual bool SetContextCurrent() = 0;

        virtual bool Init(int swapInterval) = 0;
        virtual bool PostInit() = 0;

        virtual bool IsWindowOpen() = 0;
        virtual bool CloseWindow() = 0;

        /* clear depth/stencil/color buffers */
        virtual inline void ClearBuffers() noexcept = 0;

        virtual inline void ClearColorBuffers(float r, float g, float b, float a) noexcept = 0;

        /* Swap window color buffers */
        virtual inline void SwapBuffers() noexcept = 0;

        inline glm::vec2 GetScreenSize() noexcept { return m_screenSize; }

        virtual glm::vec2 GetMousePos() = 0;
        virtual glm::vec4 GetTexturePixel(glm::vec2 uPos, unsigned int ID, glm::vec2 size) = 0;
        virtual glm::vec3 GetPixelColor(glm::vec2 uPos) = 0;

        virtual glm::vec2 GetWindowSize() noexcept = 0;
        virtual glm::vec2 GetWindowPosition() noexcept = 0;

        virtual void SetWindowSize(float ratio, unsigned int w, unsigned int h) = 0;
        virtual void SetWindowPosition(int x, int y) = 0;

        virtual void PoolEvents() = 0;

        virtual inline std::string GetVendor() = 0;
        virtual inline std::string GetRenderer() = 0;
        virtual inline std::string GetVersion() = 0;

        virtual inline void SetCursorPosition(glm::vec2 pos) noexcept = 0;

        virtual bool CreateSingleHDRFrameBO(glm::vec2 size, unsigned int& rboDepth, unsigned int& hdrFBO, unsigned int& colorBuffer) = 0;
        virtual bool CreateHDRFrameBufferObject(glm::vec2 size, unsigned int& rboDepth, unsigned int& hdrFBO, std::vector<unsigned int>& colorBuffers) = 0;
        virtual bool CreatePingPongFrameBufferObject(glm::vec2 size,std::vector<unsigned int> & pingpongFBO, std::vector<unsigned int>& pingpongColorBuffers) = 0;
        virtual inline void BindFrameBuffer(unsigned int FBO) const noexcept = 0;
        virtual inline void DeleteBuffer(unsigned int& FBO) = 0;

        // ============================= [ SHADER METHODS ] =============================

        virtual bool CompileShader(std::string path, unsigned int* fragment, unsigned int* vertex) = 0;
        virtual unsigned int LinkShader(unsigned int* fragment, unsigned int* vertex) = 0;
        virtual inline void DeleteShader(unsigned int ID) = 0;
        virtual inline void UseShader(const unsigned int&  ID) noexcept = 0;

        virtual inline void SetBool(const unsigned int&  ID, const char* name, bool v)       const noexcept = 0;
        virtual inline void SetFloat(const unsigned int&  ID, const char* name, float v)     const noexcept = 0;
        virtual inline void SetInt(const unsigned int&  ID, const char* name, int v)         const noexcept = 0;
        virtual inline void SetMat4(const unsigned int&  ID, const char* name, glm::mat4 v)  const noexcept = 0;
        virtual inline void SetVec4(const unsigned int&  ID, const char* name, glm::vec4 v)  const noexcept = 0;
        virtual inline void SetVec3(const unsigned int&  ID, const char* name, glm::vec3 v)  const noexcept = 0;
        virtual inline void SetVec2(const unsigned int&  ID, const char* name, glm::vec2 v)  const noexcept = 0;
        virtual inline void SetIVec2(const unsigned int&  ID, const char* name, glm::ivec2 v)  const noexcept = 0;

        // ============================== [ MESH METHODS ] ==============================

        virtual inline bool CalculateEmptyVAO(unsigned int& VAO) noexcept = 0;
        virtual inline bool CalculateMesh(unsigned int& VBO, unsigned int& VAO, std::vector<Vertex>& vertices, size_t count_verts) noexcept = 0;

        /** Vertex pos and texture cords */
        virtual inline bool CalculateQuad(unsigned int& VBO, unsigned int& VAO) noexcept = 0;
        virtual unsigned int CalculateSkybox() noexcept = 0;
        virtual void DrawSkybox(const unsigned int&  VAO, unsigned int CubeMap) const noexcept = 0;
        virtual inline void DrawQuad(const unsigned int&  VAO) const noexcept = 0;

        virtual inline void BindVAO(const unsigned int&  VAO) const noexcept = 0;
        virtual inline void Draw6Triangles() const noexcept = 0;

        virtual bool FreeMesh(unsigned int VAO) noexcept = 0;
        virtual inline void DrawTriangles(const unsigned int&  VAO, const unsigned int& count_vertices) const noexcept = 0;
        virtual inline void DrawInstancedVertices(unsigned int VAO, unsigned int IBO, unsigned int count) noexcept = 0;

        // ============================== [ TEXTURE METHODS ] ==============================

        virtual inline void BindTexture(const unsigned int&  ID) const noexcept = 0;
        virtual inline void BindTexture(unsigned char activeTexture, const unsigned int&  ID) const noexcept = 0;
        virtual inline void SetActiveTexture(unsigned char activeTexture) const noexcept = 0;
        virtual unsigned int CalculateTexture(unsigned char* data, int format, unsigned int w, unsigned int h, TextureFilter filter, bool alpha) = 0;
        virtual unsigned int CalculateCubeMap(unsigned int w, unsigned int h, std::vector<unsigned char*> data) = 0;
        virtual inline void DeleteTexture(unsigned int ID) noexcept = 0;
        virtual inline void FreeCubeMap(unsigned int ID) noexcept = 0;
    };
}

#endif //GAMEENGINE_ENVIRONMENT_H
