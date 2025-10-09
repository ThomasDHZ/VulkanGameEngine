#include "pch.h"
#include "GameObject.h"
#include "MegaManShot.h"

void MegaManShot_Behaviors(GameObjectBehavior& componentBehavior)
{
    componentBehavior.KeyBoardInput = nullptr;
    componentBehavior.ControllerInput = nullptr;
    componentBehavior.Update = MegaManShot_Update;
    componentBehavior.Destroy = MegaManShot_Destroy;
}

void MegaManShot_Update(uint gameObjectId, const float& deltaTime)
{
   Transform2DComponent& spriteTransform =  GameObject_FindTransform2DComponent(gameObjectId);
   spriteTransform.GameObjectPosition.x += 1;
}

 void MegaManShot_Destroy()
{
}
