#include "VulkanFileSystem.h"
#include <MemorySystem.h>
#include <FileSystem.h>

VulkanFileSystem vulkanFileSystem = VulkanFileSystem();

VulkanFileSystem::VulkanFileSystem()
{
}

VulkanFileSystem::~VulkanFileSystem()
{
}

const char* VulkanFileSystem::ReadFile(const String& filePath)
{
    return File_Read(filePath.c_str()).Data;
}

nlohmann::json VulkanFileSystem::LoadJsonFile(const String& filePath)
{
    return File_LoadJsonFile(filePath.c_str());
}

bool VulkanFileSystem::WriteFile(void* fileInfo, size_t size, const String& filePath)
{
    return File_Write(fileInfo, size, filePath.c_str());
}

String VulkanFileSystem::GetFileExtention(const String& filePath)
{
    return File_GetFileExtention(filePath.c_str());
}

String VulkanFileSystem::GetFileNameFromPath(const String& filePath)
{
    return File_GetFileNameFromPath(filePath.c_str());
}

Vector<String> VulkanFileSystem::GetFilesFromDirectory(const String& fileDirectory, Vector<const char*> fileExtenstionList)
{
   // size_t returnFileCount = 0;
    //size_t extenstionListCount = fileExtenstionList.size();
   // const char** extenstionList = memorySystem.AddPtrBuffer<const char*>(fileExtenstionList.data(), fileExtenstionList.size(), __FILE__, __LINE__, __func__, "Directory List String");
    //const char** fileList = fileSystem.GetFilesFromDirectory(fileDirectory.c_str(), extenstionList, extenstionListCount, returnFileCount);
    return Vector<String>();
}

time_t VulkanFileSystem::LastModifiedTime(const String& filePath)
{
    return File_LastModifiedTime(filePath.c_str());
}

String VulkanFileSystem::RemoveFileExtention(const String& filePath)
{
    return File_RemoveFileExtention(filePath.c_str());
}

bool VulkanFileSystem::FileExists(const String& filePath)
{
    return File_Exists(filePath.c_str());
}
