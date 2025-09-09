#include "MemorySystem.h"
#include <iostream>

MemorySystem memorySystem = MemorySystem();

extern "C" 
{
    MemoryLeakPtr MemoryLeakPtr_NewPtr(size_t memorySize, size_t elementCount, const char* file, int line, const char* type, const char* func, const char* notes)
    {
        void* memory = nullptr;
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

        const char* fileStr = file ? _strdup(file) : nullptr;
        const char* lineStr = line ? _strdup(std::to_string(line).c_str()) : nullptr;
        const char* typeStr = type ? _strdup(type) : nullptr;
        const char* funcStr = func ? _strdup(func) : nullptr;
        const char* notesStr = notes ? _strdup(notes) : nullptr;

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

    void MemoryLeakPtr_DeletePtr(void* memoryLeakPtr)
    {
        delete[] static_cast<byte*>(memoryLeakPtr);
    }

    void MemoryLeakPtr_DanglingPtrMessage(MemoryLeakPtr* ptr)
    {
        if (ptr)
        {
            String fileStr(ptr->File);
            String lineStr(ptr->Line);
            String typeStr(ptr->Type);
            String functionStr(ptr->Function);
            String noteStr(ptr->Notes);

            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole != INVALID_HANDLE_VALUE)
            {
                CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
                GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
                WORD originalAttributes = consoleInfo.wAttributes;


                size_t pos = fileStr.find_last_of("\\/");
                String filename = (pos == String::npos) ? fileStr : fileStr.substr(pos + 1);

                SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                std::cout << "Error: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);

                std::cout << "Memory Leak at: 0x" << ptr->PtrAddress;

                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " Size: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << ptr->PtrElements * sizeof(byte);

                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " IsArray: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << (ptr->isArray ? "Yes" : "No");

                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " File: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << (fileStr.empty() ? "Unknown" : filename);

                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " Line: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << (lineStr.empty() ? "Unknown" : lineStr);

                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " Type: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << (typeStr.empty() ? "Unknown" : typeStr);

                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " Function: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << (functionStr.empty() ? "Unknown" : functionStr);

                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " Notes: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << (noteStr.empty() ? "None" : noteStr) << std::endl;
            }
            else
            {
                std::cerr << "Memory Leak at: Pointer: " << ptr->PtrAddress
                          << " Size: " << ptr->PtrElements * sizeof(byte)
                          << " IsArray: " << (ptr->isArray ? "Yes" : "No")
                          << " File: " << (fileStr.empty() ? "Unknown" : fileStr)
                          << " Line: " << (lineStr.empty() ? "Unknown" : lineStr)
                          << " Function: " << (functionStr.empty() ? "Unknown" : functionStr)
                          << " Notes: " << (noteStr.empty() ? "None" : noteStr) << std::endl;
            }
        }
    }

    void MemoryLeakPtr_ReportLeaks() 
    {
        memorySystem.ReportLeaks();
        #ifdef _DEBUG
        _CrtDumpMemoryLeaks();
        #endif
    }
}