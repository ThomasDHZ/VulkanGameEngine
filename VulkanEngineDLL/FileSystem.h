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

class FileSystem
{
public:
	static FileSystem& Get();

private:
	FileSystem() = default;
	~FileSystem() = default;
	FileSystem(const FileSystem&) = delete;
	FileSystem& operator=(const FileSystem&) = delete;
	FileSystem(FileSystem&&) = delete;
	FileSystem& operator=(FileSystem&&) = delete;

#if defined(__ANDROID__)
    static inline AAssetManager* g_AssetManager = nullptr;
#endif

public:
	DLL_EXPORT const char*	  ReadFile(const String& filePath);
	DLL_EXPORT Vector<byte>	  LoadAssetFile(const String& filePath);
	DLL_EXPORT bool			  WriteFile(void* fileInfo, size_t size, const String& filePath);
	DLL_EXPORT String		  GetFileExtention(const char* fileName);
	DLL_EXPORT String		  GetFileNameFromPath(const String& filePath);
	DLL_EXPORT time_t		  LastModifiedTime(const String& filePath);
	DLL_EXPORT String		  RemoveFileExtention(const String& filePath);
	DLL_EXPORT bool			  FileExists(const String& filePath);
    DLL_EXPORT nlohmann::json LoadConfig(const String& configPath);
	DLL_EXPORT nlohmann::json LoadJsonFile(const String& filePath);
	DLL_EXPORT Vector<byte>   LoadPNG(const String& filePath, uint& width, uint& height, uint& bitsPerChannel, int& channelCount);
    DLL_EXPORT Vector<byte>   LoadImageFile(const String& filePath, int& width, int& height, int& channelCount);
	DLL_EXPORT void			  ExportTexture(VkGuid& renderPassId, const String& filePath);
	DLL_EXPORT String		  File_GetFileExtention(const char* fileName);
	DLL_EXPORT Vector<String> GetFilesFromDirectory(const String& fileDirectory);
	DLL_EXPORT Vector<String> GetFilesFromDirectory(const String& fileDirectory, const Vector<String>& fileExtensionList);

#if defined(__ANDROID__)
	DLL_EXPORT void LoadAndroidAssetManager(AAssetManager* androidAssetManager);
#endif
};
extern DLL_EXPORT FileSystem& fileSystem;
inline FileSystem& FileSystem::Get()
{
	static FileSystem instance;
	return instance;
}

