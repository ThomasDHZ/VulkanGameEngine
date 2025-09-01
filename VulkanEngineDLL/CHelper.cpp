#include "CHelper.h"
#include "MemorySystem.h"

const char** CHelper_VectorToConstCharPtrPtr(const Vector<String>& vec)
{
	const char** stringList = memorySystem.AddPtrBuffer<const char*>(vec.size() + 1, __FILE__, __LINE__, __func__);
	for (size_t x = 0; x < vec.size(); ++x) 
	{
		stringList[x] = vec[x].c_str();
	}
	return stringList;
}

Vector<String> CHelper_ConstCharPtrPtrToVector(const char** stringList, size_t stringListCount)
{
	Vector<String> stringListVector;
	stringListVector.reserve(stringListCount);
	for (size_t x = 0; x < stringListCount; ++x) 
	{
		stringListVector.emplace_back(stringList[x]);
	}
	return stringListVector;
}

void CHelper_DestroyConstCharPtrPtr(const char** stringList)
{
	memorySystem.RemovePtrBuffer<const char*>(stringList);
}