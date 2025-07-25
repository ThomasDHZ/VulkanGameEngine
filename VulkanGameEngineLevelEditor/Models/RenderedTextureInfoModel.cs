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
    public unsafe class RenderedTextureInfoModel
    {
        public string Name { get; set; } = String.Empty;
        public string RenderedTextureInfoName { get; set; } = string.Empty;
        public VkImageCreateInfoModel ImageCreateInfo { get; set; } = new VkImageCreateInfoModel();
        public VkSamplerCreateInfoModel _samplerCreateInfo { get; set; } = new VkSamplerCreateInfoModel();
        public VkAttachmentDescriptionModel AttachmentDescription { get; set; } = new VkAttachmentDescriptionModel();
        public RenderedTextureType TextureType;

        public RenderedTextureInfoModel() : base()
        {
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

        public void LoadJsonComponent(string jsonPath)
        {
            //var obj = base.LoadJsonComponent<RenderedTextureInfoModel>(jsonPath);
            //foreach (PropertyInfo property in typeof(RenderedTextureInfoModel).GetProperties())
            //{
            //    if (property.CanWrite)
            //    {
            //        property.SetValue(this, property.GetValue(obj));
            //    }
            //}
        }

    }
}
