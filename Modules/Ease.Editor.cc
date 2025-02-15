/**
 * @file Ease.Editor.cc
 * @author Lexographics
 * @brief Core Editor Module for Ease Engine
 * @version 0.1
 * @date 2022-05-19
 * 
 * @userFuncs
 *    Print  >  prints given string to editor console
 * @userValues
 *    PrintMsg > string that will be printed to console @see UserFunc:Print
 */
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "Ease.hpp"
#include "dylib.hpp"
#include "imgui-docking/imgui.h"
#include "imgui-docking/misc/cpp/imgui_stdlib.h"
#include "Core/Application.hpp"
#include "Core/ProjectSettings.hpp"
#include "rlImGui/rlImGui.h"
#include <memory>
#include <unordered_map>
#include "Resource/ResourceManager.hpp"
#include "Resource/Texture/Texture.hpp"
#include "Resource/EditorTheme/EditorTheme.hpp"
#include "ECS/Components/Components.hpp"
#include "Core/Input.hpp"

class EaseEditor;
static EaseEditor* g_Editor = nullptr;

static void Print();


class EaseEditor : public Ease::BaseModule
{
   public:
      std::string m_EditorStatusText{"Sowa Engine v" EASE_VERSION_STRING};
      std::string m_ConsoleText{""};

      Ease::Entity m_SelectedEntity{};
      enum class InspectorMode // What data does inspector displays. (entity, resource, texture etc.)
      {
         NONE = 0,
         ENTITY = 1, // Components
         TEXTURE = 2,
      };
      InspectorMode m_InspectorMode;

      struct Panel
      {
         bool visible = false;
         std::string title;
         std::function<void()> drawFunc;

         Panel(const char* _title, std::function<void()> _drawFunc)
            : title(_title), drawFunc(_drawFunc)
         {
            
         }
      };
      std::vector<Panel> panels;

      // Temporary!
      void SetTheme(const std::string& theme)
      {
         // Todo: make 'Theme' class that can be applied to editor directly ( class will be on engine so other modules can use it )
         static std::unordered_map<std::string, int> builtinThemes = {
            {"Dark", 1},
            {"Light", 2},
            {"Classic", 3}
         };
         if(builtinThemes[theme] == builtinThemes["Dark"])
         {
            ImGui::StyleColorsDark();
         }
         else if(builtinThemes[theme] == builtinThemes["Light"])
         {
            ImGui::StyleColorsLight();
         }
         else if(builtinThemes[theme] == builtinThemes["Classic"])
         {
            ImGui::StyleColorsClassic();
         }
      }

   public:
      ~EaseEditor()
      {
         std::cout << "Editor Destroyed" << std::endl;
      }

