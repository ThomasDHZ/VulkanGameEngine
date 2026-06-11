using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace MaterialBaker
{
    public class MaterialMemoryPoolSystem
    {
        [DllImport(DLLSystem.MaterialBakerDLL, CallingConvention = CallingConvention.StdCall)] private static extern MaterialMemoryPoolSystem_StartUp();
    }
}
