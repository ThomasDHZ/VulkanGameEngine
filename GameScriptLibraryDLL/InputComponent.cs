using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL
{
    public class InputComponent
    {
        public const uint MAXKEYBOARDKEY = 350;
        public KeyState[] KeyBoardState { get; set; } = new KeyState[MAXKEYBOARDKEY];
    }
}
