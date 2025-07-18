using Newtonsoft.Json;
using System;
using System.ComponentModel;
using System.IO;

namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public abstract class RenderPassEditorBaseModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public RenderPassEditorBaseModel()
        {

        }

        protected virtual T LoadJsonComponent<T>(string jsonPath)
        {
            string jsonContent = File.ReadAllText(jsonPath);
            return JsonConvert.DeserializeObject<T>(jsonContent);
        }

        protected virtual void SaveJsonComponent(string jsonPath, object obj)
        {
            string jsonString = JsonConvert.SerializeObject(obj, Formatting.Indented);
            File.WriteAllText(jsonPath, jsonString);
        }

        public virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
