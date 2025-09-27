#pragma once
#include "DLL.h"
#include "TypeDef.h"
#include <mutex>

struct MemoryLeakPtr
{
    void* PtrAddress;
    size_t PtrElements;
    bool isArray;
    const char* File;
    const char* Line;
    const char* Type;
    const char* Function;
    const char* Notes;
};

extern "C"
{
    DLL_EXPORT MemoryLeakPtr MemoryLeakPtr_NewPtr(size_t memorySize, size_t elementCount, const char* file, int line, const char* type, const char* func, const char* notes);
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

        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), elementCount, file, line, typeid(T).name(), func, notes);
        PtrAddressMap[memoryLeakPtr.PtrAddress] = memoryLeakPtr;
        return reinterpret_cast<T*>(memoryLeakPtr.PtrAddress);
    }


    template <class T>
    T* AddPtrBuffer(T elementData, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);

        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), 1, file, line, typeid(T).name(), func, notes);
        PtrAddressMap[memoryLeakPtr.PtrAddress] = memoryLeakPtr;
        return reinterpret_cast<T*>(memoryLeakPtr.PtrAddress);
    }

    template <class T>
    T* AddPtrBuffer(T* elementData, size_t elementCount, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);
        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), elementCount, file, line, typeid(T).name(), func, notes);
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

    const char* AddPtrBuffer(const char* elementData, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);
        if (!elementData)
        {
            std::cerr << "Null string data for " << notes << " at " << file << ":" << line << " in " << func << std::endl;
            return nullptr;
        }

        size_t strLen = std::strlen(elementData) + 1;
        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(strLen, 1, file, line, "const char*", func, notes);
        if (!memoryLeakPtr.PtrAddress)
        {
            std::cerr << "Failed to allocate memory for string, length " << strLen << " (" << notes << ") at " << file << ":" << line << " in " << func << std::endl;
            return nullptr;
        }

        char* buffer = reinterpret_cast<char*>(memoryLeakPtr.PtrAddress);
        std::strcpy(buffer, elementData);

        PtrAddressMap[memoryLeakPtr.PtrAddress] = memoryLeakPtr;
        return buffer;
    }

    const char* ReplacePtrBuffer(const char* srcString, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);
        if (!srcString)
        {
            std::cerr << "Null string data for " << notes << " at " << file << ":" << line << " in " << func << std::endl;
            return nullptr;
        }

        size_t strLen = std::strlen(srcString) + 1;
        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(strLen, 1, file, line, "const char*", func, notes);
        if (!memoryLeakPtr.PtrAddress)
        {
            std::cerr << "Failed to allocate memory for string, length " << strLen << " (" << notes << ") at " << file << ":" << line << " in " << func << std::endl;
            return nullptr;
        }

        char* buffer = reinterpret_cast<char*>(memoryLeakPtr.PtrAddress);
        std::strcpy(buffer, srcString);
        void* voidPtr = const_cast<void*>(reinterpret_cast<const void*>(srcString));

        auto it = PtrAddressMap.find(voidPtr);
        if (it != PtrAddressMap.end())
        {
            MemoryLeakPtr& memoryLeakPtr = it->second;
            MemoryLeakPtr_DeletePtr(memoryLeakPtr.PtrAddress);
            PtrAddressMap.erase(it);
            srcString = nullptr;
        }

        PtrAddressMap[memoryLeakPtr.PtrAddress] = memoryLeakPtr;
        return buffer;
    }

    template <class T>
    T* ReplacePtrBuffer(T*& obj, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);

        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), 1, file, line, typeid(T).name(), func, notes);
        T* buffer = reinterpret_cast<T*>(memoryLeakPtr.PtrAddress);

        void* voidPtr = reinterpret_cast<void*>(obj);
        auto it = PtrAddressMap.find(voidPtr);
        if (it != PtrAddressMap.end())
        {
            MemoryLeakPtr& memoryLeakPtr = it->second;
            MemoryLeakPtr_DeletePtr(memoryLeakPtr.PtrAddress);
            PtrAddressMap.erase(it);
            obj = nullptr;
        }
        else
        {
            std::cerr << "Warning: Attempted to remove unregistered pointer: " << obj << std::endl;
        }

        PtrAddressMap[memoryLeakPtr.PtrAddress] = memoryLeakPtr;
        return buffer;
    }

    template <class T>
    T* ReplacePtrBuffer(T*& obj, size_t elementCount, const char* file, int line, const char* func, const char* notes = "")
    {
        std::lock_guard<std::mutex> lock(Mutex);
        MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), elementCount, file, line, typeid(T).name(), func, notes);
        if (!memoryLeakPtr.PtrAddress)
        {
            std::cerr << "Failed to allocate memory for " << notes << " at " << file << ":" << line << std::endl;
            return nullptr;
        }
        T* buffer = reinterpret_cast<T*>(memoryLeakPtr.PtrAddress);
        for (size_t x = 0; x < elementCount; ++x)
        {
            new (&buffer[x]) T(obj[x]);
        }

        void* voidPtr = reinterpret_cast<void*>(obj);
        auto it = PtrAddressMap.find(voidPtr);
        if (it != PtrAddressMap.end())
        {
            MemoryLeakPtr& memoryLeakPtr = it->second;
            MemoryLeakPtr_DeletePtr(memoryLeakPtr.PtrAddress);
            PtrAddressMap.erase(it);
            obj = nullptr;
        }
        else
        {
            std::cerr << "Warning: Attempted to remove unregistered pointer: " << obj << std::endl;
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

    void RemovePtrBuffer(const char*& ptr)
    {
        std::lock_guard<std::mutex> lock(Mutex);
        void* voidPtr = const_cast<void*>(reinterpret_cast<const void*>(ptr));

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
            std::cout << ("\nMemory leaks detected:" + std::to_string(PtrAddressMap.size()) + " Memory leaks found.") << std::endl;
            for (auto& ptr : PtrAddressMap)
            {
                MemoryLeakPtr_DanglingPtrMessage(&ptr.second);
            }
        }
    }
};
DLL_EXPORT MemorySystem memorySystem;