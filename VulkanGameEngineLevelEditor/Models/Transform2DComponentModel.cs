using GlmSharp;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.GameEngine.GameObjectComponents
{
    public class Transform2DComponentModel : INotifyPropertyChanged
    {
        private string name = "Transform2DComponent";
        private Transform2DComponent transform = new Transform2DComponent();

        public event PropertyChangedEventHandler PropertyChanged;

        [Category("Transform")]
        [Tooltip("The name of this transform component.")]
        public string Name
        {
            get => name;
            set
            {
                if (name != value)
                {
                    name = value;
                    OnPropertyChanged();
                }
            }
        }

        [Category("Transform")]
        [Tooltip("The position of the game object in 2D space.")]
        public Transform2DComponent Transform
        {
            get => transform;
            set
            {
                transform = value;
                OnPropertyChanged();
            }
        }

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}