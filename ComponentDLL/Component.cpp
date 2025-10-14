#include "pch.h"
#include "Component.h"
//#include "GameObject.h"
//#include "Transform2DComponent.h"
//#include "InputComponent.h"
//#include "Sprite.h"
//
//uint32 Component_RemoveTransform2DComponent(uint gameObjectId)
//{
//    auto it = std::find(gameObjectArchive.Transform2DComponentList.begin(), gameObjectArchive.Transform2DComponentList.end(), [gameObjectId](const Transform2DComponent& transformComponent)
//        {
//            return transformComponent.GameObjectId == gameObjectId;
//        });
//    uint32 removeIndex = std::distance(gameObjectArchive.Transform2DComponentList.begin(), it);
//    gameObjectArchive.Transform2DComponentList.erase(gameObjectArchive.Transform2DComponentList.begin() + removeIndex);
//    return removeIndex;
//}
//
//uint32 Component_RemoveInputComponent(uint gameObjectId)
//{
//    auto it = std::find(gameObjectArchive.InputComponentList.begin(), gameObjectArchive.InputComponentList.end(), [gameObjectId](const InputComponent& inputComponent)
//        {
//            return inputComponent.GameObjectId == gameObjectId;
//        });
//    uint32 removeIndex = std::distance(gameObjectArchive.InputComponentList.begin(), it);
//    gameObjectArchive.InputComponentList.erase(gameObjectArchive.InputComponentList.begin() + removeIndex);
//    return removeIndex;
//}
//
//uint32 Component_RemoveSpriteComponent(uint gameObjectId)
//{
//    auto it = std::find(spriteArchive.SpriteList.begin(), spriteArchive.SpriteList.end(), [gameObjectId](const Sprite& sprite)
//        {
//            return sprite.GameObjectId == gameObjectId;
//        });
//    uint32 removeIndex = std::distance(spriteArchive.SpriteList.begin(), it);
//    spriteArchive.SpriteList.erase(spriteArchive.SpriteList.begin() + removeIndex);
//    return removeIndex;
//}