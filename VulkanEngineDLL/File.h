#pragma once
#include "DLL.h"
#include <filesystem>
#include "Typedef.h"

DLL_EXPORT const char** File_GetFilesFromDirectory(const char* fileDirectory, const char** fileExtensions, size_t fileExtenstionCount, size_t& returnFileCount);