      struct {
         static void empty()
         {
            ImGui::Text("Empty");
         }
         static void scene()
         {
            if(ImGui::Button(">", ImVec2(32.f, 32.f)))
            {
               Ease::Application::get_singleton().StartGame();
            }
            ImGui::SameLine();
            if(ImGui::Button("||", ImVec2(32.f, 32.f)))
            {
               Ease::Application::get_singleton().StopGame();
            }
            ImGui::SameLine();
            static float editor_2dview_sensitivity = 1.f;
            ImGui::SliderFloat("Sensitivity", &editor_2dview_sensitivity, 0.2f, 2.f, "%.1fx");
            
            
            Ease::Window& window = Ease::Application::get_singleton().GetWindow();
            size_t imageID = Ease::Application::get_singleton().Renderer_GetAlbedoFramebufferID();
               
            int dstWidth = ImGui::GetContentRegionAvail().x;
            int dstHeight = ImGui::GetContentRegionAvail().y;
            float aspect = (float)window.GetWindowWidth() / (float)window.GetWindowHeight();
            if(ImGui::GetContentRegionAvail().x * aspect > ImGui::GetContentRegionAvail().y 
              && ImGui::GetContentRegionAvail().y * aspect < ImGui::GetContentRegionAvail().x)
            {
               dstWidth = ImGui::GetContentRegionAvail().y * aspect;
               dstHeight = ImGui::GetContentRegionAvail().y;

               float regionAvail = ImGui::GetContentRegionAvail().x;
               ImGui::SetCursorPosX(((regionAvail - dstWidth) / 2) + ImGui::GetCursorPosX());
            }
            else
            {
               dstWidth = ImGui::GetContentRegionAvail().x;
               dstHeight = ImGui::GetContentRegionAvail().x / aspect;
               
               float regionAvail = ImGui::GetContentRegionAvail().y;
               ImGui::SetCursorPosY(((regionAvail - dstHeight) / 2.f) + ImGui::GetCursorPosY());
            }
            #ifdef EASE_EDITOR
            window.Editor_SetBlackbarWidth(((ImGui::GetContentRegionAvail().x - dstWidth) / 2));
            window.Editor_SetBlackbarHeight(((ImGui::GetContentRegionAvail().y - dstHeight) / 2.f));
            window.Editor_SetWindowWidth(dstWidth);
            window.Editor_SetWindowHeight(dstHeight);
            ImVec2 windowPos = ImGui::GetWindowPos();
            window.Editor_SetWindowPos(windowPos.x + ImGui::GetCursorPosX(), windowPos.y + ImGui::GetCursorPosY());
            #endif

            Ease::Application& app = Ease::Application::get_singleton();
            if(!app.IsRunning())
            {
               static bool Editor_DragBegin = false;
               ImGuiIO& io = ImGui::GetIO();
               Ease::Component::Transform2D& cam2dTransform = app.GetCurrentScene()->CurrentCameraTransform2D();
               Ease::Component::Camera2D& cam2DCamera = app.GetCurrentScene()->CurrentCamera2D();

               if(ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
                  Editor_DragBegin = true;
               if(!ImGui::IsMouseDown(ImGuiMouseButton_Right))
                  Editor_DragBegin = false;
               
               if(Editor_DragBegin)
               {
                  cam2dTransform.Position().x -= io.MouseDelta.x * cam2dTransform.Scale().x * editor_2dview_sensitivity;
                  cam2dTransform.Position().y += io.MouseDelta.y * cam2dTransform.Scale().x * editor_2dview_sensitivity;
               }

               if(ImGui::IsWindowHovered())
               {
                  cam2dTransform.Scale().x -= io.MouseWheel * 0.1f * (cam2dTransform.Scale().x);
                  cam2dTransform.Scale().x = MAX(0.1f, cam2dTransform.Scale().x);

                  cam2dTransform.Scale().y = cam2dTransform.Scale().x;
               }


               //if(ImGui::IsWindowHovered() && Ease::Input::IsMouseButtonClicked(Ease::Input::Button::LEFT))
               //{
               //   Ease::Entity pickedEntity = app.GetPickedEntity();
               //   if(pickedEntity.IsValid())
               //   {
               //      g_Editor->m_SelectedEntity = pickedEntity;
               //      g_Editor->m_InspectorMode = InspectorMode::ENTITY;
               //   }
               //   else
               //   {
               //      g_Editor->m_SelectedEntity.SetEntityID(entt::null);
               //      g_Editor->m_SelectedEntity.SetRegistry(nullptr);
               //      g_Editor->m_InspectorMode = InspectorMode::NONE;
               //   }
               //}
            }
            
            ImGui::Image((void*)imageID, ImVec2(dstWidth, dstHeight));
         }
         static void hierarchy()
         {
            Ease::Scene* pScene = Ease::Application::get_singleton().GetCurrentScene();
            auto view = pScene->m_Registry.view<Ease::Component::Common>();
            bool entity_rclicked = false; // is user right clicked on any entity this frame
            for(auto it = view.rbegin(); it < view.rend(); ++it)
            {
               Ease::Entity e(*it, &pScene->m_Registry);
               std::string name = e.GetComponent<Ease::Component::Common>().Name();
               
               if(name.length() > 0 && ImGui::MenuItem(name.c_str()))
               {
                  g_Editor->m_SelectedEntity.SetEntityID(*it);
                  g_Editor->m_SelectedEntity.SetRegistry(&pScene->m_Registry);
                  g_Editor->m_InspectorMode = InspectorMode::ENTITY;

               }
               if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
               {
                  ImGui::OpenPopup("__POPUP__HIERARCHY_ENTITY_RCLICK");
                  entity_rclicked = true;
                  g_Editor->m_SelectedEntity.SetEntityID(*it);
                  g_Editor->m_SelectedEntity.SetRegistry(&pScene->m_Registry);
               }
            }
            if(ImGui::BeginPopup("__POPUP__HIERARCHY_ENTITY_RCLICK"))
            {
               if(ImGui::MenuItem("Delete"))
               {
                  Ease::Application::get_singleton().GetCurrentScene()->Destroy(g_Editor->m_SelectedEntity);
               }
               if(ImGui::MenuItem("Copy"))
               {
                  Ease::Application::get_singleton().GetCurrentScene()->CopyEntity(g_Editor->m_SelectedEntity);
               }
               ImGui::EndPopup();
            }

            if(!entity_rclicked && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
               ImGui::OpenPopup("__POPUP__HIERARCHY_RCLICK");
            }
            if(ImGui::BeginPopup("__POPUP__HIERARCHY_RCLICK"))
            {
               if(ImGui::BeginMenu("New"))
               {
                  if(ImGui::MenuItem("Empty Entity"))
                  {
                     Ease::Entity e = Ease::Application::get_singleton().GetCurrentScene()->Create("Empty Entity");
                     g_Editor->m_SelectedEntity = e;
                  }
                  if(ImGui::MenuItem("Sprite2D"))
                  {
                     Ease::Entity e = Ease::Application::get_singleton().GetCurrentScene()->Create("Sprite2D");
                     e.AddComponent<Ease::Component::Transform2D>();
                     e.AddComponent<Ease::Component::SpriteRenderer2D>();
                     g_Editor->m_SelectedEntity = e;
                  }
                  if(ImGui::MenuItem("Button"))
                  {
                     Ease::Entity e = Ease::Application::get_singleton().GetCurrentScene()->Create("Button");
                     e.AddComponent<Ease::Component::UITransform>();
                     e.AddComponent<Ease::Component::Button>();
                     g_Editor->m_SelectedEntity = e;
                  }
                  if(ImGui::MenuItem("Camera2D"))
                  {
                     Ease::Entity e = Ease::Application::get_singleton().GetCurrentScene()->Create("Camera2D");
                     e.AddComponent<Ease::Component::Transform2D>();
                     e.AddComponent<Ease::Component::Camera2D>();
                     g_Editor->m_SelectedEntity = e;
                  }
                  if(ImGui::BeginMenu("Physics"))
                  {
                     if(ImGui::MenuItem("StaticBody2D"))
                     {
                        Ease::Entity e = Ease::Application::get_singleton().GetCurrentScene()->Create("StaticBody2D");
                        e.AddComponent<Ease::Component::PhysicsBody2D>().BodyType() = Ease::PhysicsBodyType::STATIC;
                        e.AddComponent<Ease::Component::Transform2D>();
                        g_Editor->m_SelectedEntity = e;
                     }
                     if(ImGui::MenuItem("DynamicBody2D"))
                     {
                        Ease::Entity e = Ease::Application::get_singleton().GetCurrentScene()->Create("DynamicBody2D");
                        e.AddComponent<Ease::Component::PhysicsBody2D>().BodyType() = Ease::PhysicsBodyType::DYNAMIC;
                        e.AddComponent<Ease::Component::Transform2D>();
                        g_Editor->m_SelectedEntity = e;
                     }
                     ImGui::EndMenu();
                  }
                  if(ImGui::MenuItem("Text2D"))
                  {
                     Ease::Entity e = Ease::Application::get_singleton().GetCurrentScene()->Create("Text2D");
                     e.AddComponent<Ease::Component::Transform2D>();
                     e.AddComponent<Ease::Component::TextRenderer2D>("Text");
                     g_Editor->m_SelectedEntity = e;
                  }
                  ImGui::EndMenu();
               }

               if(Ease::Application::get_singleton().GetCurrentScene()->GetCopiedEntityCount() > 0 && ImGui::MenuItem("Paste"))
               {
                  Ease::Application::get_singleton().GetCurrentScene()->PasteCopiedEntity();
               }
               ImGui::EndPopup();
            }
         }
         template <typename T, typename Func>
         static void DrawComponent(const char* compName, Func func, Ease::Entity& entity)
         {
            if(!entity.HasComponent<T>())
               return;
            auto& component = entity.GetComponent<T>();

            ImGui::Separator();
            ImGui::PushID(compName);
            if(ImGui::CollapsingHeader(compName))
            {
               ImGui::Indent();
               func(component);
               if(ImGui::Button((std::string("Remove ") + compName).c_str()))
               {
                  entity.RemoveComponent<T>();
               }
               ImGui::Unindent();
            }
            ImGui::PopID();
         }
         static void inspector()
         {
            if(g_Editor->m_InspectorMode == InspectorMode::ENTITY)
            {
               if(g_Editor->m_SelectedEntity.IsValid())
               {
                  Ease::Entity entity = g_Editor->m_SelectedEntity;
                  std::string& name = entity.GetComponent<Ease::Component::Common>().Name();

                  char buf[8];
                  
                  ImGui::Text("Name"); ImGui::SameLine();
                  ImGui::InputText("##Name", &name);
                  if(name.length() == 0)
                     name = "Entity";

                  DrawComponent<Ease::Component::AnimatedSprite2D>("AnimatedSprite2D", [](Ease::Component::AnimatedSprite2D& component){
                     ImGui::Text("AnimatedSprite2D");
                  }, entity);
                  DrawComponent<Ease::Component::AudioStreamPlayer>("AudioStreamPlayer", [](Ease::Component::AudioStreamPlayer& component){
                     static bool playing = false;
                     if(ImGui::Checkbox("Playing", &playing))
                     {
                        if(playing)
                           component.Play();
                        else
                           component.Stop();
                     }
                     
                     if(ImGui::Checkbox("Loop", &component.Looping()))
                        component.UpdateData();
                  }, entity);
                  DrawComponent<Ease::Component::Button>("Button", [](Ease::Component::Button& component){
                     ImGui::InputText("Text", &component.Text());
                     ImGui::Checkbox("Disabled", &component.Disabled());
                  }, entity);
                  DrawComponent<Ease::Component::Camera2D>("Camera2D", [](Ease::Component::Camera2D& component){
                     ImGui::Checkbox("Current", &component.Current());
                     ImGui::DragFloat("Zoom", &component.Zoom(), 0.05f, 0.1f, 10.f);
                  }, entity);
                  DrawComponent<Ease::Component::Group>("Group", [](Ease::Component::Group& component){
                     ImGui::Text("Groups");
                     for(size_t i = 0; i < component.Groups().size(); i++)
                     {
                        ImGui::InputText((std::string("##GROUP_INPUT") + std::to_string(i)).c_str(), &component.Groups()[i]);
                     }
                     if(ImGui::Button("+", ImVec2(32.f, 32.f)))
                        component.Groups().emplace_back("Group");
                     ImGui::SameLine();
                     if(ImGui::Button("-", ImVec2(32.f, 32.f)) && component.Groups().size() > 0)
                        component.Groups().pop_back();
                  }, entity);
                  DrawComponent<Ease::Component::NativeBehaviourClass>("NativeBehaviourClass", [](Ease::Component::NativeBehaviourClass& component){
                     ImGui::InputText("Class Name", &component.ClassName());
                  }, entity);
                  DrawComponent<Ease::Component::PhysicsBody2D>("PhysicsBody2D", [](Ease::Component::PhysicsBody2D& component){
                     int bodyType = (int)component.BodyType();
                     if((int)Ease::PhysicsBodyType::STATIC != 0
                     || (int)Ease::PhysicsBodyType::DYNAMIC != 1
                     || (int)Ease::PhysicsBodyType::KINEMATIC != 2)
                        std::cout << "ERROR: Ease::PhysicsBodyType layout mismatch" << std::endl;

                     ImGui::Combo("Shape", &bodyType, "Static\0Dynamic\0Kinematic\0\0");
                     component.BodyType() = (Ease::PhysicsBodyType)bodyType;

                     if(ImGui::CollapsingHeader("Colliders"))
                     {
                        ImGui::Indent();

                        if(ImGui::Button("+", ImVec2(32.f, 32.f)))
                           component.Colliders().emplace_back();

                        int i=0;
                        for(Ease::Collider2D& collider : component.Colliders())
                        {
                           i++;
                           if(ImGui::CollapsingHeader(std::to_string(i).c_str()))
                           {
                              ImGui::Indent();
                              ImGui::PushID(i);

                              int shape = (int)collider.shape;
                              if((int)Ease::ColliderShape2D::BOX != 0
                              || (int)Ease::ColliderShape2D::CIRCLE != 1)
                                 std::cout << "ERROR: Ease::ColliderShape2D layout mismatch" << std::endl;

                              ImGui::Combo("Shape", &shape, "Box\0Circle\0\0");
                              collider.shape = (Ease::ColliderShape2D)shape;
                              
                              ImGui::DragFloat2("Offset", &collider.offset.x);
                              
                              float rad = DEG2RAD * collider.rotation;
                              ImGui::SliderAngle("Rotation", &rad);
                              collider.rotation = RAD2DEG * rad;

                              if(collider.shape == Ease::ColliderShape2D::BOX)
                              {
                                 float size[2];
                                 size[0] = collider.width;
                                 size[1] = collider.height;
                                 ImGui::DragFloat2("Size", size);
                                 collider.width = size[0];
                                 collider.height = size[1];
                              }
                              else if(collider.shape == Ease::ColliderShape2D::CIRCLE)
                              {
                                 ImGui::DragInt("Radius", &collider.radius);
                              }

                              ImGui::DragFloat("Density", &collider.density, 0.05f, 0.f, 10.f);
                              ImGui::DragFloat("Friction", &collider.friction, 0.05f, 0.f, 10.f);
                              ImGui::DragFloat("Restitution", &collider.restitution, 0.05f, 0.f, 10.f);
                              ImGui::DragFloat("RestitutionThreshold", &collider.restitutionThreshold, 0.05f, 0.f, 10.f);

                              ImGui::PopID();
                              ImGui::Unindent();
                           }
                        }

                        ImGui::Unindent();
                     }
                  }, entity);
                  DrawComponent<Ease::Component::SpriteRenderer2D>("SpriteRenderer2D", [](Ease::Component::SpriteRenderer2D& component){
                     ImGui::Checkbox("Visible", &component.Visible());
                  }, entity);
                  DrawComponent<Ease::Component::TextRenderer2D>("TextRenderer2D", [](Ease::Component::TextRenderer2D& component){
                     ImGui::InputText("Text", &component.Text());
                     ImGui::DragFloat("Font Size", &component.FontSize());
                     
                     float color[4];
                     memcpy(color, &component.Color().r, 4 * sizeof(float));
                     color[0] /= 255.f;
                     color[1] /= 255.f;
                     color[2] /= 255.f;
                     color[3] /= 255.f;
                     ImGui::ColorPicker4("Color", color, ImGuiColorEditFlags_Uint8);
                     color[0] *= 255.f;
                     color[1] *= 255.f;
                     color[2] *= 255.f;
                     color[3] *= 255.f;
                     memcpy(&component.Color().r, color, 4 * sizeof(float));

                     ImGui::Checkbox("Visible", &component.Visible());
                  }, entity);
                  DrawComponent<Ease::Component::Transform2D>("Transform2D", [](Ease::Component::Transform2D& component){
                     ImGui::DragFloat2("Position", &component.Position().x);
                     ImGui::DragFloat2("Scale", &component.Scale().x, 0.005f);
                     float rot = DEG2RAD * component.Rotation();
                     ImGui::SliderAngle("Rotation", &rot);
                     component.Rotation() = rot * RAD2DEG;
                     // ImGui::DragInt("ZIndex", &component.ZIndex());
                  }, entity);
                  DrawComponent<Ease::Component::UITransform>("UITransform", [](Ease::Component::UITransform& component){
                     ImGui::DragFloat2("Position", &component.Position().x);
                     ImGui::DragFloat2("Scale", &component.Scale().x, 0.005f);
                     ImGui::DragFloat2("Size", &component.Size().x);
                     float rot = DEG2RAD * component.Rotation();
                     ImGui::SliderAngle("Rotation", &rot);
                     component.Rotation() = rot * RAD2DEG;
                     // ImGui::DragInt("ZIndex", &component.ZIndex());
                  }, entity);

                  if(ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                  {
                     ImGui::OpenPopup("__POPUP__INSPECTOR_RCLICK");
                  }
                  if(ImGui::BeginPopup("__POPUP__INSPECTOR_RCLICK"))
                  {
                     if(ImGui::BeginMenu("Add Component"))
                     {
#define POPUP_ADDCOMPONENT(t) {if(!entity.HasComponent<Ease::Component::t>()) if(ImGui::MenuItem(#t)) entity.AddComponent<Ease::Component::t>();}
                        POPUP_ADDCOMPONENT(AnimatedSprite2D);
                        POPUP_ADDCOMPONENT(AudioStreamPlayer);
                        POPUP_ADDCOMPONENT(Button);
                        POPUP_ADDCOMPONENT(Camera2D);
                        POPUP_ADDCOMPONENT(Group);
                        POPUP_ADDCOMPONENT(NativeBehaviourClass);
                        POPUP_ADDCOMPONENT(PhysicsBody2D);
                        POPUP_ADDCOMPONENT(SpriteRenderer2D);
                        POPUP_ADDCOMPONENT(TextRenderer2D);
                        POPUP_ADDCOMPONENT(Transform2D);
#undef POPUP_ADDCOMPONENT
                        
                        ImGui::EndMenu();
                     }
                     ImGui::EndPopup();
                  }
               }
            }
         }
         static void console()
         {
            ImGui::Text("%s", g_Editor->m_ConsoleText.c_str());
            
            if(ImGui::Button("Clear"))
               g_Editor->m_ConsoleText = "";
         }
      } draw;

      void Start() override
      {
         //ImGui::SetCurrentContext(Ease::Application::GetImGuiContext());

         userFuncs["Print"] = Print;

         panels.emplace_back("Hierarchy", draw.hierarchy);
         panels.emplace_back("Scene"    , draw.scene);
         panels.emplace_back("Inspector", draw.inspector);
         // panels.emplace_back("Project"  , draw.empty);
         panels.emplace_back("Console"  , draw.console);


         Ease::ResourceLoader& themeLoader = Ease::ResourceLoader::get_singleton();
         // Ease::ResourceManager<Ease::EditorTheme> themeLoader = Ease::Application::get_singleton().GetCurrentScene()->GetResourceManager<Ease::EditorTheme>();

         Reference<Ease::EditorTheme> theme = themeLoader.LoadResource<Ease::EditorTheme>("abs://res/theme.escfg");
         ImGui::GetStyle() = theme->GetStyle();

         //Ease::Application::get_singleton().GetCurrentScene()->LoadFromFile("scene.escn");
         // Ease::Application::get_singleton().GetCurrentScene()->SaveToFile("scene.escn");
      }

      void Update() override
      {
         static int i = 0;
         i++;
      }

      void OnImGuiRender() override
      {
         Ease::Window& window = Ease::Application::get_singleton().GetWindow();

         ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
         ImGui::SetNextWindowSize(ImVec2(window.GetWindowWidth(), window.GetWindowHeight()));
         ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
         ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
         ImGui::Begin("Editor", nullptr,
              ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoNavFocus
            | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_MenuBar
            | ImGuiWindowFlags_NoBackground);
         ImGui::PopStyleVar(2);

         static bool modal_about_ease = false;
         static bool modal_editor_settings = false;
         static bool modal_project_settings = false;

         /** <Menu Bar> **/
         if(ImGui::BeginMainMenuBar())
         {
            if(ImGui::BeginMenu("Scene"))
            {
               if(ImGui::MenuItem("Save Scene"))
               {
                  Ease::Application::get_singleton().GetCurrentScene()->Save();
               }
               static std::string scenePath = "res://game.escn";
               ImGui::InputText("Scene Path", &scenePath);
               if(ImGui::MenuItem("Open Scene"))
               {
                  Ease::Application::get_singleton().GetCurrentScene()->LoadFromFile(scenePath.c_str());
               }
               ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("View"))
            {
               ImGui::EndMenu();
            }
            
            if(ImGui::BeginMenu("Project"))
            {
               if(ImGui::MenuItem("Project Settings"))
                  modal_project_settings = true;

               ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Editor"))
            {
               if(ImGui::MenuItem("Editor Settings"))
                  modal_editor_settings = true;

               ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Help"))
            {
               if(ImGui::MenuItem("About Ease"))
                  modal_about_ease = true;

               ImGui::EndMenu();
            }

            if(ImGui::MenuItem("Export theme"))
            {
               std::shared_ptr<Ease::EditorTheme> theme = std::make_shared<Ease::EditorTheme>();
               theme->LoadFromStyle(ImGui::GetStyle());
               // Ease::ResourceManager<Ease::EditorTheme>::GetLoader().SaveResource("res/theme_new.escfg", theme);
               assert(false && "Can not export themes");
            }


            ImGui::EndMainMenuBar();
         }

         /** </Menu Bar> **/
         ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.f, 0.f), ImGuiDockNodeFlags_PassthruCentralNode);

         // ImGui::ShowDemoWindow();
         for(Panel& panel : panels)
         {
            ImGui::Begin(panel.title.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
               panel.drawFunc();
            ImGui::End();
         }

         const ImGuiViewport* viewport = ImGui::GetMainViewport();
         ImGui::SetNextWindowPos(
            ImVec2(viewport->Pos.x,
                  viewport->Pos.y + viewport->Size.y - ImGui::GetFrameHeight())
         );
         ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, ImGui::GetFrameHeight()));
         ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
         ImGui::PushStyleColor(ImGuiCol_MenuBarBg,     ImVec4(0.17f, 0.50f, 0.70f, 1.f)); // (45, 128, 178)
         ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.20f, 0.55f, 0.75f, 1.f)); // (51, 140, 191)
         if(ImGui::Begin("##statusbar", nullptr, 
              ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_MenuBar
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse))
         {
            ImGui::PopStyleVar();

            if(ImGui::BeginMenuBar())
            {
               ImGui::MenuItem("This is a Button");
               ImGui::MenuItem("This is a Button too");
               ImGui::MenuItem("This is a Button too too");

               ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - (ImGui::CalcTextSize(m_EditorStatusText.c_str()).x * 1.2f) );
               ImGui::Text("%s", m_EditorStatusText.c_str());

               ImGui::EndMenuBar();
            }
            ImGui::End();

