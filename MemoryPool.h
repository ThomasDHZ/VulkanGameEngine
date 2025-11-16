#pragma once
extern "C"
{
#include <Macro.h>
}
#include <iostream>
#include "Typedef.h"
#include <limits>

template <class T>
class MemoryPool
{
private:
	static constexpr uint32 FailedToFind = static_cast<uint32>(-1);
	static constexpr uint8  MemoryBlockUsed = 1;
	static constexpr uint8  FreeMemoryBlock = 0;

	uint8* MemoryBlockPtr = nullptr;
	size_t ObjectSize = sizeof(T);
	uint32 ObjectCount = 0;
	Vector<uint8> MemoryBlockInUse;

	uint32 FindNextFreeMemoryBlockIndex()
	{
		auto itr = std::find(MemoryBlockInUse.begin(), MemoryBlockInUse.end(), FreeMemoryBlock);
		if (itr != MemoryBlockInUse.end()) 
		{
			return static_cast<uint32>(std::distance(MemoryBlockInUse.begin(), itr));
		}
		return FailedToFind;
	}

	//void UpdateMemoryPoolSize(List<SharedPtr<T>>& memoryPoolList)
	//{
	//	uint32 newObjectCount =  ObjectCount * 2;
	//	MemoryBlock* tempMemoryBlock = new MemoryBlock[ObjectSize * newObjectCount];
	//	memcpy(tempMemoryBlock, MemoryBlockPtr, ObjectSize * ObjectCount);

	//	for (int x = 0; x < ObjectCount; x++)
	//	{
	//		memoryPoolList[x].reset();

	//		uint8_t* address = reinterpret_cast<uint8_t*>(MemoryBlockPtr) + (x * sizeof(T));
	//		T* ptr = reinterpret_cast<T*>(address);
	//		memoryPoolList[x] = SharedPtr<T>(newObject, [this, memoryIndex](T* ptr)
	//			{
	//				ptr->~T();
	//				MemoryBlockInUse[memoryIndex] = false;
	//			})
	//	}
	//	delete MemoryBlockPtr;

	//	ObjectCount = newObjectCount;
	//	MemoryBlockPtr = tempMemoryBlock;
	//}

public:

	MemoryPool()
	{

	}

	~MemoryPool()
	{
	}

	void CreateMemoryPool(uint32 objectCount) 
	{
		ObjectCount = objectCount;
		MemoryBlockInUse.resize(objectCount, FreeMemoryBlock);
		MemoryBlockPtr = reinterpret_cast<uint8_t*>(new std::aligned_storage_t<sizeof(T), alignof(T)>[objectCount]);
	}

	uint32 AllocateMemoryLocation() 
	{
		uint32 memoryIndex = FindNextFreeMemoryBlockIndex();
		if (memoryIndex == FailedToFind) 
		{
			throw std::runtime_error("No free memory block available.");
		}
		new (MemoryBlockPtr + (memoryIndex * ObjectSize)) T();
		MemoryBlockInUse[memoryIndex] = MemoryBlockUsed;
		return memoryIndex;
	}

	T& GetObjectMemory(uint32 memoryIndex)
	{
		if (memoryIndex >= ObjectCount || !MemoryBlockInUse[memoryIndex])
		{
			throw std::out_of_range("Invalid or unused memory index.");
		}
		return *reinterpret_cast<T*>(MemoryBlockPtr + (memoryIndex * ObjectSize));
	}

	void DestroyObject(uint32 memoryIndex) 
	{
		if (memoryIndex < ObjectCount && MemoryBlockInUse[memoryIndex]) 
		{
			reinterpret_cast<T*>(MemoryBlockPtr + (memoryIndex * ObjectSize))->Destroy();
			MemoryBlockInUse[memoryIndex] = FreeMemoryBlock;
		}
	}

	void Destroy() 
	{
		for (uint32 x = 0; x < MemoryBlockInUse.size(); x++) 
		{
			if (MemoryBlockInUse[x]) {
				reinterpret_cast<T*>(MemoryBlockPtr + (x * ObjectSize))->Destroy();
			}
		}

		if (MemoryBlockPtr) 
		{
			delete[] reinterpret_cast<std::aligned_storage_t<sizeof(T), alignof(T)>*>(MemoryBlockPtr);
			MemoryBlockPtr = nullptr;
		}

		MemoryBlockInUse.clear();
	}

	Vector<T*> ViewMemoryPool()
	{
		Vector<T*> memoryList;
		for (uint32 x = 0; x < ObjectCount; x++)
		{
			T* ptr = reinterpret_cast<T*>(MemoryBlockPtr + (x * ObjectSize));
			memoryList.emplace_back(ptr);
		}
		return memoryList;
	}

	void ViewMemoryMap()
	{
		const auto memory = ViewMemoryPool();
		std::cout << "Memory Map of the " << typeid(T).name() << " Memory Pool(" << sizeof(T) << " bytes each, "
			<< std::to_string(sizeof(T) * memory.size()) << " bytes total) : " << std::endl;
		std::cout << std::setw(20) << "Address" << std::setw(15) << "Value" << std::endl;

		for (size_t x = 0; x < memory.size(); x++)
		{
			std::cout << std::hex << "0x" << reinterpret_cast<void*>(memory[x]) << ": "
				<< (MemoryBlockInUse[x] == MemoryBlockUsed ? memory[x]->Name : "nullptr") << std::endl;
		}

		std::cout << std::endl << std::endl;
	}
};
