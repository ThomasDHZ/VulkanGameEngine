#pragma once
#include "Platform.h"

class NetworkingSystem
{
public:
    static NetworkingSystem& Get();
    static bool NetworkingSystemInitialized;

private:
    NetworkingSystem() = default;
    ~NetworkingSystem() = default;
    NetworkingSystem(const NetworkingSystem&) = delete;
    NetworkingSystem& operator=(const NetworkingSystem&) = delete;
    NetworkingSystem(NetworkingSystem&&) = delete;
    NetworkingSystem& operator=(NetworkingSystem&&) = delete;

    intptr_t dllInstancePtr = 0;

public:
    DLL_EXPORT void StartUp();
    intptr_t(*CreateObject)() = nullptr;
    void (*CreateAccount)(intptr_t instance, const char* userName, const char* password, const char* email) = nullptr;
    void (*SignIn)(intptr_t instance, const char* userName, const char* password) = nullptr;
    void (*Destroy)(intptr_t instance) = nullptr;
};
extern DLL_EXPORT NetworkingSystem& networkingSystem;
inline NetworkingSystem& NetworkingSystem::Get()
{
    static NetworkingSystem instance;
    return instance;
}

