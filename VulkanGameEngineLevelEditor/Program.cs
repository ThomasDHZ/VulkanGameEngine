using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor
{
    public class Program
    {
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new LevelEditorForm());
        }
    }
}