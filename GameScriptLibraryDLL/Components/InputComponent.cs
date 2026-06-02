using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.Components
{
    public unsafe struct InputComponent
    {
        public const uint MAXKEYBOARDKEY = 350;
        public KeyState* KeyBoardState { get; set; }
        public size_t KeyBoardSize { get; set; }

        public InputComponent()
        {
        }

    }
}
