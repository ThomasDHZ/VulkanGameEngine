#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "DLL.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct fileState
	{
		char* Data;
		size_t Size;
		bool Valid;
	}FileState;

	DLL_EXPORT bool	  File_Exists(const char* fileName);
	DLL_EXPORT time_t    File_LastModifiedTime(const char* fileName);
	DLL_EXPORT char*	  File_RemoveFileExtention(const char* fileName);
	DLL_EXPORT char*	  File_GetFileExtention(const char* fileName);
	DLL_EXPORT char*	  File_GetFileNameFromPath(const char* fileName);
	DLL_EXPORT FileState File_Read(const char* path);
	DLL_EXPORT int       File_Write(void* buffer, size_t size, const char* path);

#ifdef __cplusplus
}
#endif
