#ifndef _E_APPLICATION_HPP__
#define _E_APPLICATION_HPP__

#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <unordered_map>

#include "Core/Window.hpp"
#include "Ease.hpp"
#include "ECS/Scene/Scene.hpp"
#include "Resource/NativeModule/NativeModule.hpp"
#include <filesystem>

namespace nmGfx { class Renderer; }

namespace Ease
{
    class Application
    {
    public:
        static Application& get_singleton()
        {
            static Application app;
            return app;
        }

        void Run(int argc, char const *argv[]);

        Window& GetWindow() { return _window; }

        void Log(const std::string& message); // Logs given string to console (And Editor Console on editor builds)
        

        enum class ModuleLoadResult
        {
            SUCCESS = 0,
            UNDEFINED_MODULE,
            OUTDATED_VERSION,
        };
        /**
         * @brief   loads author.moduleName.extension
         * 
         * @param author      author name
         * @param moduleName  module name without extension
         * @param minimumVersion minimum version allowed for module
         */
        ModuleLoadResult LoadModule(const std::filesystem::path& modulePath, int minimumVersion);

        /**
         * @brief Adds given module to application context
         * 
         * @param name alias of module
         * @param _module module resource to add
         */
        void AddModule(const std::string& name, std::shared_ptr<NativeModule> _module);
        std::shared_ptr<NativeModule> GetModule(const std::string& name);

        void AddNativeBehaviour(const std::string& name, NativeBehaviourFactory* behaviour);
        Ease::NativeBehaviourFactory* GetFactory(const std::string& name);


        void StartGame();
        void UpdateGame();
        void StopGame();
        bool IsRunning() { return m_AppRunning; }
        Ease::Scene* GetCurrentScene() { return m_pCurrentScene; }
        Ease::Entity GetPickedEntity() { return m_MousePickedEntity; }
        
        void ChangeScene(const char* path);

        void InitModules();
        void UpdateModules();
        void Modules_OnImGuiRender();

        uint32_t Renderer_GetAlbedoFramebufferID();
    private:
        friend class Window;
        friend class Renderer;

        Application();
        ~Application();

        std::unique_ptr<nmGfx::Renderer> _renderer;
        Window _window;

        Scene m_GameScene;
        Scene m_CopyScene;
        Scene* m_pCurrentScene;

        std::unordered_map<std::string, std::shared_ptr<NativeModule>> m_Modules;
        std::unordered_map<std::string, NativeBehaviourFactory*> m_NativeBehaviours;

        bool m_AppRunning = false;

        Ease::Entity m_MousePickedEntity;
    };
} // namespace Ease

#endif