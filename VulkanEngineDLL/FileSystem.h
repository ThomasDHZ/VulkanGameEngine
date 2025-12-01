#pragma once
#include "Platform.h"
#include "MemorySystem.h"
#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

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

class FileSystem
{
private:
#if defined(__ANDROID__)
	AAssetManager* g_AssetManager = nullptr;
#endif

public:
	FileSystem() {};
	~FileSystem() {};

#if defined(__ANDROID__)
	void LoadAndroidAssetManager(AAssetManager* androidAssetManager);
#endif
	DLL_EXPORT const char* ReadFile(const String& filePath) { return File_Read(filePath.c_str()).Data; }
	DLL_EXPORT Vector<byte> LoadAssetFile(const String& filePath);
	DLL_EXPORT bool WriteFile(void* fileInfo, size_t size, const String& filePath) { return File_Write(fileInfo, size, filePath.c_str()); }
	DLL_EXPORT String GetFileExtention(const char* fileName);
	DLL_EXPORT String GetFileNameFromPath(const String& filePath) { return File_GetFileNameFromPath(filePath.c_str()); }
	DLL_EXPORT time_t LastModifiedTime(const String& filePath) { return File_LastModifiedTime(filePath.c_str()); }
	DLL_EXPORT String RemoveFileExtention(const String& filePath) { return File_RemoveFileExtention(filePath.c_str()); }
	DLL_EXPORT bool FileExists(const String& filePath) { return File_Exists(filePath.c_str()); }
	DLL_EXPORT nlohmann::json LoadJsonFile(const String& filePath);
	DLL_EXPORT Vector<String> GetFilesFromDirectory(const String& fileDirectory);
	DLL_EXPORT Vector<String> GetFilesFromDirectory(const String& fileDirectory, const Vector<String>& fileExtensionList);
};
extern DLL_EXPORT FileSystem fileSystem;

