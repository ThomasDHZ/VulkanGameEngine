using System.ComponentModel;
using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct RenderAreaModel : INotifyPropertyChanged
    {
        private bool _useDefaultRenderArea;
        public bool UseDefaultRenderArea
        {
            get => _useDefaultRenderArea;
            set { _useDefaultRenderArea = value; PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(UseDefaultRenderArea))); }
        }
        public VkRect2D RenderArea { get; set; }
        public event PropertyChangedEventHandler PropertyChanged;
    }
}
