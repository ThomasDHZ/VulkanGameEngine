using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Enum
{
    public enum MaterialPropertiesEnum : uint
    {
        kMaterialFeature_None = 0,
        kMaterialFeature_ClearCoat = 1u << 0,
        kMaterialFeature_Sheen = 1u << 1,
        kMaterialFeature_SubSurfaceScattering = 1u << 2,
        kMaterialFeature_Transmission = 1u << 3,
        kMaterialFeature_Anisotropy = 1u << 4,
        kMaterialFeature_FILM = 1u << 5,
        kMaterialFeature_TWO_SIDED = 1u << 6,
        kMaterialFeature_ClearCoatNormal = 1u << 7,
        kMaterialFeature_AlbedoOnly = 1u << 8,
        kMaterialFeature_UsingAlpha = 1u << 9,
    };
}
