#pragma once
#include <vulkan/vulkan_core.h>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include "Typedef.h"
#include "File.h"

class Json : public nlohmann::json
{
public:
	static nlohmann::json ReadJson(const String filePath)
	{
		return File::ReadJsonFile(filePath);
	}

	static void to_json(nlohmann::json& json, String& string)
	{
		json = string;
	}
	static void from_json(nlohmann::json& json, String& string)
	{
		json.get_to(string);
	}
	static void to_json(nlohmann::json& json, std::vector<String>& string)
	{
		json = string;
	}
	static void from_json(nlohmann::json& json, std::vector<String>& string)
	{
		/*	for (auto& vec2 : numberList)
			{
				json[0].get_to(vec2.x);
				json[1].get_to(vec2.y);
			}*/
	}

	static void to_json(nlohmann::json& json, int& number)
	{
		json = number;
	}
	static void from_json(nlohmann::json& json, int& number)
	{
		json.get_to(number);
	}
	static void to_json(nlohmann::json& json, std::vector<int>& numberList)
	{
		json = numberList;
	}
	static void from_json(nlohmann::json& json, std::vector<int>& numberList)
	{
		/*	for (auto& vec2 : numberList)
			{
				json[0].get_to(vec2.x);
				json[1].get_to(vec2.y);
			}*/
	}


	static void to_json(nlohmann::json& json, uint32_t& number)
	{
		json = number;
	}
	static void from_json(nlohmann::json& json, uint32_t& number)
	{
		json.get_to(number);
	}
	static void to_json(nlohmann::json& json, std::vector<uint32_t>& numberList)
	{
		json = numberList;
	}
	static void from_json(nlohmann::json& json, std::vector<uint32_t>& numberList)
	{
		json.get_to(numberList);
	}

	static void to_json(nlohmann::json& json, float number)
	{
		json = number;
	}
	static void from_json(nlohmann::json& json, float number)
	{
		json.get_to(number);
	}

	static void to_json(nlohmann::json& json, std::vector<float>& numberList)
	{
		json = numberList;
	}
	static void from_json(nlohmann::json& json, std::vector<float>& numberList)
	{
		json.get_to(numberList);
	}

	static void to_json(nlohmann::json& json, glm::vec2& vec2)
	{
		json = { vec2.x, vec2.y };
	}
	static void from_json(nlohmann::json& json, glm::vec2& vec2)
	{
		json[0].get_to(vec2.x);
		json[1].get_to(vec2.y);
	}

	static void to_json(nlohmann::json& json, std::vector<glm::vec2>& vec2List)
	{
		for (auto& vec2 : vec2List)
		{
			json = { vec2.x, vec2.y };
		}
	}
	static void from_json(nlohmann::json& json, std::vector<glm::vec2>& vec2List)
	{
		for (auto& vec2 : vec2List)
		{
			json[0].get_to(vec2.x);
			json[1].get_to(vec2.y);
		}
	}

	static void to_json(nlohmann::json& json, glm::vec3& vec3)
	{
		json = { vec3.x, vec3.y, vec3.z };
	}
	static void from_json(nlohmann::json& json, glm::vec3& vec3)
	{
		json[0].get_to(vec3.x);
		json[1].get_to(vec3.y);
		json[2].get_to(vec3.z);
	}

	static void to_json(nlohmann::json& json, std::vector<glm::vec3>& vec3List)
	{
		for (auto& vec3 : vec3List)
		{
			json = { vec3.x, vec3.y, vec3.z };
		}
	}
	static void from_json(nlohmann::json& json, std::vector<glm::vec3>& vec3List)
	{
		for (auto& vec3 : vec3List)
		{
			json[0].get_to(vec3.x);
			json[1].get_to(vec3.y);
			json[2].get_to(vec3.z);
		}
	}

	static void to_json(nlohmann::json& json, glm::vec4& vec4)
	{
		json = { vec4.x, vec4.y, vec4.z, vec4.w };
	}
	static void from_json(nlohmann::json& json, glm::vec4& vec4)
	{
		json[0].get_to(vec4.x);
		json[1].get_to(vec4.y);
		json[2].get_to(vec4.z);
		json[3].get_to(vec4.w);
	}

