using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine;
using VulkanGameEngineLevelEditor.LevelEditor.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.Registries;

namespace VulkanGameEngineLevelEditor
{
    internal static class Program
    {
        [STAThread]
        private static void Main()
        {
            try
            {
                ComponentRegistry.Initialize();
                LightRegistry.Initialize();
                ObjectLinkerRegistry.Initialize();
                DLLSystem.SetSharedDllDirectory();
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.Run(new LevelEditorForm());
            }
            catch (Exception ex)
            {
                MessageBox.Show(
                    $"Startup error: {ex.Message}",
                    "Fatal",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);
            }
        }
    }
}