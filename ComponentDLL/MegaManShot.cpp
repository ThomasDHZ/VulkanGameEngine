#include "pch.h"
#include "GameObject.h"
#include "MegaManShot.h"
#include "MegaManObject.h"

void MegaManShot_Behaviors(GameObjectBehavior& componentBehavior)
{
    componentBehavior.KeyBoardInput = nullptr;
    componentBehavior.ControllerInput = nullptr;
    componentBehavior.Update = MegaManShot_Update;
    componentBehavior.Destroy = MegaManShot_Destroy;
}

void MegaManShot_Update(uint gameObjectId, const float& deltaTime)
{
   GameObject gameObject = GameObject_FindGameObject(gameObjectId);
   if (gameObject.GameObjectType == GameObjectTypeEnum::kGameObjectMegaManShot)
   {
       MegaManShot* objectData = static_cast<MegaManShot*>(gameObject.GameObjectData);
       if (objectData->MegaManShotDistanceTraveled > objectData->MegaManShotDistance)
       {
           MegaManShot_Destroy(gameObjectId);
       }
       else
       {
           objectData->MegaManShotDistanceTraveled += 1;

           Transform2DComponent& spriteTransform = GameObject_FindTransform2DComponent(gameObjectId);
           spriteTransform.GameObjectPosition.x += 1;
       }
   }
}

void MegaManShot_Destroy(uint gameObjectId)
{
    GameObject& gameObject = GameObject_FindGameObject(gameObjectId);
    GameObject& parentGameObject = GameObject_FindGameObject(gameObject.ParentGameObjectId);
    MegaManObject* objectData = static_cast<MegaManObject*>(parentGameObject.GameObjectData);
    objectData->CurrentShotCount--;
    gameObject.GameObjectAlive = false;
    if (objectData->CurrentShotCount == UINT32_MAX)
    {
        objectData->CurrentShotCount = 0;
    }
}

