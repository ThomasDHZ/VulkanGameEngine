#pragma once
#include "DLL.h"
#include "TypeDef.h"
#include <mutex>

struct MemoryLeakPtr 
{
    void* PtrAddress;        
    size_t PtrElements;      
    bool isArray;           
    const char* DanglingPtrMessage;
    const char* File;
    const char* Line;
    const char* Function;
    const char* Notes;
};

extern "C" 
{
    DLL_EXPORT MemoryLeakPtr MemoryLeakPtr_NewPtr(size_t memorySize, size_t elementCount, const char* file, int line, const char* func, const char* notes);
    DLL_EXPORT void MemoryLeakPtr_DeletePtr(void* ptr);
    DLL_EXPORT void MemoryLeakPtr_DanglingPtrMessage(MemoryLeakPtr* ptr);
}

class MemorySystem 
{
private:
    std::mutex Mutex;
    UnorderedMap<void*, MemoryLeakPtr> PtrAddressMap;

public:
    MemorySystem()
    {
        #ifdef _DEBUG
            _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        #endif
    }

    template <class T>
    T* AddPtrBuffer(size_t elementCount, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);

        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), elementCount, file, line, func, notes);
        PtrAddressMap[memoryLeakPtr.PtrAddress] = memoryLeakPtr;
        return reinterpret_cast<T*>(memoryLeakPtr.PtrAddress);
    }


    template <class T>
    T* AddPtrBuffer(T elementData, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);

        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), 1, file, line, func, notes);
        PtrAddressMap[memoryLeakPtr.PtrAddress] = memoryLeakPtr;
        return reinterpret_cast<T*>(memoryLeakPtr.PtrAddress);
    }

    template <class T>
    T* AddPtrBuffer(T* elementData, size_t elementCount, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);
        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), elementCount, file, line, func, notes);
        if (!memoryLeakPtr.PtrAddress)
        {
            std::cerr << "Failed to allocate memory for " << notes << " at " << file << ":" << line << std::endl;
            return nullptr;
        }
        T* buffer = reinterpret_cast<T*>(memoryLeakPtr.PtrAddress);
        for (size_t x = 0; x < elementCount; ++x) 
        {
            new (&buffer[x]) T(elementData[x]);
        }
        PtrAddressMap[memoryLeakPtr.PtrAddress] = memoryLeakPtr;
        return buffer;
    }

    template <class T>
    void RemovePtrBuffer(T*& ptr)
    {
        std::lock_guard<std::mutex> lock(Mutex);
        void* voidPtr = reinterpret_cast<void*>(ptr);

        auto it = PtrAddressMap.find(voidPtr);
        if (it != PtrAddressMap.end())
        {
            MemoryLeakPtr& memoryLeakPtr = it->second;
            MemoryLeakPtr_DeletePtr(memoryLeakPtr.PtrAddress);
            PtrAddressMap.erase(it);
            ptr = nullptr;
        }
        else
        {
            std::cerr << "Warning: Attempted to remove unregistered pointer: " << ptr << std::endl;
        }
    }

    void ReportLeaks() 
    {
        std::lock_guard<std::mutex> lock(Mutex);

        if (!PtrAddressMap.empty()) 
        {
            fprintf(stderr, "Memory leaks detected in DLL:\n");
            for (auto& ptr : PtrAddressMap) 
            {
                MemoryLeakPtr_DanglingPtrMessage(&ptr.second);
            }
        }
    }
};
DLL_EXPORT MemorySystem memorySystem;