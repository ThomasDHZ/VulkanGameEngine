#pragma once
#include "DLL.h"

class NetworkingSystem
{
public:
    static NetworkingSystem& Get();

private:
    NetworkingSystem() = default;
    ~NetworkingSystem() = default;
    NetworkingSystem(const NetworkingSystem&) = delete;
    NetworkingSystem& operator=(const NetworkingSystem&) = delete;
    NetworkingSystem(NetworkingSystem&&) = delete;
    NetworkingSystem& operator=(NetworkingSystem&&) = delete;

public:

    DLL_EXPORT void StartUp();
};
extern DLL_EXPORT NetworkingSystem& networkingSystem;
inline NetworkingSystem& NetworkingSystem::Get()
{
    static NetworkingSystem instance;
    return instance;
}

