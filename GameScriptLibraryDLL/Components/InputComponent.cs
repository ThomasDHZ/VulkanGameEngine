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
        public const uint MAXMOUSEKEY = 3;

        public KeyState* KeyBoardState { get; set; }
        public float MouseX { get; set; } = 0.0f;
        public float MouseY { get; set; } = 0.0f;
        public float MouseDeltaX { get; set; } = 0.0f;
        public float MouseDeltaY { get; set; } = 0.0f;
        public bool* MouseButtons { get; set; }
        public float DeltaTime { get; set; } = 0.0f;
        public uint Sequence { get; set; } = 0;
        size_t KeyBoardButtonSize { get; set; }
        size_t MouseButtonKeySize { get; set; }

        public InputComponent()
        {
        }
    }
}
