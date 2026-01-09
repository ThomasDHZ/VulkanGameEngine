//#pragma once
//#include "Platform.h"
//#include "ShaderSystem.h"
//
//struct ShaderStructMemoryPool
//{
//	uint32 ShaderStructId = MAXUINT32;
//	uint32 ShaderStructData = MAXUINT32;
//	uint32 BufferId = MAXUINT32;
//	size_t ObjectSize = 0;
//	uint32 ObjectCount = 0;
//};
//
//template <typename T>
//size_t GetMemberOffset(const String& name)
//{
//	const auto& map = ReflectedMembers<T>::GetOffsetMap();
//	auto it = map.find(name);
//	if (it == map.end())
//	{
//		LogError("Unknown member '{}' in struct {}", name, typeid(T).name());
//		return 0;
//	}
//	return it->second;
//}
//
//#define REFLECT_MEMBER(structName, memberName) \
//    reflectMap[#memberName] = offsetof(structName, memberName);
//
//#define BEGIN_REFLECT(structName) \
//    template <> \
//    struct ReflectedMembers<structName> { \
//        static const UnorderedMap<String, size_t>& GetOffsetMap() { \
//            static UnorderedMap<String, size_t> reflectMap; \
//            if (reflectMap.empty()) {
//
//#define END_REFLECT() \
//            } \
//            return reflectMap; \
//        } \
//    };
//
//class MemoryPoolSystem
//{
//public:
//	static MemoryPoolSystem& Get();
//
//private:
//	MemoryPoolSystem() = default;
//	~MemoryPoolSystem() = default;
//	MemoryPoolSystem(const MemoryPoolSystem&) = delete;
//	MemoryPoolSystem& operator=(const MemoryPoolSystem&) = delete;
//	MemoryPoolSystem(MemoryPoolSystem&&) = delete;
//	MemoryPoolSystem& operator=(MemoryPoolSystem&&) = delete;
//
//	static constexpr uint32 FailedToFind = static_cast<uint32>(-1);
//	static constexpr byte  MemoryBlockUsed = 1;
//	static constexpr byte  FreeMemoryBlock = 0;
//
//	template <typename T>
//	uint32 FindNextFreeMemoryBlockIndex(ShaderStructMemoryPool<T>& memoryPool)
//	{
//		auto itr = std::find(memoryPool.MemoryBlockInUse.begin(), memoryPool.MemoryBlockInUse.end(), FreeMemoryBlock);
//		if (itr != memoryPool.MemoryBlockInUse.end())
//		{
//			return static_cast<uint32>(std::distance(memoryPool.MemoryBlockInUse.begin(), itr));
//		}
//		return FailedToFind;
//	}
//
//	template <typename T>
//	void ValidateStructLayout(const ShaderStructDLL& shaderStruct)
//	{
//		size_t currentOffset = 0;
//		for (auto& variable : shaderStruct.ShaderBufferVariableList)
//		{
//			size_t alignment = variable.ByteAlignment;
//			currentOffset = (currentOffset + alignment - 1) & ~(alignment - 1);
//
//			size_t cppOffset = GetMemberOffset<T>(variable.Name);
//			if (cppOffset != currentOffset)
//			{
//				LogError("Offset mismatch in struct '{}': member '{}' -> C++ offset {}, shader expects {}",
//					shaderStruct.Name, var.Name, cppOffset, currentOffset);
//			}
//
//			if (variable.Size != GetMemberSize<T>(variable.Name))
//			{
//				LogError("Size mismatch for member '{}.{}': C++ {} bytes, shader {} bytes",
//					shaderStruct.Name, variable.Name, GetMemberSize<T>(variable.Name), variable.Size);
//			}
//			currentOffset += var.Size;
//		}
//
//		if (currentOffset != shaderStruct.ShaderBufferSize)
//		{
//			LogWarning("Shader struct '{}' total size computation drift ({} vs {})",
//				shaderStruct.Name, currentOffset, shaderStruct.ShaderBufferSize);
//		}
//	}
//public:
//
//	Vector<ShaderStructMemoryPool>          PipelineShaderStructMemoryPoolMap;
//	Vector<Vector<byte>>					MemoryBlockPtr;
//	Vector<Vector<byte>>					MemoryBlockInUse;
//
//	template <typename T>
//	void CreateMemoryPool(const String& shaderStructName, uint32 objectCount)
//	{
//		PipelineShaderStructMemoryPoolMap.emplace_back(ShaderStructMemoryPool<T>
//		{
//			.ObjectSize = sizeof(T),
//			.ObjectCount = objectCount,
//			.MemoryBlockPtr = Vector<byte>(sizeof(T) * objectCount, 0x00),
//			.MemoryBlockInUse = Vector<byte>(objectCount, 0x00)
//		});
//	}
//
//	template <typename T>
//	void AddObject(ShaderStructMemoryPool<T>& memoryPool, T& shaderStruct)
//	{
//		int memoryIndex = FindNextFreeMemoryBlockIndex(memoryPool);
//		memoryPool.MemoryBlockInUse[memoryIndex] = FreeMemoryBlock;
//	}
//
//	template <typename T>
//	void UpdateObject(ShaderStructMemoryPool& memoryPool, T& shaderStruct)
//	{
//		memoryPool.
//	}
//
//	void DestroyObject(uint32 memoryIndex)
//	{
//		if (memoryIndex < ObjectCount && MemoryBlockInUse[memoryIndex])
//		{
//			reinterpret_cast<T*>(MemoryBlockPtr + (memoryIndex * ObjectSize))->Destroy();
//			MemoryBlockInUse[memoryIndex] = FreeMemoryBlock;
//		}
//	}
//
//	void Destroy()
//	{
//		for (uint32 x = 0; x < MemoryBlockInUse.size(); x++)
//		{
//			if (MemoryBlockInUse[x]) {
//				reinterpret_cast<T*>(MemoryBlockPtr + (x * ObjectSize))->Destroy();
//			}
//		}
//
//		if (MemoryBlockPtr)
//		{
//			delete[] reinterpret_cast<std::aligned_storage_t<sizeof(T), alignof(T)>*>(MemoryBlockPtr);
//			MemoryBlockPtr = nullptr;
//		}
//
//		MemoryBlockInUse.clear();
//	}
//
//	Vector<T*> ViewMemoryPool()
//	{
//		Vector<T*> memoryList;
//		for (uint32 x = 0; x < ObjectCount; x++)
//		{
//			T* ptr = reinterpret_cast<T*>(MemoryBlockPtr + (x * ObjectSize));
//			memoryList.emplace_back(ptr);
//		}
//		return memoryList;
//	}
//
//	void ViewMemoryMap()
//	{
//		const auto memory = ViewMemoryPool();
//		std::cout << "Memory Map of the " << typeid(T).name() << " Memory Pool(" << sizeof(T) << " bytes each, "
//			<< std::to_string(sizeof(T) * memory.size()) << " bytes total) : " << std::endl;
//		std::cout << std::setw(20) << "Address" << std::setw(15) << "Value" << std::endl;
//
//		for (size_t x = 0; x < memory.size(); x++)
//		{
//			std::cout << std::hex << "0x" << reinterpret_cast<void*>(memory[x]) << ": "
//				<< (MemoryBlockInUse[x] == MemoryBlockUsed ? memory[x]->Name : "nullptr") << std::endl;
//		}
//
//		std::cout << std::endl << std::endl;
//	}
//};
//extern DLL_EXPORT MemoryPoolSystem& memoryPoolSystem;
//inline MemoryPoolSystem& MemoryPoolSystem::Get()
//{
//	static MemoryPoolSystem instance;
//	return instance;
//}