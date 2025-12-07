#include "MemorySystem.h"
#include <algorithm>

MemorySystem& memorySystem = MemorySystem::Get();

const char* MemorySystem::AddPtrBuffer(const char* str, const char* file, int line, const char* func, const char* notes) 
{
    if (!str) return nullptr;
    size_t len = std::strlen(str) + 1;
    char* buffer = static_cast<char*>(::operator new(len));
    if (!buffer) return nullptr;
    std::strcpy(buffer, str);

    MemoryLeakPtr info;
    info.PtrAddress = buffer;
    info.PtrElements = 1;
    info.isArray = false;
    info.File = file ? file : "unknown";
    info.Line = std::to_string(line);
    info.Type = "const char*";
    info.Function = func ? func : "unknown";
    info.Notes = notes ? notes : "";

    std::lock_guard<std::mutex> lock(Mutex);
    PtrAddressMap[buffer] = std::move(info);
    return buffer;
}

MemoryLeakPtr MemorySystem::NewPtr(size_t memorySize, size_t elementCount, const char* file, int line, const char* type, const char* func, const char* notes)
{
    void* memory = nullptr;
#if defined(_WIN32)
    const char* fileStr = file ? _strdup(file) : nullptr;
    const char* lineStr = line ? _strdup(std::to_string(line).c_str()) : nullptr;
    const char* typeStr = type ? _strdup(type) : nullptr;
    const char* funcStr = func ? _strdup(func) : nullptr;
    const char* notesStr = notes ? _strdup(notes) : nullptr;
#else
    const char* fileStr = file ? strdup(file) : nullptr;
    const char* lineStr = line ? strdup(std::to_string(line).c_str()) : nullptr;
    const char* typeStr = type ? strdup(type) : nullptr;
    const char* funcStr = func ? strdup(func) : nullptr;
    const char* notesStr = notes ? strdup(notes) : nullptr;
#endif
    try
    {
        memory = new byte[memorySize * elementCount];
    }
    catch (const std::bad_alloc&)
    {
        fprintf(stderr, "Allocation failed: %s\n", notes ? notes : "Unknown");
        return MemoryLeakPtr
        {
            .PtrAddress = nullptr,
            .PtrElements = 0,
            .isArray = false,

        };
    }

    return MemoryLeakPtr
    {
        .PtrAddress = memory,
        .PtrElements = elementCount,
        .isArray = elementCount > 1,
        .File = fileStr,
        .Line = lineStr,
        .Type = typeStr,
        .Function = funcStr,
        .Notes = notesStr
    };
}

void MemorySystem::DeletePtr(void* ptr)
{
    std::lock_guard<std::mutex> lock(Mutex);

    auto it = PtrAddressMap.find(ptr);
    if (it != PtrAddressMap.end())
    {
        MemoryLeakPtr memoryLeakPtr = it->second;
        delete[] static_cast<byte*>(memoryLeakPtr.PtrAddress);
        FreeMemory(memoryLeakPtr);
        PtrAddressMap.erase(it);
        ptr = nullptr;
    }
    else
    {
        std::cerr << "Warning: Attempted to remove unregistered pointer: " << ptr << std::endl;
    }
}

void MemorySystem::ReportLeaks() 
{
    std::lock_guard<std::mutex> lock(Mutex);

    if (PtrAddressMap.empty()) {
        std::cout << "No memory leaks detected.\n";
        return;
    }

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD originalAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    if (hConsole != INVALID_HANDLE_VALUE) {
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        originalAttributes = consoleInfo.wAttributes;
    }
#endif

    size_t maxAddr = 6;
    size_t maxSize = 4;
    size_t maxArray = 5; 
    size_t maxFile = 4; 
    size_t maxLine = 4; 
    size_t maxFunc = 4; 
    size_t maxNotes = 5;

    for (const auto& [addr, info] : PtrAddressMap) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << addr;
        maxAddr = std::max(maxAddr, oss.str().length());

        std::string sizeStr = std::to_string(info.PtrElements);
        maxSize = std::max(maxSize, sizeStr.length());

        std::string arrayStr = info.isArray ? "Yes" : "No";
        maxArray = std::max(maxArray, arrayStr.length());

        std::string fileBase = info.File.substr(info.File.find_last_of("\\/") + 1);
        maxFile = std::max(maxFile, fileBase.length());

        maxLine = std::max(maxLine, info.Line.length());
        maxFunc = std::max(maxFunc, info.Function.length());
        maxNotes = std::max(maxNotes, info.Notes.length());
    }

    std::cout << "\nMEMORY LEAKS DETECTED: " << PtrAddressMap.size() << "\n";
    std::cout << String(119, '-') << "\n";

    for (const auto& [addr, info] : PtrAddressMap) {
        std::ostringstream addrStream;
        addrStream << "0x" << std::hex << std::uppercase << addr;
        String addrStr = addrStream.str();

        String sizeStr = std::to_string(info.PtrElements);
        String arrayStr = info.isArray ? "Yes" : "No";
        String fileBase = info.File.substr(info.File.find_last_of("\\/") + 1);
        String lineStr = info.Line;
        String funcStr = info.Function;
        String noteStr = info.Notes.empty() ? "None" : info.Notes;

        #ifdef _WIN32
                if (hConsole != INVALID_HANDLE_VALUE) 
                {
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
                    std::cout << std::left << std::setw(7) << "Error: ";
                    SetConsoleTextAttribute(hConsole, originalAttributes);
                    std::cout << "Memory Leak at: " << std::setw(maxAddr) << addrStr;

                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    std::cout << " Size: ";
                    SetConsoleTextAttribute(hConsole, originalAttributes);
                    std::cout << std::setw(maxSize) << sizeStr;

                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    std::cout << " IsArray: ";
                    SetConsoleTextAttribute(hConsole, originalAttributes);
                    std::cout << std::setw(maxArray) << arrayStr;

                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    std::cout << " File: ";
                    SetConsoleTextAttribute(hConsole, originalAttributes);
                    std::cout << std::setw(maxFile) << fileBase;

                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    std::cout << " Line: ";
                    SetConsoleTextAttribute(hConsole, originalAttributes);
                    std::cout << std::setw(maxLine) << lineStr;

                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    std::cout << " Function: ";
                    SetConsoleTextAttribute(hConsole, originalAttributes);
                    std::cout << std::setw(maxFunc) << funcStr;

                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    std::cout << " Notes: ";
                    SetConsoleTextAttribute(hConsole, originalAttributes);
                    std::cout << noteStr << "\n";
                }
                else
        #endif
        {
            std::cout << "Error: Memory Leak at: " << addrStr
                << " Size: " << sizeStr
                << " IsArray: " << arrayStr
                << " File: " << fileBase
                << " Line: " << lineStr
                << " Function: " << funcStr
                << " Notes: " << noteStr << "\n";
        }
    }

    std::cout << std::string(119, '-') << "\n";
}

extern "C"
{
    MemoryLeakPtr MemoryLeakPtr_NewPtr(size_t memorySize, size_t elementCount, const char* file, int line, const char* type, const char* func, const char* notes)
    {
        return memorySystem.NewPtr(memorySize, elementCount, file, line, type, func, notes);
    }

    void MemoryLeakPtr_DeletePtr(void* memoryLeakPtr)
    {
        memorySystem.DeletePtr(memoryLeakPtr);
    }

    void MemoryLeakPtr_ReportLeaks()
    {
        memorySystem.ReportLeaks();
    }
}