            ImGui::PopStyleColor(2);
         } else { ImGui::PopStyleVar(); ImGui::PopStyleColor(2); }
         
         ImGui::End();


         if(modal_project_settings)
         {
            Ease::Window& window = Ease::Application::get_singleton().GetWindow();
            ImGui::SetNextWindowSize(ImVec2(
               (window.GetWindowWidth() / 1.75f),
               (window.GetWindowHeight() / 1.5f)
            ));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
            if(ImGui::Begin("Project Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
            {
               static int selected_tab = 0;
               static bool project_changed = false;
               
               const int TAB_GENERAL = 0;
               const int TAB_TEST        = 1;

               Ease::ProjectSettings& projectSettings = Ease::ProjectSettings::get_singleton();

   
               if(ImGui::Button("General"))
                  selected_tab = TAB_GENERAL;
               
               ImGui::SameLine();
               if(ImGui::Button("Test"))
                  selected_tab = TAB_TEST;
               

               if(selected_tab == TAB_GENERAL)
               {
                  if(ImGui::BeginTable("__PROJECT__PROJECTSETTINGS_GENERAL_", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                  {
                     static int general_selected_tab = 0;
                     
                     const int TAB_GENERAL_APPLICATION = 0;
                     const int TAB_GENERAL_WINDOW      = 1;


                     ImGui::TableNextColumn();
                     if(ImGui::MenuItem("Application"))
                        general_selected_tab = TAB_GENERAL_APPLICATION;
                     if(ImGui::MenuItem("Window"))
                        general_selected_tab = TAB_GENERAL_WINDOW;

                     ImGui::TableNextColumn();
                     if(general_selected_tab == TAB_GENERAL_APPLICATION)
                     {
                        ImGui::Text("Name"); ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - ImGui::GetCursorPosX());
                        if(ImGui::InputText("##App_Name", &projectSettings._application.Name))
                           project_changed = true;
                        
                        ImGui::Text("Description"); ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - ImGui::GetCursorPosX());
                        if(ImGui::InputText("##App_Desc", &projectSettings._application.Description))
                           project_changed = true;

                        ImGui::Text("MainScene"); ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - ImGui::GetCursorPosX());
                        if(ImGui::InputText("##App_MainScene", &projectSettings._application.MainScene))
                           project_changed = true; 
                     }
                     if(general_selected_tab == TAB_GENERAL_WINDOW)
                     {
                        ImGui::Text("Window Size"); ImGui::SameLine();
                        int window_size[2] = { projectSettings._window.WindowWidth, projectSettings._window.WindowHeight };
                        if(ImGui::DragInt2("##Win_Window_Size", window_size, 1.f, 1, 0))
                           project_changed = true;
                        projectSettings._window.WindowWidth = window_size[0];
                        projectSettings._window.WindowHeight = window_size[1];

                        ImGui::Text("Video Size"); ImGui::SameLine();
                        int video_size[2] = { projectSettings._window.VideoWidth, projectSettings._window.VideoHeight };
                        if(ImGui::DragInt2("##Win_Video_Size", video_size, 1.f, 1, 0))
                           project_changed = true;
                        projectSettings._window.VideoWidth = video_size[0];
                        projectSettings._window.VideoHeight = video_size[1];

                        ImGui::Text("Fullscreen"); ImGui::SameLine();
                        if(ImGui::Checkbox("##Win_Fullscreen", &projectSettings._window.Fullscreen))
                           project_changed = true;
                     }
                     
                     ImGui::EndTable();
                  }
               }

               if(selected_tab == TAB_TEST)
               {
                  ImGui::Text("Test items");
                  ImGui::Text("Test items");
                  ImGui::Text("Test items");
                  ImGui::Text("Test items");
               }

               ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - (ImGui::CalcTextSize("Close").x * (3.f / 2.f)));
               if (ImGui::Button( "Close", ImVec2(
                     ImGui::CalcTextSize("Close").x * 3,
                     ImGui::CalcTextSize("Close").y * 1.5)))
               {
                  modal_project_settings = false;

                  if(project_changed)
                  {
                     project_changed = false;
                     projectSettings.SaveProject();
                  }
               }
               ImGui::SetWindowPos(ImVec2(
                  (window.GetWindowWidth() / 2) - (ImGui::GetWindowSize().x / 2),
                  (window.GetWindowHeight() / 2) - (ImGui::GetWindowSize().y / 2)
               ));
            }
            ImGui::PopStyleVar();
            ImGui::End();
         }

         if(modal_editor_settings)
         {
            Ease::Window& window = Ease::Application::get_singleton().GetWindow();
            ImGui::SetNextWindowSize(ImVec2(
               (window.GetWindowWidth() / 2.5f),
               (window.GetWindowHeight() / 2.f)
            ));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
            if(ImGui::Begin("Editor Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
            {
               static int selected_tab = 0;
               
               const int TAB_INTERFACE = 0;
               const int TAB_THEME     = 1;
   
               if(ImGui::Button("Interface"))
                  selected_tab = TAB_INTERFACE;
               
               ImGui::SameLine();
               if(ImGui::Button("Theme"))
                  selected_tab = TAB_THEME;
               

               if(selected_tab == TAB_INTERFACE)
               {
                  ImGui::Text("Inteface items");
                  ImGui::Text("Inteface items");
                  ImGui::Text("Inteface items");
                  ImGui::Text("Inteface items");
               }

               if(selected_tab == TAB_THEME)
               {
                  static std::string selected_preset = "Dark";

                  ImGui::Text("Preset"); ImGui::SameLine();
                  
                  if(ImGui::BeginCombo("##Preset", selected_preset.c_str()))
                  {
                     if(ImGui::Selectable("Dark"))
                     {
                        selected_preset = "Dark";
                        SetTheme(selected_preset);
                     }
                     if(ImGui::Selectable("Light"))
                     {
                        selected_preset = "Light";
                        SetTheme(selected_preset);
                     }
                     if(ImGui::Selectable("Classic"))
                     {
                        selected_preset = "Classic";
                        SetTheme(selected_preset);
                     }

                     ImGui::EndCombo();
                  }
                  //if(ImGui::Combo("##Presets", &theme_preset_index, presets_str.c_str()))
                     //std::cout << "Set theme to " << theme_preset_index << std::endl;
               }


               ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - (ImGui::CalcTextSize("Close").x * (3.f / 2.f)));
               if (ImGui::Button( "Close", ImVec2(
                     ImGui::CalcTextSize("Close").x * 3,
                     ImGui::CalcTextSize("Close").y * 1.5)))
               {
                  modal_editor_settings = false;
               }
               ImGui::SetWindowPos(ImVec2(
                  (window.GetWindowWidth() / 2) - (ImGui::GetWindowSize().x / 2),
                  (window.GetWindowHeight() / 2) - (ImGui::GetWindowSize().y / 2)
               ));
            }
            ImGui::PopStyleVar();
            ImGui::End();
         }

         if(modal_about_ease)
         {
            // static std::shared_ptr<Ease::Texture> iconTexture = Ease::ResourceManager<Ease::Texture>::GetLoader().LoadResource("res/icon.png");
         
            Ease::Window& window = Ease::Application::get_singleton().GetWindow();
            ImGui::SetNextWindowSize(ImVec2(
               (window.GetWindowWidth() / 2.5f),
               (window.GetWindowHeight() / 2.f)
            ));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
            if(ImGui::Begin("About Ease", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
            {
               ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - 64);
               // RLImGuiImageSize(&iconTexture->GetTexture(), 128, 128);

               ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - (ImGui::CalcTextSize("Ease Engine").x / 2));
               ImGui::Text("Ease Engine");

               ImGui::NewLine();
               ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - (ImGui::CalcTextSize("Lexographics").x / 2));
               ImGui::Text("Lexographics");


               ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - (ImGui::CalcTextSize("Close").x * (3.f / 2.f)));
               if (ImGui::Button( "Close", ImVec2(
                     ImGui::CalcTextSize("Close").x * 3,
                     ImGui::CalcTextSize("Close").y * 1.5)))
               {
                  modal_about_ease = false;
               }
               ImGui::SetWindowPos(ImVec2(
                  (window.GetWindowWidth() / 2) - (ImGui::GetWindowSize().x / 2),
                  (window.GetWindowHeight() / 2) - (ImGui::GetWindowSize().y / 2)
               ));
            }
            ImGui::PopStyleVar();
            ImGui::End();
         }
      }
};



DYLIB_API Ease::BaseModule* Create()
{
   EaseEditor* lib = new EaseEditor;
   lib->metadata.authorName = "Ease";
   lib->metadata.moduleName = "Editor";
   lib->metadata.version = 1;
   g_Editor = lib;
   return lib;
}

DYLIB_API void Destroy(Ease::BaseModule* lib)
{
   delete reinterpret_cast<EaseEditor*>(lib);
}


void Print()
{
   g_Editor->m_ConsoleText += g_Editor->userValues["PrintMsg"].str_value + "\n";
   std::cout << "[Editor] " << g_Editor->userValues["PrintMsg"].str_value << std::endl;
}
