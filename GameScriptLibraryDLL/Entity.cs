using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL
{
    public class Entity
    {
        public EntityHandle Id { get; internal set; }
        public static T GetComponent<T>() 
        {
            T a = default(T);
            return a;
        }
    }
}
