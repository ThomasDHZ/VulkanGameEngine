#include "File.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "Macro.h"
#include "File.h"
#include "MemorySystem.h"

const char** File_GetFilesFromDirectory(const char* fileDirectory, const char** fileExtensions, size_t fileExtensionCount, size_t& returnFileCount)
{
    Vector<const char*> fileList;
    String fileDirectoryString(fileDirectory);
    Vector<String> fileExtensionList(fileExtensions, fileExtensions + fileExtensionCount);

    try
    {
        if (std::filesystem::exists(fileDirectory) && std::filesystem::is_directory(fileDirectory))
        {
            for (const auto& entry : std::filesystem::directory_iterator(fileDirectory))
            {
                if (entry.is_regular_file())
                {
                    auto ext = entry.path().extension().string();
                    if (!ext.empty() && ext.front() == '.')
                        ext.erase(0, 1);

                    for (const auto& allowedExt : fileExtensionList)
                    {
                        if (ext == allowedExt)
                        {
                            const char* pathStr = memorySystem.AddPtrBuffer(entry.path().string().c_str(), __FILE__, __LINE__, __func__, entry.path().string().c_str());
                            fileList.push_back(pathStr);
                            break;
                        }
                    }
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        std::cerr << "Filesystem error: " << ex.what() << std::endl;
    }

    returnFileCount = fileList.size();
    return memorySystem.AddPtrBuffer<const char*>(fileList.data(), fileList.size(), __FILE__, __LINE__, __func__, "Directory List String");;
}