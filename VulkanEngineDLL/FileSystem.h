#pragma once
#include "DLL.h"
#include "Typedef.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "MemorySystem.h"

typedef struct fileState
{
	char* Data;
	size_t Size;
	bool Valid;
}FileState;


#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT bool		 File_Exists(const char* fileName);
	DLL_EXPORT time_t    File_LastModifiedTime(const char* fileName);
	DLL_EXPORT char*	 File_RemoveFileExtention(const char* fileName);
	DLL_EXPORT char*	 File_GetFileExtention(const char* fileName);
	DLL_EXPORT char*	 File_GetFileNameFromPath(const char* fileName);
	DLL_EXPORT FileState File_Read(const char* path);
	DLL_EXPORT int       File_Write(void* buffer, size_t size, const char* path);

#ifdef __cplusplus
}
#endif

DLL_EXPORT nlohmann::json File_LoadJsonFile(const char* filePath); 
DLL_EXPORT const char** File_GetFilesFromDirectory(const char* fileDirectory, const char** fileExtensions, size_t fileExtenstionCount, size_t& returnFileCount);

class FileSystem
{
private:
public:
	FileSystem() {};
	~FileSystem() {};

	const char* ReadFile(const String& filePath) { return File_Read(filePath.c_str()).Data; }
	bool WriteFile(void* fileInfo, size_t size, const String& filePath) { return File_Write(fileInfo, size, filePath.c_str()); }
	String GetFileExtention(const String& filePath) { return File_GetFileExtention(filePath.c_str()); }
	String GetFileNameFromPath(const String& filePath) { return File_GetFileNameFromPath(filePath.c_str()); }
	time_t LastModifiedTime(const String& filePath) { return File_LastModifiedTime(filePath.c_str()); }
	String RemoveFileExtention(const String& filePath) { return File_RemoveFileExtention(filePath.c_str()); }
	bool FileExists(const String& filePath) { return File_Exists(filePath.c_str()); }
	nlohmann::json LoadJsonFile(const String& filePath) { return File_LoadJsonFile(filePath.c_str()); }
	Vector<String> GetFilesFromDirectory(const String& fileDirectory, Vector<const char*> fileExtenstionList) 
	{
		size_t returnFileCount = 0;
		size_t extenstionListCount = fileExtenstionList.size();
		const char** extenstionList = memorySystem.AddPtrBuffer<const char*>(fileExtenstionList.data(), fileExtenstionList.size(), __FILE__, __LINE__, __func__, "Directory List String");
		const char** fileList = File_GetFilesFromDirectory(fileDirectory.c_str(), extenstionList, extenstionListCount, returnFileCount);
		return Vector<String>(fileList, fileList + returnFileCount);
	}
};
DLL_EXPORT FileSystem fileSystem;

