using System;
using System.ComponentModel;
using System.Reflection;

namespace VulkanGameEngineLevelEditor.Models
{
    public enum RenderedTextureType
    {
        ColorRenderedTexture,
        DepthRenderedTexture,
        InputAttachmentTexture,
        ResolveAttachmentTexture
    };

    [Serializable]
    public unsafe class RenderedTextureInfoModel : RenderPassEditorBaseModel
    {
        private string Name = String.Empty;
        private string _renderedTextureInfoName = string.Empty;
        private VkImageCreateInfoModel _imageCreateInfo = new VkImageCreateInfoModel();
        private VkSamplerCreateInfoModel _samplerCreateInfo = new VkSamplerCreateInfoModel();
        private VkAttachmentDescriptionModel _attachmentDescription = new VkAttachmentDescriptionModel();
        private RenderedTextureType _textureType;

        public string RenderedTextureInfoName
        {
            get => _renderedTextureInfoName;
            set
            {
                if (_renderedTextureInfoName != value)
                {
                    _renderedTextureInfoName = value;
                    OnPropertyChanged(nameof(RenderedTextureInfoName));
                }
            }
        }

        [Category("Image")]
        public VkImageCreateInfoModel ImageCreateInfo
        {
            get => _imageCreateInfo;
            set
            {
                if (_imageCreateInfo != value)
                {
                    _imageCreateInfo = value;
                    OnPropertyChanged(nameof(ImageCreateInfo));
                }
            }
        }

        [Category("Sampler")]
        public VkSamplerCreateInfoModel SamplerCreateInfo
        {
            get => _samplerCreateInfo;
            set
            {
                if (_samplerCreateInfo != value)
                {
                    _samplerCreateInfo = value;
                    OnPropertyChanged(nameof(SamplerCreateInfo));
                }
            }
        }

        [Category("Attachment")]
        public VkAttachmentDescriptionModel AttachmentDescription
        {
            get => _attachmentDescription;
            set
            {
                if (_attachmentDescription != value)
                {
                    _attachmentDescription = value;
                    OnPropertyChanged(nameof(AttachmentDescription));
                }
            }
        }

        [Category("Texture")]
        public RenderedTextureType TextureType
        {
            get => _textureType;
            set
            {
                if (_textureType != value)
                {
                    _textureType = value;
                    OnPropertyChanged(nameof(TextureType));
                }
            }
        }

        public RenderedTextureInfoModel() : base()
        {
            _imageCreateInfo = new VkImageCreateInfoModel();
            _samplerCreateInfo = new VkSamplerCreateInfoModel();
            _attachmentDescription = new VkAttachmentDescriptionModel();
        }

        public RenderedTextureInfoModel(string jsonFilePath) : base()
        {
            LoadJsonComponent(jsonFilePath);
        }

        public RenderedTextureInfoModel(string name, string jsonFilePath) : base()
        {
            Name = name;
            LoadJsonComponent(jsonFilePath);
        }

        //public RenderedTextureInfoDLL ToDLL()
        //{
        //    fixed (byte* namePtr = System.Text.Encoding.UTF8.GetBytes(_name + "\0"))
        //    fixed (byte* textureInfoPtr = System.Text.Encoding.UTF8.GetBytes(_renderedTextureInfoName + "\0"))
        //    {
        //        return new RenderedTextureInfoDLL
        //        {
        //            //  Name = (IntPtr)namePtr,
        //            _attachmentDescription = _attachmentDescription.Convert(),
        //            _textureType = _textureType,
        //            _imageCreateInfo = _imageCreateInfo.Convert(),
        //            //_renderedTextureInfoName = (IntPtr)textureInfoPtr,
        //            _samplerCreateInfo = _samplerCreateInfo.Convert()
        //        };
        //    }
        //}

        public void LoadJsonComponent(string jsonPath)
        {
            var obj = base.LoadJsonComponent<RenderedTextureInfoModel>(jsonPath);
            foreach (PropertyInfo property in typeof(RenderedTextureInfoModel).GetProperties())
            {
                if (property.CanWrite)
                {
                    property.SetValue(this, property.GetValue(obj));
                }
            }
        }

    }
}
