using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL
{
    public interface IScript
    {
        void StartUp(EntityHandle entity, vec2 startPosition, vec2 rotation);
        void Update(float deltaTime);
    }
}