	static void to_json(nlohmann::json& json, std::vector<glm::vec4>& vec4List)
	{
		for (auto& vec4 : vec4List)
		{
			json = { vec4.x, vec4.y, vec4.z, vec4.w };
		}
	}
	static void from_json(nlohmann::json& json, std::vector<glm::vec4>& vec4List)
	{
		for (auto& vec4 : vec4List)
		{
			json[0].get_to(vec4.x);
			json[1].get_to(vec4.y);
			json[2].get_to(vec4.z);
			json[3].get_to(vec4.w);
		}
	}

	static void to_json(nlohmann::json& json, glm::ivec2& ivec2)
	{
		json = { ivec2.x, ivec2.y };
	}
	static void from_json(nlohmann::json& json, glm::ivec2& ivec2)
	{
		json[0].get_to(ivec2.x);
		json[1].get_to(ivec2.y);
	}

	static void to_json(nlohmann::json& json, std::vector<glm::ivec2>& ivec2List)
	{
		for (auto& ivec2 : ivec2List)
		{
			json = { ivec2.x, ivec2.y };
		}
	}
	static void from_json(nlohmann::json& json, std::vector<glm::ivec2>& ivec2List)
	{
		for (auto& ivec2 : ivec2List)
		{
			json[0].get_to(ivec2.x);
			json[1].get_to(ivec2.y);
		}
	}

	static void to_json(nlohmann::json& json, glm::ivec3& ivec3)
	{
		json = { ivec3.x, ivec3.y, ivec3.z };
	}
	static void from_json(nlohmann::json& json, glm::ivec3& ivec3)
	{
		json[0].get_to(ivec3.x);
		json[1].get_to(ivec3.y);
		json[2].get_to(ivec3.z);
	}

	static void to_json(nlohmann::json& json, std::vector<glm::ivec3>& ivec3List)
	{
		for (auto& ivec3 : ivec3List)
		{
			json = { ivec3.x, ivec3.y, ivec3.z };
		}
	}
	static void from_json(nlohmann::json& json, std::vector<glm::ivec3>& ivec3List)
	{
		for (auto& ivec3 : ivec3List)
		{
			json[0].get_to(ivec3.x);
			json[1].get_to(ivec3.y);
			json[2].get_to(ivec3.z);
		}
	}

	static void to_json(nlohmann::json& json, glm::ivec4& ivec4)
	{
		json = { ivec4.x, ivec4.y, ivec4.z, ivec4.w };
	}
	static void from_json(nlohmann::json& json, glm::ivec4& ivec4)
	{
		json[0].get_to(ivec4.x);
		json[1].get_to(ivec4.y);
		json[2].get_to(ivec4.z);
		json[3].get_to(ivec4.w);
	}

	static void to_json(nlohmann::json& json, std::vector<glm::ivec4>& ivec4List)
	{
		for (auto& ivec4 : ivec4List)
		{
			json = { ivec4.x, ivec4.y, ivec4.z, ivec4.w };
		}
	}
	static void from_json(nlohmann::json& json, std::vector<glm::ivec4>& ivec4List)
	{
		for (auto& ivec4 : ivec4List)
		{
			json[0].get_to(ivec4.x);
			json[1].get_to(ivec4.y);
			json[2].get_to(ivec4.z);
			json[3].get_to(ivec4.w);
		}
	}

	static void to_json(nlohmann::json& json, glm::mat2& mat2)
	{
		json = { mat2[0][0], mat2[0][1],
				 mat2[1][0], mat2[1][1] };
	}
	static void from_json(nlohmann::json& json, glm::mat4& mat2)
	{
		json[0].get_to(mat2[0][0]);
		json[1].get_to(mat2[0][1]);

		json[4].get_to(mat2[1][0]);
		json[5].get_to(mat2[1][1]);
	}

