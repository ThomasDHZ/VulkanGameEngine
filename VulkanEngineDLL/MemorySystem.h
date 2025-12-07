// MemorySystem.h
#pragma once
#include "Platform.h"

struct MemoryLeakPtr {
    void* PtrAddress = nullptr;
    size_t PtrElements = 0;
    bool isArray = false;
    std::string File;
    std::string Line;
    std::string Type;
    std::string Function;
    std::string Notes;
};

class MemorySystem {
public:
    static MemorySystem& Get();

private:
    MemorySystem() = default;
    ~MemorySystem() = default;
    MemorySystem(const MemorySystem&) = delete;
    MemorySystem& operator=(const MemorySystem&) = delete;
    MemorySystem(MemorySystem&&) = delete;
    MemorySystem& operator=(MemorySystem&&) = delete;

    std::mutex Mutex;
    std::unordered_map<void*, MemoryLeakPtr> PtrAddressMap;

    void FreeMemory(MemoryLeakPtr& ptr)
    {
        ptr = MemoryLeakPtr{};
    }

    void DeletePtrImpl(void* ptr) {
        std::lock_guard<std::mutex> lock(Mutex);
        auto it = PtrAddressMap.find(ptr);
        if (it == PtrAddressMap.end()) {
            std::cerr << "Warning: Double free or unregistered pointer: " << ptr << std::endl;
            return;
        }

        MemoryLeakPtr& info = it->second;
        if (info.isArray) {
            char* bytes = static_cast<char*>(info.PtrAddress);
        }

        ::operator delete(info.PtrAddress);
        FreeMemory(it->second);
        PtrAddressMap.erase(it);
    }

public:

    template <typename T>
    T* AddPtrBuffer(size_t elementCount, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);
        size_t size = sizeof(T) * elementCount;
        void* memory = ::operator new(size);
        if (!memory) return nullptr;

        T* buffer = static_cast<T*>(memory);
        for (size_t i = 0; i < elementCount; ++i) {
            new (&buffer[i]) T();
        }

        MemoryLeakPtr info;
        info.PtrAddress = memory;
        info.PtrElements = elementCount;
        info.isArray = elementCount > 1;
        info.File = file ? file : "unknown";
        info.Line = std::to_string(line);
        info.Type = typeid(T).name();
        info.Function = func ? func : "unknown";
        info.Notes = notes ? notes : "";

        PtrAddressMap[memory] = std::move(info);
        return buffer;
    }

    template <typename T>
    T* AddPtrBuffer(T elementData, const char* file, int line, const char* func, const char* notes = "") {
        T* ptr = AddPtrBuffer<T>(1, file, line, func, notes);
        if (ptr) *ptr = elementData;
        return ptr;
    }

    template <typename T>
    T* AddPtrBuffer(T* src, size_t count, const char* file, int line, const char* func, const char* notes = "") {
        T* ptr = AddPtrBuffer<T>(count, file, line, func, notes);
        if (ptr && src) {
            for (size_t i = 0; i < count; ++i) {
                ptr[i] = src[i];
            }
        }
        return ptr;
    }

    template <typename T>
    T* ReplacePtrBuffer(T*& oldPtr, size_t count, const char* file, int line, const char* func, const char* notes = "") {
        T* newPtr = AddPtrBuffer<T>(count, file, line, func, notes);
        if (newPtr && oldPtr) {
            for (size_t i = 0; i < count; ++i) {
                newPtr[i] = std::move(oldPtr[i]);
            }
            DeletePtr(oldPtr);
        }
        oldPtr = newPtr;
        return newPtr;
    }

    template <typename T>
    void DeletePtr(T*& ptr) {
        if (!ptr) return;
        DeletePtrImpl(ptr);
        ptr = nullptr;
    }

    void DeletePtr(const char*& ptr) {
        if (!ptr) return;
        DeletePtrImpl(const_cast<char*>(ptr));
        ptr = nullptr;
    }

    DLL_EXPORT const char* AddPtrBuffer(const char* str, const char* file, int line, const char* func, const char* notes = "");
    DLL_EXPORT MemoryLeakPtr NewPtr(size_t size, size_t count, const char* file, int line, const char* type, const char* func, const char* notes);
    DLL_EXPORT void DeletePtr(void* ptr);
    DLL_EXPORT void ReportLeaks();
};
extern DLL_EXPORT MemorySystem& memorySystem;
inline MemorySystem& MemorySystem::Get()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    static MemorySystem instance;
    return instance;
}

extern "C" {
     MemoryLeakPtr MemoryLeakPtr_NewPtr(size_t size, size_t count, const char* file, int line, const char* type, const char* func, const char* notes);
     void MemoryLeakPtr_DeletePtr(void* ptr);
     void MemoryLeakPtr_ReportLeaks();
}