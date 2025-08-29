#pragma once
extern "C"
{
#include "CTypedef.h"
}

#include <ctype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <nlohmann/json.hpp>
#include <vector>
#include "VkGuid.h"

typedef glm::vec1 vec1;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::ivec1 ivec1;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef std::string String;

typedef uint8_t MemoryAddress;

template <typename T, typename P> using Map = std::map<T, P>;
template <typename T, typename P> using UnorderedMap = std::unordered_map<T, P>;
template <typename T> using Vector = std::vector<T>;
template <typename T> using Span = std::span<T>;
template <typename T> using SharedPtr = std::shared_ptr<T>;
template <typename T> using UniquePtr = std::unique_ptr<T>;
template <typename T> using WeakPtr = std::weak_ptr<T>;

typedef VkGuid RenderPassGuid;
typedef VkGuid TextureGuid;
