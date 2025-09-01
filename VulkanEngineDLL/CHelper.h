#pragma once
#include "DLL.h"
#include "Typedef.h"

DLL_EXPORT const char** CHelper_VectorToConstCharPtrPtr(const Vector<String>& vec);
DLL_EXPORT Vector<String> CHelper_ConstCharPtrPtrToVector(const char** stringList, size_t stringListCount);
DLL_EXPORT void CHelper_DestroyConstCharPtrPtr(const char** stringList);