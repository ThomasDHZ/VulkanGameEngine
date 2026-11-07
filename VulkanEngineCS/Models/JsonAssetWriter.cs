using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Models
{
    public class JsonAssetWriter<T>
    {
        public string JsonWriteLocation { get; set; } = string.Empty;
        public T JsonWriteObject { get; set; }
        public JsonAssetWriter()
        {
        }

    }
}
