#pragma once

#include <Platform.h>
#include "MemorySystem.h"
#include <ktx/include/ktx.h>
#include <ktx/include/ktxvulkan.h>

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
	const char*		 ReadFile(const String& filePath);
	Vector<byte>	 LoadAssetFile(const String& filePath);
	bool			 WriteFile(void* fileInfo, size_t size, const String& filePath);
	String			 GetFileExtention(const char* fileName);
	String			 GetFileNameFromPath(const String& filePath);
	time_t			 LastModifiedTime(const String& filePath);
	String			 RemoveFileExtention(const String& filePath);
	bool			 FileExists(const String& filePath);
	nlohmann::json	 LoadConfig(const String& configPath);
	nlohmann::json	 LoadJsonFile(const String& filePath);
	Vector<byte>	 LoadPNG(const String& filePath, uint& width, uint& height, uint& bitsPerChannel, int& channelCount);
	Vector<byte>	 LoadImageFile(const String& filePath, int& width, int& height, int& channelCount);
	ktxVulkanTexture LoadKTX2File(const String& filePath);
	// void			 ExportTexture(VkGuid& renderPassId, const String& filePath);
	String			 File_GetFileExtention(const char* fileName);
	Vector<String>	 GetFilesFromDirectory(const String& fileDirectory);
	Vector<String>	 GetFilesFromDirectory(const String& fileDirectory, const Vector<String>& fileExtensionList);

	template<typename T>
	T LoadJsonFile(const String& filePath)
	{
		nlohmann::json json = LoadJsonFile(filePath);
		return json.get<T>();
	}

#if defined(__ANDROID__)
	void LoadAndroidAssetManager(AAssetManager* androidAssetManager);
#endif
};
extern  FileSystem& fileSystem;
inline FileSystem& FileSystem::Get()
{
	static FileSystem instance;
	return instance;
}

