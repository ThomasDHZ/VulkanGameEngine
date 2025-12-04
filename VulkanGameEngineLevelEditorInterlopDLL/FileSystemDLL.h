#pragma once
#include <FileSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT bool		   File_Exists(const char* fileName);
	DLL_EXPORT time_t      File_LastModifiedTime(const char* fileName);
	DLL_EXPORT const char* File_RemoveFileExtention(const char* fileName);
	DLL_EXPORT const char* File_GetFileExtention(const char* fileName);
	DLL_EXPORT const char* File_GetFileNameFromPath(const char* fileName);
	DLL_EXPORT const char* File_Read(const char* path);
	DLL_EXPORT int		   File_Write(void* buffer, size_t size, const char* path);
#ifdef __cplusplus
}
#endif