	static void to_json(nlohmann::json& json, std::vector<glm::mat2>& mat2List)
	{
		for (auto& mat2 : mat2List)
		{
			json = { mat2[0][0], mat2[0][1],
					 mat2[1][0], mat2[1][1] };
		}
	}
	static void from_json(nlohmann::json& json, std::vector<glm::mat3>& mat2List)
	{
		for (auto& mat2 : mat2List)
		{
			json[0].get_to(mat2[0][0]);
			json[1].get_to(mat2[0][1]);

			json[4].get_to(mat2[1][0]);
			json[5].get_to(mat2[1][1]);
		}
	}

	static void to_json(nlohmann::json& json, glm::mat3& mat3)
	{
		json = { mat3[0][0], mat3[0][1], mat3[0][2],
				 mat3[1][0], mat3[1][1], mat3[1][2],
				 mat3[2][0], mat3[2][1], mat3[2][2] };
	}
	static void from_json(nlohmann::json& json, glm::mat3& mat3)
	{
		json[0].get_to(mat3[0][0]);
		json[1].get_to(mat3[0][1]);
		json[2].get_to(mat3[0][2]);

		json[4].get_to(mat3[1][0]);
		json[5].get_to(mat3[1][1]);
		json[6].get_to(mat3[1][2]);

		json[8].get_to(mat3[2][0]);
		json[9].get_to(mat3[2][1]);
		json[10].get_to(mat3[2][2]);
	}

	static void to_json(nlohmann::json& json, std::vector<glm::mat3>& mat3List)
	{
		for (auto& mat3 : mat3List)
		{
			json = { mat3[0][0], mat3[0][1], mat3[0][2],
					 mat3[1][0], mat3[1][1], mat3[1][2],
					 mat3[2][0], mat3[2][1], mat3[2][2] };
		}
	}
	//static void from_json(nlohmann::json& json, std::vector<glm::mat3>& mat3List)
	//{
	//	for (auto& mat3 : mat3List)
	//	{
	//		json[0].get_to(mat3[0][0]);
	//		json[1].get_to(mat3[0][1]);
	//		json[2].get_to(mat3[0][2]);

	//		json[4].get_to(mat3[1][0]);
	//		json[5].get_to(mat3[1][1]);
	//		json[6].get_to(mat3[1][2]);

	//		json[8].get_to(mat3[2][0]);
	//		json[9].get_to(mat3[2][1]);
	//		json[10].get_to(mat3[2][2]);
	//	}
	//}

	static void to_json(nlohmann::json& json, glm::mat4& mat4)
	{
		json = { mat4[0][0], mat4[0][1], mat4[0][2], mat4[0][3],
				 mat4[1][0], mat4[1][1], mat4[1][2], mat4[1][3],
				 mat4[2][0], mat4[2][1], mat4[2][2], mat4[2][3],
				 mat4[3][0], mat4[3][1],mat4[3][2], mat4[3][3] };
	}
	//static void from_json(nlohmann::json& json, glm::mat4& mat4)
	//{
	//	json[0].get_to(mat4[0][0]);
	//	json[1].get_to(mat4[0][1]);
	//	json[2].get_to(mat4[0][2]);
	//	json[3].get_to(mat4[0][3]);

	//	json[4].get_to(mat4[1][0]);
	//	json[5].get_to(mat4[1][1]);
	//	json[6].get_to(mat4[1][2]);
	//	json[7].get_to(mat4[1][3]);

	//	json[8].get_to(mat4[2][0]);
	//	json[9].get_to(mat4[2][1]);
	//	json[10].get_to(mat4[2][2]);
	//	json[11].get_to(mat4[2][3]);

	//	json[12].get_to(mat4[3][0]);
	//	json[13].get_to(mat4[3][1]);
	//	json[14].get_to(mat4[3][2]);
	//	json[15].get_to(mat4[3][3]);
	//}

