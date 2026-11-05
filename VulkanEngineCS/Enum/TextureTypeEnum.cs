using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Enum
{
    public enum TextureTypeEnum : uint
    {
        kTextureType_Undefined,
        kTextureType_ColorTexture,
        kTextureType_DepthTexture,
        kTextureType_StencilTexture,
        kTextureType_DataTexture,
        kTextureType_CubeMap,
        kTextureType_StorageTexture
    };
}
