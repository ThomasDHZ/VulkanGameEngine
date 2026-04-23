using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.GameEngine.Components
{
    public enum ComponentTypeEnum : uint
    {
        kInputComponent,
        kSpriteComponent,
        kTransform2DComponent,
        kTransform3DComponent,
        kCameraFollowComponent,
        kDirectionalLightComponent,
        kPointLightComponent,
        kEndOfEnum
    }
}
