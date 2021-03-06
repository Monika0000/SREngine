//
// Created by Nikita on 18.11.2020.
//

#ifndef GAMEENGINE_WINDOW_H
#define GAMEENGINE_WINDOW_H

#include <Types/WindowFormat.h>

#include <Render/Render.h>
#include <Render/Camera.h>
#include <Environment/Environment.h>
#include <Render/PostProcessing.h>
#include <thread>
#include <mutex>
#include <functional>
#include <Types/EditorGrid.h>
#include <Render/ColorBuffer.h>
#include <Types/Time.h>
#include <Math/Vector3.h>

using namespace Framework::Graphics::Types;

namespace Framework::Graphics {
    class Window {
    public:
        Window(
                const char* win_name,
                WindowFormat format,
                Render* render,
                bool vsync,
                bool fullScreen,
                unsigned int smoothSamples
                )
                : m_env(Environment::Get())
        {
            this->m_win_name        = win_name;
            this->m_format          = format;
            this->m_render          = render;
            this->m_fullScreen      = fullScreen;
            this->m_vsync           = vsync;
            this->m_smoothSamples   = smoothSamples;
        }
    private:
        ~Window() = default;
    private:
        volatile bool                                       m_isCreate              = false;
        volatile bool                                       m_isInit                = false;
        volatile bool                                       m_isRun                 = false;
        volatile bool                                       m_isClose               = false;

        volatile bool                                       m_hasErrors             = false;
        volatile bool                                       m_isEnvInit             = false;
        volatile bool                                       m_isWindowClose         = false;

        volatile bool                                       m_isWindowFocus         = true;

        bool                                                m_isNeedResize          = false;
        bool                                                m_isNeedMove            = false;
    private:
        std::thread                                         m_thread                = std::thread();

        Helper::Types::Time*                                m_time                  = nullptr;

        Environment*                                        m_env                   = nullptr;

        const char*                                         m_win_name              = "Unnamed";
        WindowFormat                                        m_format                = WindowFormat::Unknown;
        unsigned int                                        m_smoothSamples         = 4;

        Render*                                             m_render                = nullptr;

        std::mutex                                          m_camerasMutex          = std::mutex();
        std::vector<Camera*>                                m_newCameras            = std::vector<Camera*>();
        size_t                                              m_countNewCameras       = 0;
        std::vector<Camera*>                                m_camerasToDestroy      = std::vector<Camera*>();
        size_t                                              m_countCamerasToDestroy = 0;
        std::vector<Camera*>                                m_cameras               = std::vector<Camera*>();
        size_t                                              m_countCameras          = 0;

        //std::map<std::string, std::function<void(void)>>    m_contextFuncs          = std::map<std::string, std::function<void(void)>>();
        //size_t                                              m_countContextFuncs     = 0;
        //std::mutex                                          m_contexFuncsMutex      = std::mutex();

        bool                                                m_GUIEnabled            = true;

        ImGuiWindow*                                        m_aimedWindowTarget     = nullptr;
        Camera*                                             m_aimedCameraTarget     = nullptr;
        Mesh*                                               m_aimedMesh             = nullptr;
        bool                                                m_requireGetAimed       = false;
        /*
          0 - nothing
          1 - in process
          2 - complete
         */

        GUI::ICanvas*                                       m_canvas                = nullptr;

        bool                                                m_vsync                 = false;
        bool                                                m_fullScreen            = false;

        glm::vec2                                           m_windowPos             = { 0, 0 };
        glm::vec2                                           m_newWindowPos          = { 0, 0 };
    private:
        void PoolEvents();
        void Thread();
        bool InitEnvironment();
        void Draw();

        void FindAimedMesh();
    public:
       // [[nodiscard]] inline ColorBuffer* GetColorBuffer() const noexcept { return this->m_colorBuffer; }

        inline void AddCamera(Camera* camera) {
            Debug::Log("Window::AddCamera() : register new camera...");
            m_camerasMutex.lock();
            //camera->SetUse(true);
            m_newCameras.push_back(camera);
            m_countNewCameras++;
            m_camerasMutex.unlock();
        } //TODO: mutex
        inline void DestroyCamera(Camera* camera){
            Debug::Log("Window::RemoveCamera() : register camera to remove...");
            if (!camera)
            {
                Debug::Error("Window::RemoveCamera() : camera is nullptr! The application will now crash...");
                return;
            }
            m_camerasMutex.lock();
            m_camerasToDestroy.push_back(camera);
            m_countCamerasToDestroy++;
            m_camerasMutex.unlock();
        }

        inline Render* GetRender() {
            if (!m_render) {
                Debug::Error("Window::GetRender() : render is nullptr! Engine may be crash...");
                return nullptr;
            }
            return m_render;
        }
        [[nodiscard]] inline bool IsRun() const noexcept { return m_isRun; }

        [[nodiscard]] Mesh* PopAimedMesh() noexcept {
            if (m_aimedMesh) {
                Mesh* aim = m_aimedMesh;
                m_aimedMesh = nullptr;
                return aim;
            } else
                return nullptr;
        }
        bool RequireAimedMesh(Camera* camera, ImGuiWindow* window) noexcept {
            if (this->m_requireGetAimed)
                return false;

            this->m_requireGetAimed = true;

            this->m_aimedCameraTarget = camera;
            this->m_aimedWindowTarget = window;
            this->m_aimedMesh = nullptr;

            return true;
        }
        /*[[nodiscard]] Mesh* GetLastAimed() const noexcept { return m_aimedMesh; }
        [[nodiscard]] Mesh* AwaitGetAimed(Camera* camera,  ImGuiWindow* window) noexcept {
            ret:
            if (this->m_requireGetAimedStat != 0)
                goto ret;

            this->m_requireGetAimedStat = 1;

            this->m_aimedCameraTarget = camera;
            this->m_aimedWindowTarget = window;
            this->m_aimedMesh = nullptr;

            wait:
            if (this->m_requireGetAimedStat != 2)
                goto wait;

            Mesh* aim = m_aimedMesh;

            this->m_aimedCameraTarget = nullptr;
            this->m_aimedWindowTarget = nullptr;
            this->m_requireGetAimedStat = 0;

            return aim;
        }*/
    public:
        void CentralizeWindow();
        void Resize(unsigned int w, unsigned int h);
        void CentralizeCursor() noexcept;
        inline void SetGUIEnabled(bool value) noexcept {
            this->m_GUIEnabled = value;
        }
        bool SetCanvas(GUI::ICanvas* canvas);

        //bool AddFunctionAtContext(const std::string& funName, std::function<void(void)> fun);
        //bool RemoveFunctionFromContext(const std::string& funName);
    public:
        [[nodiscard]] inline bool IsFullScreen() const noexcept { return this->m_env->IsFullScreen(); }
        inline void SetFullScreen(bool value) const noexcept { this->m_env->SetFullScreen(value); }
        [[nodiscard]] inline bool IsWindowOpen()  const noexcept { return !this->m_isWindowClose; }
        [[nodiscard]] inline bool IsWindowFocus() const noexcept { return this->m_isWindowFocus;  }
        [[nodiscard]] inline Math::Vector2 GetWindowSize() const noexcept {
            return Math::Vector2(
                    m_format.Width(),
                    m_format.Height()
                );
        }
        glm::vec2 GetGlobalWindowMousePos(Camera* camera, ImGuiWindow* win);
    public:
        bool Create();
        bool Init();
        bool Run();
        bool Close();
        bool Free();
    };
}

#endif //GAMEENGINE_WINDOW_H
