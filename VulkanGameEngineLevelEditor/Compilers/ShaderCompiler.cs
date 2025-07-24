using CSScripting;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor;

namespace VulkanGameEngineLevelEditor.Compilers
{
    public static class ShaderCompiler
    {
        public static SystemMessenger systemMessenger { get; set; }
        public static bool CompileShader(string shaderFilePath, string shaderFile)
        {
            string ext = shaderFile.GetExtension();
            string args = $"--target-env=vulkan1.4 --target-spv=spv1.6 {shaderFilePath}/{shaderFile} -o {shaderFilePath}{shaderFile.GetFileName()}{ext}.spv";

            if(systemMessenger == null)
            {
                MessageBox.Show($"Messager must be set.");
                return false;
            }

            if (!File.Exists($"{shaderFilePath}/{shaderFile}"))
            {
                MessageBox.Show($"Shader file not found: {shaderFilePath}/{shaderFile}");
                return false;
            }

            try
            {
                systemMessenger.WriteLine($@"Building {shaderFile}:");

                ProcessStartInfo startInfo = new ProcessStartInfo
                {
                    FileName = ConstConfig.ShaderCompilerPath,
                    Arguments = args,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = true
                };

                using (Process process = new Process { StartInfo = startInfo })
                {
                    process.Start();
                    string output = process.StandardOutput.ReadToEnd();
                    string error = process.StandardError.ReadToEnd();
                    process.WaitForExit();

                    string result = output + error;
                    if (process.ExitCode != 0)
                    {
                        systemMessenger.CompilerWriteLine(result, "Error");
                        MessageBox.Show($"Compilation failed with error:\n{result}");
                        return false;
                    }
                    else
                    {
                        systemMessenger.CompilerWriteLine(result, "Success");
                        return true;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error: {ex.Message}");
                return false;
            }
        }

        public static bool CompileAllShaders(string shaderFileDirectory)
        {
            if (systemMessenger == null)
            {
                MessageBox.Show($"Messager must be set.\n");
                return false;
            }

            string[] fileList = Directory.GetFiles(shaderFileDirectory);
            systemMessenger.WriteLine($@"Building on directory: {shaderFileDirectory}:");
            foreach (string file in fileList)
            {
                if (file.GetExtension() == ".vert" ||
                    file.GetExtension() == ".frag")
                {
                    CompileShader(shaderFileDirectory, file.GetFileName());
                }
            }
            return true;
        }

        private static void MessageFormator()
        {

        }
    }
}
