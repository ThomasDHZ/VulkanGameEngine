#include "pch.h"
#include "FileSystemDLL.h"

bool File_Exists(const char* fileName)
{
	return fileSystem.FileExists(fileName);
}

time_t File_LastModifiedTime(const char* fileName)
{
	return fileSystem.LastModifiedTime(fileName);
}

const char* File_RemoveFileExtention(const char* fileName)
{
	return fileSystem.RemoveFileExtention(fileName).c_str();
}

const char* File_GetFileExtention(const char* fileName)
{
	return fileSystem.GetFileExtention(fileName).c_str();
}

const char* File_GetFileNameFromPath(const char* filePath)
{
	return 	fileSystem.GetFileNameFromPath(filePath).c_str();
}

const char* File_Read(const char* path)
{
	return 	fileSystem.ReadFile(path);
}

int File_Write(void* buffer, size_t size, const char* path)
{
	return 	fileSystem.WriteFile(buffer, size, path);
}