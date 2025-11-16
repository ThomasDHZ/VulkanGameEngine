#pragma once
#include <inttypes.h>
#include <ctype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include "../External/nlohmann/include/nlohmann/json.hpp"
#include <vector>
#include "VkGuid.h"

typedef uint32_t uint;
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
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

typedef unsigned char byte;
typedef unsigned char MemoryBlock;
typedef uint8_t MemoryAddress;

template <typename T, typename P> using Map = std::map<T, P>;
template <typename T, typename P> using UnorderedMap = std::unordered_map<T, P>;
template <typename T> using Vector = std::vector<T>;
template <typename T> using Span = std::span<T>;
template <typename T> using SharedPtr = std::shared_ptr<T>;
template <typename T> using UniquePtr = std::unique_ptr<T>;
template <typename T> using WeakPtr = std::weak_ptr<T>;

typedef VkGuid LevelGuid;
typedef VkGuid TextureGuid;
typedef VkGuid VramSpriteGuid;
typedef VkGuid RenderPassGuid;
typedef VkGuid MaterialGuid;

typedef uint SpriteId;
typedef uint SpriteBatchId;
typedef uint RenderPassId;
typedef uint RenderPipelineId;
typedef uint LevelId;
typedef uint AnimationListId;

