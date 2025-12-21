using System.ComponentModel;
using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct RenderAreaModel : INotifyPropertyChanged
    {
        public bool UseSwapChainRenderArea { get; set; }
        public VkRect2D RenderArea { get; set; }
        public event PropertyChangedEventHandler PropertyChanged;
    }
}
