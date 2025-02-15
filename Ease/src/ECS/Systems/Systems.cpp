#include "ECS/Systems/Systems.hpp"
#include "ECS/Scene/Scene.hpp"

namespace Ease::Systems
{
   void ProcessAll(Ease::Scene* pScene, SystemsFlags flags, bool pickable /* = false*/)
   {
      if(flags & SystemsFlags::SpriteRenderer2D)
         Systems::System_SpriteRenderer2D(pScene, pickable);

      if(flags & SystemsFlags::AnimatedSprite2D)
         Systems::System_AnimatedSprite2D(pScene, pickable);
         
      if(flags & SystemsFlags::TextRenderer2D)
         Systems::System_TextRenderer2D(pScene, pickable);
      
      if(flags & SystemsFlags::PhysicsBody2D)
         Systems::System_PhysicsBody2D(pScene);
      
      if(flags & SystemsFlags::Physics2DDebugDraw)
         Systems::System_Physics2DDebugDraw(pScene, pickable);
      
      if(flags & SystemsFlags::ButtonDraw)
         Systems::System_ButtonDraw(pScene, pickable);
      
      if(flags & SystemsFlags::ButtonPicking)
         Systems::System_ButtonPicking(pScene);
      
      if(flags & SystemsFlags::ButtonLogic)
         Systems::System_ButtonLogic(pScene);
      
      if(flags & SystemsFlags::Camera2DLogic)
         Systems::System_Camera2DLogic(pScene);
      
      if(flags & SystemsFlags::NativeBehaviourLogic)
         Systems::System_NativeBehaviour(pScene);
   }
} // namespace Ease::Systems
