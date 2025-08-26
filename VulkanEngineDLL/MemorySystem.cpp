#include "MemorySystem.h"
#include <iostream>

MemorySystem memorySystem = MemorySystem();

extern "C" 
{
    MemoryLeakPtr MemoryLeakPtr_NewPtr(size_t memorySize, size_t elementCount, const char* file, int line, const char* func, const char* notes)
    {
        void* memory = nullptr;
        try
        {
            memory = 
                
                
                
                
                
                byte[memorySize * elementCount];
        }
        catch (const std::bad_alloc&)
        {
            fprintf(stderr, "Allocation failed: %s\n", notes ? notes : "Unknown");
            return MemoryLeakPtr
            {
                .PtrAddress = nullptr,
                .PtrElements = 0,
                .isArray = false,
                .DanglingPtrMessage = ""
            };
        }

        return MemoryLeakPtr
        {
            .PtrAddress = memory,
            .PtrElements = elementCount,
            .isArray = elementCount > 1,
            .File = file,
            .Line = std::to_string(line),
            .Function = func,
            .Notes = notes
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
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole != INVALID_HANDLE_VALUE) 
            {
                CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
                GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
                WORD originalAttributes = consoleInfo.wAttributes;

                size_t pos = ptr->File.find_last_of("\\/");
                String filename = (pos == String::npos) ? ptr->File : ptr->File.substr(pos + 1);

                SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                std::cout << "Error: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << "Memory Leak at: ";
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << "File: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << filename;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " Line: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << ptr->Line;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " Function: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << ptr->Function;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                std::cout << " Notes: ";
                SetConsoleTextAttribute(hConsole, originalAttributes);
                std::cout << ptr->Notes << std::endl;
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