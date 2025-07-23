using System;
using System.ComponentModel;
using System.Reflection;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;

namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class VkAttachmentDescriptionModel : RenderPassEditorBaseModel
    {
        private string Name;
        private VkStructureType _structureType;
        private VkAttachmentDescriptionFlagBits _flags;
        private void* _pNext;
        private VkFormat _format;
        private VkSampleCountFlagBits _samples;
        private VkAttachmentLoadOp _loadOp;
        private VkAttachmentStoreOp _storeOp;
        private VkAttachmentLoadOp _stencilLoadOp;
        private VkAttachmentStoreOp _stencilStoreOp;
        private VkImageLayout _initialLayout;
        private VkImageLayout _finalLayout;

        [Category("Attachment Description")]
        public VkAttachmentDescriptionFlagBits flags
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

        [Category("Attachment Description")]
        public VkFormat format
        {
            get => _format;
            set
            {
                if (_format != value)
                {
                    _format = value;
                    OnPropertyChanged(nameof(format));
                }
            }
        }

        [Category("Attachment Description")]
        public VkSampleCountFlagBits samples
        {
            get => _samples;
            set
            {
                if (_samples != value)
                {
                    _samples = value;
                    OnPropertyChanged(nameof(samples));
                }
            }
        }

        [Category("Load/Store Operations")]
        public VkAttachmentLoadOp loadOp
        {
            get => _loadOp;
            set
            {
                if (_loadOp != value)
                {
                    _loadOp = value;
                    OnPropertyChanged(nameof(loadOp));
                }
            }
        }

        [Category("Load/Store Operations")]
        public VkAttachmentStoreOp storeOp
        {
            get => _storeOp;
            set
            {
                if (_storeOp != value)
                {
                    _storeOp = value;
                    OnPropertyChanged(nameof(storeOp));
                }
            }
        }

        [Category("Load/Store Operations")]
        public VkAttachmentLoadOp stencilLoadOp
        {
            get => _stencilLoadOp;
            set
            {
                if (_stencilLoadOp != value)
                {
                    _stencilLoadOp = value;
                    OnPropertyChanged(nameof(stencilLoadOp));
                }
            }
        }

        [Category("Load/Store Operations")]
        public VkAttachmentStoreOp stencilStoreOp
        {
            get => _stencilStoreOp;
            set
            {
                if (_stencilStoreOp != value)
                {
                    _stencilStoreOp = value;
                    OnPropertyChanged(nameof(stencilStoreOp));
                }
            }
        }

        [Category("Attachment Description")]
        public VkImageLayout initialLayout
        {
            get => _initialLayout;
            set
            {
                if (_initialLayout != value)
                {
                    _initialLayout = value;
                    OnPropertyChanged(nameof(initialLayout));
                }
            }
        }

        [Category("Attachment Description")]
        public VkImageLayout finalLayout
        {
            get => _finalLayout;
            set
            {
                if (_finalLayout != value)
                {
                    _finalLayout = value;
                    OnPropertyChanged(nameof(finalLayout));
                }
            }
        }

        public VkAttachmentDescriptionModel() : base()
        {
        }

        public VkAttachmentDescriptionModel(string jsonFilePath) : base()
        {
           // LoadJsonComponent(jsonFilePath);
        }

        public VkAttachmentDescriptionModel(string name, string jsonFilePath) : base()
        {
            Name = name;
           // LoadJsonComponent(@$"{ConstConfig.BaseDirectoryPath} DefaultColorAttachmentDescriptionModel);
        }

        public VkAttachmentDescription Convert()
        {
            return new VkAttachmentDescription
            {
                flags = flags,
                format = format,
                samples = samples,
                loadOp = loadOp,
                storeOp = storeOp,
                stencilLoadOp = stencilLoadOp,
                stencilStoreOp = stencilStoreOp,
                initialLayout = initialLayout,
                finalLayout = finalLayout
            };
        }

        public void LoadJsonComponent(string jsonPath)
        {
            var obj = base.LoadJsonComponent<VkAttachmentDescriptionModel>(jsonPath);
            foreach (PropertyInfo property in typeof(VkAttachmentDescriptionModel).GetProperties())
            {
                if (property.CanWrite)
                {
                    property.SetValue(this, property.GetValue(obj));
                }
            }
        }

        public void SaveJsonComponent()
        {
           // base.SaveJsonComponent($@"{ConstConfig.AttachmentDescriptionModelPath}{this.Name}.json", this);
        }
    }
}
