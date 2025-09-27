#pragma once
#include "DLL.h"
#include "Typedef.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>

DLL_EXPORT const char** File_GetFilesFromDirectory(const char* fileDirectory, const char** fileExtensions, size_t fileExtenstionCount, size_t& returnFileCount);