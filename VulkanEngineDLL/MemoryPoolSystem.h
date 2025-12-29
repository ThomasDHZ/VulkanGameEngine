#pragma once
#include "Platform.h"
#include "ShaderSystem.h"

struct ShaderStructMemoryPool
{
	Vector<byte> MemoryBlockPtr;
	size_t ObjectSize = 0;
	uint32 ObjectCount = 0;
	Vector<uint8> MemoryBlockInUse;
};

class MemoryPoolSystem
{
public:
	static MemoryPoolSystem& Get();

private:
	MemoryPoolSystem() = default;
	~MemoryPoolSystem() = default;
	MemoryPoolSystem(const MemoryPoolSystem&) = delete;
	MemoryPoolSystem& operator=(const MemoryPoolSystem&) = delete;
	MemoryPoolSystem(MemoryPoolSystem&&) = delete;
	MemoryPoolSystem& operator=(MemoryPoolSystem&&) = delete;

	static constexpr uint32 FailedToFind = static_cast<uint32>(-1);
	static constexpr uint8  MemoryBlockUsed = 1;
	static constexpr uint8  FreeMemoryBlock = 0;

	uint32 FindNextFreeMemoryBlockIndex(ShaderStructMemoryPool& memoryPool)
	{
		auto itr = std::find(memoryPool.MemoryBlockInUse.begin(), memoryPool.MemoryBlockInUse.end(), FreeMemoryBlock);
		if (itr != memoryPool.MemoryBlockInUse.end())
		{
			return static_cast<uint32>(std::distance(memoryPool.MemoryBlockInUse.begin(), itr));
		}
		return FailedToFind;
	}

public:
	//void CreateMemoryPool(const String& shaderStructName, uint32 objectCount)
	//{
	//	ShaderStructDLL shaderStruct = shaderSystem.FindShaderProtoTypeStruct(shaderStructName);
	//	
	//	ShaderStructMemoryPool memoryPool;
	//	memoryPool.ObjectCount = objectCount;
	//	memoryPool.MemoryBlockInUse.resize(objectCount, FreeMemoryBlock);
	//	//memoryPool.MemoryBlockPtr = reinterpret_cast<uint8_t*>(new std::aligned_storage_t<sizeof(shaderStruct.ShaderBufferSize), alignof(shaderStruct.ShaderBufferSize)>[objectCount]);
	//}

	//uint32 AllocateMemoryLocation(ShaderStructMemoryPool& memoryPool)
	//{
	//	uint32 memoryIndex = FindNextFreeMemoryBlockIndex(memoryPool);
	//	if (memoryIndex == FailedToFind)
	//	{
	//		throw std::runtime_error("No free memory block available.");
	//	}
	//	new (memoryPool.MemoryBlockPtr + (memoryIndex * memoryPool.ObjectSize)) T();
	//	memoryPool.MemoryBlockInUse[memoryIndex] = MemoryBlockUsed;
	//	return memoryIndex;
	//}

	///*T& GetObjectMemory(ShaderStructMemoryPool& memoryPool, uint32 memoryIndex)
	//{
	//	if (memoryIndex >= memoryPool.ObjectCount || !memoryPool.MemoryBlockInUse[memoryIndex])
	//	{
	//		throw std::out_of_range("Invalid or unused memory index.");
	//	}
	//	return *reinterpret_cast<T*>(memoryPool.MemoryBlockPtr + (memoryIndex * memoryPool.ObjectSize));
	//}*/

	////void DestroyObject(ShaderStructMemoryPool& memoryPool, uint32 memoryIndex)
	////{
	////	if (memoryIndex < memoryPool.ObjectCount && memoryPool.MemoryBlockInUse[memoryIndex])
	////	{
	////		reinterpret_cast<T*>(memoryPool.MemoryBlockPtr + (memoryIndex * memoryPool.ObjectSize))->Destroy();
	////		memoryPool.MemoryBlockInUse[memoryIndex] = FreeMemoryBlock;
	////	}
	////}

	//void Destroy(ShaderStructMemoryPool& memoryPool)
	//{
	///*	for (uint32 x = 0; x < memoryPool.MemoryBlockInUse.size(); x++)
	//	{
	//		if (memoryPool.MemoryBlockInUse[x]) {
	//			reinterpret_cast<T*>(memoryPool.MemoryBlockPtr + (x * memoryPool.ObjectSize))->Destroy();
	//		}
	//	}*/

	///*	if (memoryPool.MemoryBlockPtr)
	//	{
	//		delete[] reinterpret_cast<std::aligned_storage_t<sizeof(T), alignof(T)>*>(memoryPool.MemoryBlockPtr);
	//		memoryPool.MemoryBlockPtr = nullptr;
	//	}*/

	//	memoryPool.MemoryBlockInUse.clear();
	//}

	////Vector<T*> ViewMemoryPool(ShaderStructMemoryPool& memoryPool)
	////{
	////	Vector<T*> memoryList;
	////	for (uint32 x = 0; x < memoryPool.ObjectCount; x++)
	////	{
	////		T* ptr = reinterpret_cast<T*>(memoryPool.MemoryBlockPtr + (x * memoryPool.ObjectSize));
	////		memoryList.emplace_back(ptr);
	////	}
	////	return memoryList;
	////}

	////void ViewMemoryMap(ShaderStructMemoryPool& memoryPool)
	////{
	////	const auto memory = ViewMemoryPool(memoryPool);
	////	std::cout << "Memory Map of the " << typeid(T).name() << " Memory Pool(" << sizeof(T) << " bytes each, "
	////		<< std::to_string(sizeof(T) * memory.size()) << " bytes total) : " << std::endl;
	////	std::cout << std::setw(20) << "Address" << std::setw(15) << "Value" << std::endl;

	////	for (size_t x = 0; x < memory.size(); x++)
	////	{
	////		std::cout << std::hex << "0x" << reinterpret_cast<void*>(memory[x]) << ": "
	////			<< (memoryPool.MemoryBlockInUse[x] == MemoryBlockUsed ? memory[x]->Name : "nullptr") << std::endl;
	////	}

	////	std::cout << std::endl << std::endl;
	////}
};
extern DLL_EXPORT MemoryPoolSystem& memoryPoolSystem;
inline MemoryPoolSystem& MemoryPoolSystem::Get()
{
	static MemoryPoolSystem instance;
	return instance;
}