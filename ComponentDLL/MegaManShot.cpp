#include "pch.h"
#include "MegaManShot.h"

void MegaManShot_Behaviors(ComponentBehavior& componentBehavior)
{
    componentBehavior.KeyBoardInput = nullptr;
    componentBehavior.ControllerInput = nullptr;
    componentBehavior.Movement = MegaManShot_Movement;
    componentBehavior.Destroy = MegaManShot_Destroy;
}

void MegaManShot_Movement(const float& deltaTime, Transform2DComponent& transform2D, bool direction)
{
    transform2D.GameObjectPosition += 200;
}

 void MegaManShot_Destroy()
{
}
