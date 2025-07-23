using GlmSharp;
using System;
using System.ComponentModel;
using System.Reflection;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;


namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class VkImageCreateInfoModel : RenderPassEditorBaseModel
    {
        public string Name { get; set; }
        [ReadOnly(true)]
        public VkStructureType _sType = VkStructureType.VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        public VkImageCreateFlagBits _flags = 0;
        public void* _pNext;
        public VkImageType _imageType;
        public VkFormat _format;
        public VkExtent3DModel _extent = new VkExtent3DModel();
        public uint _mipLevels;
        public uint _arrayLayers;
        public VkSampleCountFlagBits _samples;
        public VkImageTiling _tiling;
        public VkImageUsageFlagBits _usage;
        public VkSharingMode _sharingMode;
        public uint _queueFamilyIndexCount;
        public unsafe uint* _pQueueFamilyIndices;
        public VkImageLayout _initialLayout;

        public VkImageCreateInfoModel() : base()
        {
        }

        public void LoadJsonComponent(string jsonPath)
        {
            var obj = base.LoadJsonComponent<VkImageCreateInfoModel>(jsonPath);
            foreach (PropertyInfo property in typeof(VkImageCreateInfoModel).GetProperties())
            {
                if (property.CanWrite)
                {
                    property.SetValue(this, property.GetValue(obj));
                }
            }
        }

        public void SaveJsonComponent()
        {
          //  base.SaveJsonComponent($@"{ConstConfig.CreateImageInfoPath}{this.Name}.json", this);
        }
    }
}
