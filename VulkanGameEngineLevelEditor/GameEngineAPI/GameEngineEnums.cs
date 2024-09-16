﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    public enum ColorChannelUsed
    {
        ChannelR = 1,
        ChannelRG,
        ChannelRGB,
        ChannelRGBA
    };

    public enum TextureUsageEnum
    {
        kUse_Undefined,
        kUse_2DImageTexture,
        kUse_2DDataTexture,
        kUse_2DRenderedTexture,
        kUse_2DDepthTexture,
        kUse_3DImageTexture,
        kUse_3DDataTexture,
        kUse_3DRenderedTexture,
        kUse_3DDepthTexture,
        kUse_CubeMapTexture,
        kUse_CubeMapDepthTexture
    };

    public enum TextureTypeEnum
    {
        kType_UndefinedTexture,
        kType_TextureAtlas,
        kType_RenderedColorTexture,
        kType_RenderedDepthTexture,
        kType_ReadableTexture,
        kType_DiffuseTextureMap,
        kType_SpecularTextureMap,
        kType_AlbedoTextureMap,
        kType_MetallicTextureMap,
        kType_RoughnessTextureMap,
        kType_AmbientOcclusionTextureMap,
        kType_NormalTextureMap,
        kType_DepthTextureMap,
        kType_AlphaTextureMap,
        kType_EmissionTextureMap,
        kType_PaletteRotationMap,
        kType_CubeMapTexture,
        kType_CubeMapDepthTexture,
        kType_EnvironmentTexture,
        kType_RenderedCubeMap,
        kType_BakedTexture
    };
}
