#pragma once
#include "DLL.h"
#include "Typedef.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>

typedef struct fileState
{
	char* Data;
	size_t Size;
	bool Valid;
}FileState;

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT bool	  File_Exists(const char* fileName);
	DLL_EXPORT time_t    File_LastModifiedTime(const char* fileName);
	DLL_EXPORT char* File_RemoveFileExtention(const char* fileName);
	DLL_EXPORT char* File_GetFileExtention(const char* fileName);
	DLL_EXPORT char* File_GetFileNameFromPath(const char* fileName);
	DLL_EXPORT FileState File_Read(const char* path);
	DLL_EXPORT int       File_Write(void* buffer, size_t size, const char* path);

#ifdef __cplusplus
}
#endif

DLL_EXPORT nlohmann::json File_LoadJsonFile(const char* filePath); 
DLL_EXPORT const char** File_GetFilesFromDirectory(const char* fileDirectory, const char** fileExtensions, size_t fileExtenstionCount, size_t& returnFileCount);