using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Enum
{
    public enum TextureUsageTypeEnum : uint
    {
        kUsageType_Undefined,
        kUsageType_SwapChainTexture,
        kUsageType_OffscreenColorTexture,
        kUsageType_DepthBufferTexture,
        kUsageType_GBufferTexture,
        kUsageType_IrradianceTexture,
        kUsageType_PrefilterTexture,
        kUsageType_CubeMap,
        kUsageType_BRDFTexture,
        kUsageType_Texture
    };
}
