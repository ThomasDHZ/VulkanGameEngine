﻿using Silk.NET.Core.Attributes;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class VkStencilOpState
    {
        public StencilOp failOp { get; set; }
        public StencilOp passOp { get; set; }
        public StencilOp depthFailOp { get; set; }
        public CompareOp compareOp { get; set; }
        public uint compareMask { get; set; }
        public uint writeMask { get; set; }
        public uint reference { get; set; }
        public VkStencilOpState() { }

        public StencilOpState Convert()
        {
            return new StencilOpState
            {
                CompareMask = compareMask,
                WriteMask = writeMask,
                Reference = reference,
                CompareOp = compareOp,
                DepthFailOp = depthFailOp,
                FailOp = failOp,
                PassOp = passOp
            };
        }

        public StencilOpState* ConvertPtr()
        {
            StencilOpState* ptr = (StencilOpState*)Marshal.AllocHGlobal(sizeof(StencilOpState));
            ptr->FailOp = failOp;
            ptr->PassOp = passOp;
            ptr->DepthFailOp = depthFailOp;
            ptr->CompareOp = compareOp;
            ptr->CompareMask = compareMask;
            ptr->WriteMask = writeMask;
            ptr->Reference = reference;
            return ptr;
        }
    }
}