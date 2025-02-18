#include "GameObjectComponent.h"
#include "MemoryManager.h"

std::string GameObjectComponent::GetCSNameSpacePath(ComponentTypeEnum componentType)
{
    switch (componentType) 
    {
        case kRenderMesh2DComponent: return CSNameSpace + "RenderMesh2DComponent";
        case kTransform2DComponent:  return CSNameSpace + "Transform2DComponent";
        case kInputComponent:        return CSNameSpace + "InputComponent";
        default:                     return "Undefined";
    }
}

GameObjectComponent::GameObjectComponent()
{

}

GameObjectComponent::GameObjectComponent(void* ptr, SharedPtr<GameObject> parentGameObjectPtr, ComponentTypeEnum componentType)
{
    //void* reqw = this;
    //Name = std::make_shared<Coral::String>(Coral::String().New("component"));
    ComponentType = componentType;
    //ParentGameObjectPtr = parentGameObjectPtr;

    //CSclass = std::make_shared<Coral::Type>(MemoryManager::GetECSassemblyModule()->GetType(GetCSNameSpacePath(componentType)));
    //SharedPtr<GameObject> parentPtr = ParentGameObjectPtr.lock();
    //if (parentPtr)
    //{
    //    CSobject = std::make_shared<Coral::ManagedObject>(CSclass->CreateInstance(ptr, parentGameObjectPtr.get(), parentPtr->GetCSObjectHandle()));
    //}
}

GameObjectComponent::GameObjectComponent(void* ptr, SharedPtr<GameObject> parentGameObjectPtr, String name, ComponentTypeEnum componentType)
{
    //Name = std::make_shared<Coral::String>(Coral::String().New(name));
    ComponentType = componentType;
    //ParentGameObjectPtr = parentGameObjectPtr;

    //CSclass = std::make_shared<Coral::Type>(MemoryManager::GetECSassemblyModule()->GetType(GetCSNameSpacePath(componentType)));
    //SharedPtr<GameObject> parentPtr = ParentGameObjectPtr.lock();
    //if (parentPtr)
    //{
    //    CSobject = std::make_shared<Coral::ManagedObject>(CSclass->CreateInstance(ptr, parentGameObjectPtr.get(), parentPtr->GetCSObjectHandle()));
    //}
}

GameObjectComponent::~GameObjectComponent()
{
}

void GameObjectComponent::Input(float deltaTime)
{
    //CSobject->InvokeMethod("Input", KeyboardKeyCode::KEY_D, deltaTime);
}

void GameObjectComponent::Update(float deltaTime)
{
    //CSobject->InvokeMethod("Update", deltaTime);
}

void GameObjectComponent::BufferUpdate(VkCommandBuffer& commandBuffer, float deltaTime)
{
    //CSobject->InvokeMethod("BufferUpdate", commandBuffer, deltaTime);
}

void GameObjectComponent::Draw(VkCommandBuffer& commandBuffer, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkDescriptorSet& descriptorSet, SceneDataBuffer& sceneProperties)
{

}

void GameObjectComponent::Destroy()
{
    //CSobject->InvokeMethod("Destroy");
}

SharedPtr<GameObjectComponent> GameObjectComponent::Clone() const
{
    return SharedPtr<GameObjectComponent>();
}

size_t GameObjectComponent::GetMemorySize() const
{
    return sizeof(GameObjectComponent);
}
