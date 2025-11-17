#pragma once
#include <File.h>
#include "Typedef.h"
#include <nlohmann/json.hpp>

class VulkanFileSystem
{
private:
public:
	VulkanFileSystem();
	~VulkanFileSystem();

	const char* ReadFile(const String& filePath);
	nlohmann::json LoadJsonFile(const String& filePath);
	bool   WriteFile(void* fileInfo, size_t size, const String& filePath);
	String GetFileExtention(const String& filePath);
	String GetFileNameFromPath(const String& filePath);
	Vector<String> GetFilesFromDirectory(const String& fileDirectory, Vector<const char*> fileExtenstionList);
	time_t LastModifiedTime(const String& filePath);
	String RemoveFileExtention(const String& filePath);
	bool FileExists(const String& filePath);

};
extern VulkanFileSystem vulkanFileSystem;

