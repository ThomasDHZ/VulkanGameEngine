#include "NetworkingSystem.h"
#include "DLLSystem.h"
#include "EngineConfigSystem.h"

NetworkingSystem& networkingSystem = NetworkingSystem::Get();
bool NetworkingSystem::NetworkingSystemInitialized = false;

void NetworkingSystem::StartUp()
{
    bool success = dllSystem.InitializeDLLRuntime(configSystem.NetworkingDLL);
    if (success) NetworkingSystemInitialized = true;
}

