#include "ECS/Systems/Systems.hpp"
#include "ECS/Scene/Scene.hpp"

#include "ECS/Components/UITransform/UITransform.hpp"
#include "ECS/Components/Button/Button.hpp"
#include "ECS/Components/NinePatchRect/NinePatchRect.hpp"

#include "Core/Renderer.hpp"
#include "Resource/ResourceManager.hpp"
#include "Resource/Texture/Texture.hpp"

namespace Ease::Systems
{
   void System_ButtonPicking(Ease::Scene* pScene)
   {
      ResourceManager<Ease::Texture>& loader = pScene->GetResourceManager<Ease::Texture>();
      

      auto view = pScene->m_Registry.view<Component::UITransform, Component::Button>();
      for(const auto& entityID : view)
      {
         Entity entity(entityID, &pScene->m_Registry);
         auto& transformc = entity.GetComponent<Component::UITransform>();
         auto& buttonc = entity.GetComponent<Component::Button>();
         
         if(!buttonc.Visible()) continue;
         Component::Button::ButtonState state = buttonc.GetState();
         Ease::Component::NinePatchRect nPatch;
         switch (state)
         {
         case Component::Button::ButtonState::NORMAL:
            nPatch = buttonc.TextureNormal();
            break;
         case Component::Button::ButtonState::HOVER:
            nPatch = buttonc.TextureHover();
            break;
         case Component::Button::ButtonState::CLICK:
            nPatch = buttonc.TextureClick();
            break;
         case Component::Button::ButtonState::DISABLED:
            nPatch = buttonc.TextureDisabled();
            break;
         default:
            continue;
            break;
         }
         
         // Ease::Renderer::get_singleton().DrawNinePatchRectWithID(transformc.Position(), transformc.Size(), transformc.Scale(), 0, transformc.Rotation(), nPatch, (uint32_t)entityID);
      }
   }
} // namespace Ease::Systems
