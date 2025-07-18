using System;
using System.ComponentModel;
using System.Reflection;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;


namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class VkSubpassDependencyModel : RenderPassEditorBaseModel
    {
        private string Name;
        private uint _srcSubpass;
        private uint _dstSubpass;
        private VkPipelineStageFlagBits _srcStageMask;
        private VkPipelineStageFlagBits _dstStageMask;
        private VkAccessFlagBits _srcAccessMask;
        private VkAccessFlagBits _dstAccessMask;
        private VkDependencyFlagBits _dependencyFlags;


        [Category("Subpass Dependency")]
        public uint srcSubpass
        {
            get => _srcSubpass;
            set
            {
                if (_srcSubpass != value)
                {
                    _srcSubpass = value;
                    OnPropertyChanged(nameof(srcSubpass));
                }
            }
        }

        [Category("Subpass Dependency")]
        public uint dstSubpass
        {
            get => _dstSubpass;
            set
            {
                if (_dstSubpass != value)
                {
                    _dstSubpass = value;
                    OnPropertyChanged(nameof(dstSubpass));
                }
            }
        }

        [Category("Pipeline Stages")]
        [Browsable(true)]
        [Editor(typeof(FlagEnumUIEditor), typeof(System.Drawing.Design.UITypeEditor))]
        public VkPipelineStageFlagBits srcStageMask
        {
            get => _srcStageMask;
            set
            {
                if (_srcStageMask != value)
                {
                    _srcStageMask = value;
                    OnPropertyChanged(nameof(srcStageMask));
                }
            }
        }

        [Category("Pipeline Stages")]
        [Browsable(true)]
        [Editor(typeof(FlagEnumUIEditor), typeof(System.Drawing.Design.UITypeEditor))]
        public VkPipelineStageFlagBits dstStageMask
        {
            get => _dstStageMask;
            set
            {
                if (_dstStageMask != value)
                {
                    _dstStageMask = value;
                    OnPropertyChanged(nameof(dstStageMask));
                }
            }
        }

        [Category("Access Masks")]
        [Browsable(true)]
        [Editor(typeof(FlagEnumUIEditor), typeof(System.Drawing.Design.UITypeEditor))]
        public VkAccessFlagBits srcAccessMask
        {
            get => _srcAccessMask;
            set
            {
                if (_srcAccessMask != value)
                {
                    _srcAccessMask = value;
                    OnPropertyChanged(nameof(srcAccessMask));
                }
            }
        }

        [Category("Access Masks")]
        [Browsable(true)]
        [Editor(typeof(FlagEnumUIEditor), typeof(System.Drawing.Design.UITypeEditor))]
        public VkAccessFlagBits dstAccessMask
        {
            get => _dstAccessMask;
            set
            {
                if (_dstAccessMask != value)
                {
                    _dstAccessMask = value;
                    OnPropertyChanged(nameof(dstAccessMask));
                }
            }
        }

        [Category("Subpass Dependency")]
        [Editor(typeof(FlagEnumUIEditor), typeof(System.Drawing.Design.UITypeEditor))]
        public VkDependencyFlagBits dependencyFlags
        {
            get => _dependencyFlags;
            set
            {
                if (_dependencyFlags != value)
                {
                    _dependencyFlags = value;
                    OnPropertyChanged(nameof(dependencyFlags));
                }
            }
        }

        public VkSubpassDependencyModel() : base()
        {
        }
        public VkSubpassDependencyModel(string jsonFilePath) : base()
        {
            LoadJsonComponent(jsonFilePath);
        }

        public VkSubpassDependencyModel(string name, string jsonFilePath) : base()
        {
            Name = name;
            LoadJsonComponent(ConstConfig.DefaultSubpassDependencyModel);
        }

        public VkSubpassDependencyModel(uint? srcSubpass = null, uint? dstSubpass = null, VkPipelineStageFlagBits? srcStageMask = null, VkPipelineStageFlagBits? dstStageMask = null, VkAccessFlagBits? srcAccessMask = null, VkAccessFlagBits? dstAccessMask = null, VkDependencyFlagBits? dependencyFlags = null)
        {
            if (srcSubpass.HasValue)
            {
                this.srcSubpass = srcSubpass.Value;
            }

            if (dstSubpass.HasValue)
            {
                this.dstSubpass = dstSubpass.Value;
            }

            if (srcStageMask.HasValue)
            {
                this.srcStageMask = srcStageMask.Value;
            }

            if (dstStageMask.HasValue)
            {
                this.dstStageMask = dstStageMask.Value;
            }

            if (srcAccessMask.HasValue)
            {
                this.srcAccessMask = srcAccessMask.Value;
            }

            if (dstAccessMask.HasValue)
            {
                this.dstAccessMask = dstAccessMask.Value;
            }

            if (dependencyFlags.HasValue)
            {
                this.dependencyFlags = dependencyFlags.Value;
            }
        }

        public VkSubpassDependency Convert()
        {
            return new VkSubpassDependency()
            {
                dstAccessMask = dstAccessMask,
                srcAccessMask = srcAccessMask,
                srcStageMask = srcStageMask,
                dstStageMask = dstStageMask,
                dependencyFlags = dependencyFlags,
                dstSubpass = dstSubpass,
                srcSubpass = srcSubpass
            };
        }

        public void LoadJsonComponent(string jsonPath)
        {
            var obj = base.LoadJsonComponent<VkSubpassDependencyModel>(jsonPath);
            foreach (PropertyInfo property in typeof(VkSubpassDependencyModel).GetProperties())
            {
                if (property.CanWrite)
                {
                    property.SetValue(this, property.GetValue(obj));
                }
            }
        }

        public void SaveJsonComponent()
        {
            base.SaveJsonComponent($@"{ConstConfig.SubpassDependencyModelPath}{this.Name}.json", this);
        }
    }
}