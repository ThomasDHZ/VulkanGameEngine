using System;
using System.Drawing;
using System.Windows.Forms;
using Silk.NET.Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public static class MessageLogger
    {
        public static RichTextBox RichTextBox { get; set; }

        private static readonly object _lock = new object();

        public static void LogMessage(string message, DebugUtilsMessageSeverityFlagsEXT severity)
        {
            if (RichTextBox == null) return;
            if (RichTextBox.InvokeRequired)
            {
                RichTextBox.BeginInvoke(new Action<string, DebugUtilsMessageSeverityFlagsEXT>(AppendMessage), message, severity);
                return;
            }
            AppendMessage(message, severity);
        }

        private static void AppendMessage(string message, DebugUtilsMessageSeverityFlagsEXT severity)
        {
            if (RichTextBox == null) return;

            string prefix = severity switch
            {
                DebugUtilsMessageSeverityFlagsEXT.ErrorBitExt => "[ERROR] ",
                DebugUtilsMessageSeverityFlagsEXT.WarningBitExt => "[WARN]  ",
                DebugUtilsMessageSeverityFlagsEXT.InfoBitExt => "[INFO]  ",
                DebugUtilsMessageSeverityFlagsEXT.VerboseBitExt => "[VERBOSE] ",
                _ => "[UNKNOWN] "
            };

            Color color = severity switch
            {
                DebugUtilsMessageSeverityFlagsEXT.ErrorBitExt => Color.Red,
                DebugUtilsMessageSeverityFlagsEXT.WarningBitExt => Color.Orange,
                DebugUtilsMessageSeverityFlagsEXT.InfoBitExt => Color.LimeGreen,
                DebugUtilsMessageSeverityFlagsEXT.VerboseBitExt => Color.LightBlue,
                _ => Color.White
            };

            RichTextBox.SelectionStart = RichTextBox.TextLength;
            RichTextBox.SelectionLength = 0;

            RichTextBox.SelectionColor = color;
            RichTextBox.SelectionFont = new Font(RichTextBox.Font, FontStyle.Bold);
            RichTextBox.AppendText(prefix);

            RichTextBox.SelectionColor = Color.White;
            RichTextBox.SelectionFont = new Font(RichTextBox.Font, FontStyle.Regular);
            RichTextBox.AppendText(message.Trim() + Environment.NewLine);

            RichTextBox.ScrollToCaret();
        }

        public static void Clear()
        {
            if (RichTextBox == null) return;
            if (RichTextBox.InvokeRequired)
            {
                RichTextBox.BeginInvoke(new Action(Clear));
                return;
            }
            RichTextBox.Clear();
        }
    }
}