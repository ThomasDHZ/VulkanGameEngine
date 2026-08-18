#pragma once
#include <MemorySystem.h>
class DllHelper
{
public:
	template<typename T>
	static T ExtractDllFunction(void* ptr)
	{
		if (!ptr) return T();
		T* funtionPtr = static_cast<T*>(ptr);
		T result = std::move(*funtionPtr);

		//ptr->~T();
		memorySystem.DeletePtr(ptr);
		return result;
	}
};