	static void to_json(nlohmann::json& json, std::vector<glm::mat4>& mat4List)
	{
		for (auto& mat4 : mat4List)
		{
			json = { mat4[0][0], mat4[0][1], mat4[0][2], mat4[0][3],
					 mat4[1][0], mat4[1][1], mat4[1][2], mat4[1][3],
					 mat4[2][0], mat4[2][1], mat4[2][2], mat4[2][3],
					 mat4[3][0], mat4[3][1],mat4[3][2], mat4[3][3] };
		}
	}
	static void from_json(nlohmann::json& json, std::vector<glm::mat4> mat4List)
	{
		for (auto& mat4 : mat4List)
		{
			json[0].get_to(mat4[0][0]);
			json[1].get_to(mat4[0][1]);
			json[2].get_to(mat4[0][2]);
			json[3].get_to(mat4[0][3]);

			json[4].get_to(mat4[1][0]);
			json[5].get_to(mat4[1][1]);
			json[6].get_to(mat4[1][2]);
			json[7].get_to(mat4[1][3]);

			json[8].get_to(mat4[2][0]);
			json[9].get_to(mat4[2][1]);
			json[10].get_to(mat4[2][2]);
			json[11].get_to(mat4[2][3]);

			json[12].get_to(mat4[3][0]);
			json[13].get_to(mat4[3][1]);
			json[14].get_to(mat4[3][2]);
			json[15].get_to(mat4[3][3]);
		}
	}

	static VkAttachmentDescription LoadAttachmentDescription(nlohmann::json json)
	{
		return VkAttachmentDescription
		{
			.flags = json["flags"],
			.format = json["format"],
			.samples = json["samples"],
			.loadOp = json["loadOp"],
			.storeOp = json["storeOp"],
			.stencilLoadOp = json["stencilLoadOp"],
			.stencilStoreOp = json["stencilStoreOp"],
			.initialLayout = json["initialLayout"],
			.finalLayout = json["finalLayout"]
		};
	}

	static VkImageCreateInfo LoadImageCreateInfo(nlohmann::json json)
	{
		return VkImageCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = json["Flags"],
			.imageType = json["ImageType"],
			.format = json["Format"],
			.extent = VkExtent3D{
				json["extent"]["width"],
				json["extent"]["height"],
				json["extent"]["depth"]
			},
			.mipLevels = json["mipLevels"],
			.arrayLayers = json["arrayLayers"], 
			.samples = json["samples"],
			.tiling = json["tiling"],
			.usage = json["usage"],
			.sharingMode = json["sharingMode"],
			.queueFamilyIndexCount = json["queueFamilyIndexCount"],
			.pQueueFamilyIndices = json["pQueueFamilyIndices"].get<std::vector<uint32_t>>().data(),
			.initialLayout = json["initialLayout"]
		};
	}

	static VkSamplerCreateInfo LoadVulkanSamplerCreateInfo(nlohmann::json json)
	{
		return VkSamplerCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = json["flags"],
			.magFilter = json["magFilter"],
			.minFilter = json["minFilter"],
			.mipmapMode = json["mipmapMode"],
			.addressModeU = json["addressModeU"],
			.addressModeV = json["addressModeV"],
			.addressModeW = json["addressModeW"],
			.mipLodBias = json["mipLodBias"],
			.anisotropyEnable = json["anisotropyEnable"],
			.maxAnisotropy = json["maxAnisotropy"],
			.compareEnable = json["compareEnable"],
			.compareOp = json["compareOp"],
			.minLod = json["minLod"],
			.maxLod = json["maxLod"],
			.borderColor = json["borderColor"],
			.unnormalizedCoordinates = json["unnormalizedCoordinates"]
		};
	}

	static VkSubpassDependency LoadSubpassDependency(nlohmann::json json)
	{
		return VkSubpassDependency
		{
			.srcSubpass = json["srcSubpass"],
			.dstSubpass = json["dstSubpass"],
			.srcStageMask = json["srcStageMask"],
			.dstStageMask = json["dstStageMask"],
			.srcAccessMask = json["srcAccessMask"],
			.dstAccessMask = json["dstAccessMask"],
			.dependencyFlags = json["dependencyFlags"]
		};
	}
};