#pragma once
#include "Texture.h"
#include <fstream>
#include "BakedTexture.h"

enum BakeTextureFormat
{
    Bake_BMP,
    Bake_JPG,
    Bake_PNG,
    Bake_TGA
};

class RenderedTexture : public Texture
{
private:
    void CreateTextureImage();
    void CreateTextureView();
    void CreateTextureSampler();
public:
    RenderedTexture();
    RenderedTexture(glm::ivec2 TextureResolution);
    RenderedTexture(glm::ivec2 TextureResolution, VkSampleCountFlagBits sampleCount);
    RenderedTexture(glm::ivec2 TextureResolution, VkSampleCountFlagBits sampleCount, VkFormat format);
    ~RenderedTexture();

    void RecreateRendererTexture(glm::vec2 TextureResolution);
    //void BakeDepthTexture(const char* filename, BakeTextureFormat textureFormat);
    std::shared_ptr<BakedTexture> BakeColorTexture(const char* filename, BakeTextureFormat textureFormat);
    //void BakeEnvironmentMapTexture(const char* filename);
};

std::vector<byte> ExportColorTexture(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, const char* filename, std::shared_ptr<Texture> texture, BakeTextureFormat textureFormat, uint32 channels);