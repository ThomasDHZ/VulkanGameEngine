﻿using Silk.NET.Core.Attributes;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    [TypeConverter(typeof(ExpandableObjectConverter))]
    public class SubpassDependencyModel : RenderPassEditorBaseModel
    {
        private uint _srcSubpass;
        private uint _dstSubpass;
        private PipelineStageFlags _srcStageMask;
        private PipelineStageFlags _dstStageMask;
        private AccessFlags _srcAccessMask;
        private AccessFlags _dstAccessMask;
        private DependencyFlags _dependencyFlags;


        [Category("Subpass Dependency")]
        public uint SrcSubpass
        {
            get => _srcSubpass;
            set
            {
                if (_srcSubpass != value)
                {
                    _srcSubpass = value;
                    OnPropertyChanged(nameof(SrcSubpass));
                }
            }
        }

        [Category("Subpass Dependency")]
        public uint DstSubpass
        {
            get => _dstSubpass;
            set
            {
                if (_dstSubpass != value)
                {
                    _dstSubpass = value;
                    OnPropertyChanged(nameof(DstSubpass));
                }
            }
        }

        [Category("Pipeline Stages")]
        public PipelineStageFlags SrcStageMask
        {
            get => _srcStageMask;
            set
            {
                if (_srcStageMask != value)
                {
                    _srcStageMask = value;
                    OnPropertyChanged(nameof(SrcStageMask));
                }
            }
        }

        [Category("Pipeline Stages")]
        public PipelineStageFlags DstStageMask
        {
            get => _dstStageMask;
            set
            {
                if (_dstStageMask != value)
                {
                    _dstStageMask = value;
                    OnPropertyChanged(nameof(DstStageMask));
                }
            }
        }

        [Category("Access Masks")]
        public AccessFlags SrcAccessMask
        {
            get => _srcAccessMask;
            set
            {
                if (_srcAccessMask != value)
                {
                    _srcAccessMask = value;
                    OnPropertyChanged(nameof(SrcAccessMask));
                }
            }
        }

        [Category("Access Masks")]
        public AccessFlags DstAccessMask
        {
            get => _dstAccessMask;
            set
            {
                if (_dstAccessMask != value)
                {
                    _dstAccessMask = value;
                    OnPropertyChanged(nameof(DstAccessMask));
                }
            }
        }

        [Category("Subpass Dependency")]
        public DependencyFlags DependencyFlags
        {
            get => _dependencyFlags;
            set
            {
                if (_dependencyFlags != value)
                {
                    _dependencyFlags = value;
                    OnPropertyChanged(nameof(DependencyFlags));
                }
            }
        }

        public SubpassDependencyModel() : base()
        {

        }

        public SubpassDependencyModel(string name) : base(name)
        {
        }

        public SubpassDependencyModel(uint? srcSubpass = null, uint? dstSubpass = null, PipelineStageFlags? srcStageMask = null, PipelineStageFlags? dstStageMask = null, AccessFlags? srcAccessMask = null, AccessFlags? dstAccessMask = null, DependencyFlags? dependencyFlags = null)
        {
            if (srcSubpass.HasValue)
            {
                SrcSubpass = srcSubpass.Value;
            }

            if (dstSubpass.HasValue)
            {
                DstSubpass = dstSubpass.Value;
            }

            if (srcStageMask.HasValue)
            {
                SrcStageMask = srcStageMask.Value;
            }

            if (dstStageMask.HasValue)
            {
                DstStageMask = dstStageMask.Value;
            }

            if (srcAccessMask.HasValue)
            {
                SrcAccessMask = srcAccessMask.Value;
            }

            if (dstAccessMask.HasValue)
            {
                DstAccessMask = dstAccessMask.Value;
            }

            if (dependencyFlags.HasValue)
            {
                DependencyFlags = dependencyFlags.Value;
            }
        }

        public SubpassDependency ConvertToVulkan()
        {
            return new SubpassDependency()
            {
                DstAccessMask = DstAccessMask,
                SrcAccessMask = SrcAccessMask,
                SrcStageMask = SrcStageMask,
                DstStageMask = DstStageMask,
                DependencyFlags = DependencyFlags,
                DstSubpass = DstSubpass,
                SrcSubpass = SrcSubpass
            };
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

    }
}