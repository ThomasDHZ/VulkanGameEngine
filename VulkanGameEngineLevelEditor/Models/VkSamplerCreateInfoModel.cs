using System;
using System.ComponentModel;
using System.Reflection;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;


namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class VkSamplerCreateInfoModel : RenderPassEditorBaseModel
    {
        //  IMapper _mapper;
        public string Name { get; set; } = string.Empty;
        private VkStructureType _sType = VkStructureType.VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        private VkSamplerCreateFlagBits _flags = 0;
        private void* _pNext = null;
        private VkFilter _magFilter;
        private VkFilter _minFilter;
        private VkSamplerMipmapMode _mipmapMode;
        private VkSamplerAddressMode _addressModeU;
        private VkSamplerAddressMode _addressModeV;
        private VkSamplerAddressMode _addressModeW;
        private float _mipLodBias;
        private bool _anisotropyEnable;
        private float _maxAnisotropy;
        private bool _compareEnable;
        private VkCompareOp _compareOp;
        private float _minLod;
        private float _maxLod;
        private VkBorderColor _borderColor;
        private bool _unnormalizedCoordinates;

        //    public VkSamplerCreateInfoDLL DLL => _mapper.Map<VkSamplerCreateInfoDLL>(this);

        [Browsable(false)]
        [Newtonsoft.Json.JsonIgnore]
        public VkStructureType sType
        {
            get => _sType;
            set
            {
                if (_sType != value)
                {
                    _sType = value;
                    OnPropertyChanged(nameof(sType));
                }
            }
        }

        [Category("Sampler Properties")]
        [Browsable(false)]
        [Newtonsoft.Json.JsonIgnore]
        public VkSamplerCreateFlagBits flags
        {
            get => _flags;
            set
            {
                if (_flags != value)
                {
                    _flags = value;
                    OnPropertyChanged(nameof(flags));
                }
            }
        }

        [Browsable(false)]
        [Newtonsoft.Json.JsonIgnore]
        [Editor(typeof(FlagEnumUIEditor), typeof(System.Drawing.Design.UITypeEditor))]
        public void* pNext
        {
            get => _pNext;
            set
            {
                if (_pNext != value)
                {
                    _pNext = value;
                    OnPropertyChanged(nameof(pNext));
                }
            }
        }

        [Category("Sampler Properties")]
        public VkFilter magFilter
        {
            get => _magFilter;
            set
            {
                if (_magFilter != value)
                {
                    _magFilter = value;
                    OnPropertyChanged(nameof(magFilter));
                }
            }
        }

        [Category("Sampler Properties")]
        public VkFilter minFilter
        {
            get => _minFilter;
            set
            {
                if (_minFilter != value)
                {
                    _minFilter = value;
                    OnPropertyChanged(nameof(minFilter));
                }
            }
        }

        [Category("Sampler Properties")]
        public VkSamplerMipmapMode mipmapMode
        {
            get => _mipmapMode;
            set
            {
                if (_mipmapMode != value)
                {
                    _mipmapMode = value;
                    OnPropertyChanged(nameof(mipmapMode));
                }
            }
        }

        [Category("Address Modes")]
        public VkSamplerAddressMode addressModeU
        {
            get => _addressModeU;
            set
            {
                if (_addressModeU != value)
                {
                    _addressModeU = value;
                    OnPropertyChanged(nameof(addressModeU));
                }
            }
        }

        [Category("Address Modes")]
        public VkSamplerAddressMode addressModeV
        {
            get => _addressModeV;
            set
            {
                if (_addressModeV != value)
                {
                    _addressModeV = value;
                    OnPropertyChanged(nameof(addressModeV));
                }
            }
        }

        [Category("Address Modes")]
        public VkSamplerAddressMode addressModeW
        {
            get => _addressModeW;
            set
            {
                if (_addressModeW != value)
                {
                    _addressModeW = value;
                    OnPropertyChanged(nameof(addressModeW));
                }
            }
        }

        [Category("Sampler Properties")]
        public float mipLodBias
        {
            get => _mipLodBias;
            set
            {
                if (_mipLodBias != value)
                {
                    _mipLodBias = value;
                    OnPropertyChanged(nameof(mipLodBias));
                }
            }
        }

        [Category("Anisotropy")]
        public bool anisotropyEnable
        {
            get => _anisotropyEnable;
            set
            {
                if (_anisotropyEnable != value)
                {
                    _anisotropyEnable = value;
                    OnPropertyChanged(nameof(anisotropyEnable));
                }
            }
        }

        [Category("Anisotropy")]
        public float maxAnisotropy
        {
            get => _maxAnisotropy;
            set
            {
                if (_maxAnisotropy != value)
                {
                    _maxAnisotropy = value;
                    OnPropertyChanged(nameof(maxAnisotropy));
                }
            }
        }

        [Category("Comparison")]
        public bool compareEnable
        {
            get => _compareEnable;
            set
            {
                if (_compareEnable != value)
                {
                    _compareEnable = value;
                    OnPropertyChanged(nameof(compareEnable));
                }
            }
        }

        [Category("Comparison")]
        public VkCompareOp compareOp
        {
            get => _compareOp;
            set
            {
                if (_compareOp != value)
                {
                    _compareOp = value;
                    OnPropertyChanged(nameof(compareOp));
                }
            }
        }

        [Category("LOD")]
        public float minLod
        {
            get => _minLod;
            set
            {
                if (_minLod != value)
                {
                    _minLod = value;
                    OnPropertyChanged(nameof(minLod));
                }
            }
        }

        [Category("LOD")]
        public float maxLod
        {
            get => _maxLod;
            set
            {
                if (_maxLod != value)
                {
                    _maxLod = value;
                    OnPropertyChanged(nameof(maxLod));
                }
            }
        }

        [Category("Border Color")]
        public VkBorderColor borderColor
        {
            get => _borderColor;
            set
            {
                if (_borderColor != value)
                {
                    _borderColor = value;
                    OnPropertyChanged(nameof(borderColor));
                }
            }
        }

        [Category("Sampler Properties")]
        public bool unnormalizedCoordinates
        {
            get => _unnormalizedCoordinates;
            set
            {
                if (_unnormalizedCoordinates != value)
                {
                    _unnormalizedCoordinates = value;
                    OnPropertyChanged(nameof(unnormalizedCoordinates));
                }
            }
        }

        public VkSamplerCreateInfoModel() : base()
        {
        }
    }
}

