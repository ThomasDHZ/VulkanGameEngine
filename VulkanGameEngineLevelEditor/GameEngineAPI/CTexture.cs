﻿using GlmSharp;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Drawing.Imaging;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.Vulkan;
using Image = Silk.NET.Vulkan.Image;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    public static unsafe class CTexture
    {
        static Vk vk = Vk.GetApi();
        public static void CreateBuffer(uint size, BufferUsageFlags usage, MemoryPropertyFlags properties, Silk.NET.Vulkan.Buffer* buffer, DeviceMemory* bufferMemory)
        {
            BufferCreateInfo bufferInfo = new BufferCreateInfo
            {
                SType = StructureType.BufferCreateInfo,
                Size = size,
                Usage = usage,
                SharingMode = SharingMode.Exclusive
            };
            var BufferInfo = bufferInfo;
            vk.CreateBuffer(VulkanRenderer.device, &BufferInfo, null, buffer);

            MemoryRequirements memRequirements;
            vk.GetBufferMemoryRequirements(VulkanRenderer.device, *buffer, &memRequirements);

            MemoryAllocateInfo allocInfo = new MemoryAllocateInfo
            {
                SType = StructureType.MemoryAllocateInfo,
                AllocationSize = memRequirements.Size,
                MemoryTypeIndex = VulkanRenderer.GetMemoryType(memRequirements.MemoryTypeBits, properties)
            };

            vk.AllocateMemory(VulkanRenderer.device, &allocInfo, null, bufferMemory);
            vk.BindBufferMemory(VulkanRenderer.device, *buffer, *bufferMemory, 0);
        }


        public static void UpdateImageLayout(CommandBuffer commandBuffer, Silk.NET.Vulkan.Image image, ref Silk.NET.Vulkan.ImageLayout oldImageLayout, Silk.NET.Vulkan.ImageLayout newImageLayout, uint MipLevel, ImageAspectFlags imageAspectFlags)
        {
            ImageSubresourceRange ImageSubresourceRange = new ImageSubresourceRange();
            ImageSubresourceRange.AspectMask = imageAspectFlags;
            ImageSubresourceRange.LevelCount = Vk.RemainingMipLevels;
            ImageSubresourceRange.LayerCount = 1;

            ImageMemoryBarrier barrier = new ImageMemoryBarrier();
            barrier.SType = StructureType.ImageMemoryBarrier;
            barrier.OldLayout = oldImageLayout;
            barrier.NewLayout = newImageLayout;
            barrier.Image = image;
            barrier.SubresourceRange = ImageSubresourceRange;
            barrier.SrcAccessMask = 0;
            barrier.DstAccessMask = AccessFlags.TransferWriteBit;

            vk.CmdPipelineBarrier(commandBuffer, PipelineStageFlags.AllCommandsBit, PipelineStageFlags.AllCommandsBit, 0, 0, null, 0, null, 1, &barrier);
            oldImageLayout = newImageLayout;
        }

        public static byte[] GetTextureData(byte[] Data, uint Width, uint Height)
        {
            if (Data == null || Width <= 0 || Height <= 0)
            {
                throw new InvalidOperationException("Invalid texture data.");
            }

            var bitmap = new Bitmap((int)Width, (int)Height, PixelFormat.Format32bppArgb);
            BitmapData bmpData = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.WriteOnly, bitmap.PixelFormat);

            byte* ptr = (byte*)bmpData.Scan0.ToPointer();
            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {
                    int dataIndex = (y * (int)Width + x) * 4;
                    ptr[dataIndex + 0] = Data[dataIndex + 2];
                    ptr[dataIndex + 1] = Data[dataIndex + 1];
                    ptr[dataIndex + 2] = Data[dataIndex + 0];
                    ptr[dataIndex + 3] = Data[dataIndex + 3];
                }
            }

            byte[] data = new byte[Width * Height * 4];
            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {

                    int index = (y * (int)Width + x) * 4;
                    data[index + 0] = 255;
                    data[index + 1] = 255;
                    data[index + 2] = 255;
                    data[index + 3] = 255;
                }
            }

            return data;
        }

        public static Result CreateTextureImage(ImageCreateInfo createInfo, out Silk.NET.Vulkan.Image image, out DeviceMemory textureMemory, int Width, int Height, Format format, uint MipLevels)
        {
            image = new Silk.NET.Vulkan.Image();
            textureMemory = new DeviceMemory();


            Silk.NET.Vulkan.Image tempImagePtr = new Silk.NET.Vulkan.Image();
            Result result = vk.CreateImage(VulkanRenderer.device, &createInfo, null, &tempImagePtr);
          

            MemoryRequirements memRequirements;
            vk.GetImageMemoryRequirements(VulkanRenderer.device, tempImagePtr, &memRequirements);

            MemoryAllocateInfo allocInfo = new MemoryAllocateInfo
            {
                SType = StructureType.MemoryAllocateInfo,
                AllocationSize = memRequirements.Size,
                MemoryTypeIndex = VulkanRenderer.GetMemoryType(memRequirements.MemoryTypeBits, MemoryPropertyFlags.DeviceLocalBit)
            };

            DeviceMemory textureMemoryPtr = new DeviceMemory();
            result = vk.AllocateMemory(VulkanRenderer.device, &allocInfo, null, &textureMemoryPtr);
            if (result != Result.Success)
            {
            }

            image = tempImagePtr;
            textureMemory = textureMemoryPtr;
            result = vk.BindImageMemory(VulkanRenderer.device, image, textureMemory, 0);
            if (result != Result.Success)
            {
            }

            

            return result;
        }

        public static Result TransitionImageLayout(CommandBuffer commandBuffer, Silk.NET.Vulkan.Image image, uint mipmapLevels, ref Silk.NET.Vulkan.ImageLayout oldLayout, Silk.NET.Vulkan.ImageLayout newLayout, ImageAspectFlags colorFlags)
        {
            PipelineStageFlags sourceStage = PipelineStageFlags.AllCommandsBit;
            PipelineStageFlags destinationStage = PipelineStageFlags.AllCommandsBit;
            ImageMemoryBarrier barrier = new ImageMemoryBarrier()
            {
                SType = StructureType.ImageMemoryBarrier,
                OldLayout = oldLayout,
                NewLayout = newLayout,
                SrcQueueFamilyIndex = Vk.QueueFamilyIgnored,
                DstQueueFamilyIndex = Vk.QueueFamilyIgnored,
                Image = image,

                SubresourceRange = new ImageSubresourceRange()
                {
                    AspectMask = colorFlags,
                    LevelCount = mipmapLevels,
                    BaseArrayLayer = 0,
                    BaseMipLevel = 0,
                    LayerCount = Vk.RemainingArrayLayers,
                }
            };
            if (oldLayout == Silk.NET.Vulkan.ImageLayout.Undefined &&
                newLayout == Silk.NET.Vulkan.ImageLayout.TransferDstOptimal)
            {
                barrier.SrcAccessMask = 0;
                barrier.DstAccessMask = AccessFlags.TransferWriteBit;

                sourceStage = PipelineStageFlags.TopOfPipeBit;
                destinationStage = PipelineStageFlags.TransferBit;
            }
            else if (oldLayout == Silk.NET.Vulkan.ImageLayout.TransferDstOptimal &&
                     newLayout == Silk.NET.Vulkan.ImageLayout.ReadOnlyOptimal)
            {
                barrier.SrcAccessMask = AccessFlags.TransferWriteBit;
                barrier.DstAccessMask = AccessFlags.AccessMemoryReadBit;

                sourceStage = PipelineStageFlags.TransferBit;
                destinationStage = PipelineStageFlags.FragmentShaderBit;
            }

            vk.CmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, null, 0, null, 1, &barrier);
            oldLayout = newLayout;

            return Result.Success;
        }

        public static Result CopyBufferToTexture(ref Silk.NET.Vulkan.Buffer buffer, Silk.NET.Vulkan.Image image, Extent3D extent, TextureUsageEnum textureUsage, ImageAspectFlags imageAspectFlags)
        {
            BufferImageCopy bufferImage = new BufferImageCopy()
            {
                BufferOffset = 0,
                BufferRowLength = 0,
                BufferImageHeight = 0,
                ImageSubresource = new ImageSubresourceLayers
                {
                    AspectMask = imageAspectFlags,
                    MipLevel = 0,
                    BaseArrayLayer = 0,
                    LayerCount = (uint)(textureUsage == TextureUsageEnum.kUse_CubeMapTexture ? 6 : 1),
                },
                ImageOffset = new Offset3D { X = 0, Y = 0, Z = 0 },
                ImageExtent = extent
            };

            CommandBuffer commandBuffer = VulkanRenderer.BeginSingleUseCommandBuffer();
            vk.CmdCopyBufferToImage(commandBuffer, buffer, image, Silk.NET.Vulkan.ImageLayout.TransferDstOptimal, 1, &bufferImage);
            VulkanRenderer.EndSingleUseCommandBuffer(commandBuffer);

            return Result.Success;
        }

        public static Result QuickTransitionImageLayout(Image image, Silk.NET.Vulkan.ImageLayout oldLayout, Silk.NET.Vulkan.ImageLayout newLayout, uint MipMapLevels, ImageAspectFlags imageAspectFlags)
        {
            CommandBuffer commandBuffer = VulkanRenderer.BeginSingleUseCommandBuffer();
            CTexture.TransitionImageLayout(commandBuffer, image, MipMapLevels, ref oldLayout, newLayout, imageAspectFlags);
            Result result = VulkanRenderer.EndSingleUseCommandBuffer(commandBuffer);

            return result;
        }

        public static Result CopyBufferToTexture(ref Silk.NET.Vulkan.Buffer buffer, Silk.NET.Vulkan.Image image, TextureUsageEnum textureType, vec3 textureSize, Extent3D extent, ImageAspectFlags imageAspectFlags)
        {
            BufferImageCopy BufferImage = new BufferImageCopy()
            {
                BufferOffset = 0,
                BufferRowLength = 0,
                BufferImageHeight = 0,
                ImageExtent = new Extent3D()
                {
                    Width = (uint)extent.Width,
                    Height = (uint)extent.Height,
                    Depth = (uint)extent.Depth,
                },
                ImageOffset = new Offset3D()
                {
                    X = 0,
                    Y = 0,
                    Z = 0,
                },
                ImageSubresource = new ImageSubresourceLayers()
                {
                    AspectMask = imageAspectFlags,
                    MipLevel = 0,
                    BaseArrayLayer = 0,
                    LayerCount = 1,
                }

            };
            if (textureType == TextureUsageEnum.kUse_CubeMapTexture)
            {
                BufferImage.ImageSubresource.LayerCount = 6;
            }
            CommandBuffer commandBuffer = VulkanRenderer.BeginSingleUseCommandBuffer();
            vk.CmdCopyBufferToImage(commandBuffer, buffer, image, Silk.NET.Vulkan.ImageLayout.TransferDstOptimal, 1, &BufferImage);
            return VulkanRenderer.EndSingleUseCommandBuffer(commandBuffer);
        }

        public static Result GenerateMipmaps(Silk.NET.Vulkan.Image image, int Width, int Height, Format format, uint mipLevels, ImageAspectFlags imageAspectFlags)
        {
            uint mipWidth = (uint)Width;
            uint mipHeight = (uint)Height;

            vk.GetPhysicalDeviceFormatProperties(VulkanRenderer.physicalDevice, format, out FormatProperties formatProperties);
            if ((formatProperties.OptimalTilingFeatures & FormatFeatureFlags.SampledImageFilterLinearBit) == 0)
            {
                // Handle error if needed
            }

            CommandBuffer commandBuffer = VulkanRenderer.BeginSingleUseCommandBuffer();
            ImageMemoryBarrier imageMemoryBarrier = new ImageMemoryBarrier
            {
                SType = StructureType.ImageMemoryBarrier,
                Image = image,
                SrcQueueFamilyIndex = uint.MaxValue,
                DstQueueFamilyIndex = uint.MaxValue,
                SubresourceRange = new ImageSubresourceRange
                {
                    AspectMask = imageAspectFlags,
                    BaseArrayLayer = 0,
                    LayerCount = 1,
                    LevelCount = 1
                }
            };

            for (uint x = 1; x < mipLevels; x++)
            {
                imageMemoryBarrier.SubresourceRange.BaseMipLevel = x - 1;
                imageMemoryBarrier.OldLayout = Silk.NET.Vulkan.ImageLayout.TransferDstOptimal;
                imageMemoryBarrier.NewLayout = Silk.NET.Vulkan.ImageLayout.TransferSrcOptimal;
                imageMemoryBarrier.SrcAccessMask = AccessFlags.TransferWriteBit;
                imageMemoryBarrier.DstAccessMask = AccessFlags.TransferReadBit;
                vk.CmdPipelineBarrier(commandBuffer, PipelineStageFlags.TransferBit, PipelineStageFlags.TransferBit, 0, 0, null, 0, null, 1, ref imageMemoryBarrier);

                ImageBlit imageBlit = new ImageBlit
                {
                    SrcOffsets = new ImageBlit.SrcOffsetsBuffer()
                    {
                        Element0 = new Offset3D(0, 0, 0),
                        Element1 = new Offset3D((int)mipWidth, (int)mipHeight, 1)
                    },
                    DstOffsets = new ImageBlit.DstOffsetsBuffer()
                    {
                        Element0 = new Offset3D(0, 0, 0),
                        Element1 = new Offset3D((int)mipWidth > 1 ? (int)mipWidth / 2 : 1, (int)mipHeight > 1 ? (int)mipHeight / 2 : 1, 1)
                    },
                    SrcSubresource = new ImageSubresourceLayers
                    {
                        AspectMask = imageAspectFlags,
                        MipLevel = x - 1,
                        BaseArrayLayer = 0,
                        LayerCount = 1
                    },
                    DstSubresource = new ImageSubresourceLayers
                    {
                        AspectMask = imageAspectFlags,
                        MipLevel = x,
                        BaseArrayLayer = 0,
                        LayerCount = 1
                    }
                };
                vk.CmdBlitImage(commandBuffer, image, Silk.NET.Vulkan.ImageLayout.TransferSrcOptimal, image, Silk.NET.Vulkan.ImageLayout.TransferDstOptimal, 1, ref imageBlit, Filter.Linear);

                imageMemoryBarrier.OldLayout = Silk.NET.Vulkan.ImageLayout.TransferSrcOptimal;
                imageMemoryBarrier.NewLayout = Silk.NET.Vulkan.ImageLayout.ShaderReadOnlyOptimal;
                imageMemoryBarrier.SrcAccessMask = AccessFlags.TransferReadBit;
                imageMemoryBarrier.DstAccessMask = AccessFlags.ShaderReadBit;

                vk.CmdPipelineBarrier(commandBuffer, PipelineStageFlags.TransferBit, PipelineStageFlags.FragmentShaderBit, 0, 0, null, 0, null, 1, ref imageMemoryBarrier);

                if (mipWidth > 1)
                {
                    mipWidth /= 2;
                }
                if (mipHeight > 1)
                {
                    mipHeight /= 2;
                }
            }

            imageMemoryBarrier.SubresourceRange.BaseMipLevel = mipLevels - 1;
            imageMemoryBarrier.OldLayout = Silk.NET.Vulkan.ImageLayout.TransferDstOptimal;
            imageMemoryBarrier.NewLayout = Silk.NET.Vulkan.ImageLayout.ShaderReadOnlyOptimal;
            imageMemoryBarrier.SrcAccessMask = AccessFlags.TransferWriteBit;
            imageMemoryBarrier.DstAccessMask = AccessFlags.ShaderReadBit;

            vk.CmdPipelineBarrier(commandBuffer, PipelineStageFlags.TransferBit, PipelineStageFlags.FragmentShaderBit, 0, 0, null, 0, null, 1, ref imageMemoryBarrier);
            return VulkanRenderer.EndSingleUseCommandBuffer(commandBuffer);
        }
    }
}
