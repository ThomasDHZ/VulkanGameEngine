using System;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor
{
    public class SystemMessenger : TextWriter
    {
        private readonly RichTextBox _richTextBox;
        public override Encoding Encoding => Encoding.UTF8;

        public SystemMessenger(RichTextBox richTextBox)
        {
            _richTextBox = richTextBox;
        }

        public override void Write(char value)
        {
            if (_richTextBox.InvokeRequired)
            {
                _richTextBox.Invoke(new Action(() => Write(value)));
            }
            else
            {
                _richTextBox.AppendText(value.ToString());
                _richTextBox.ScrollToCaret();
            }
        }

        public override void Write(string value)
        {
            if (_richTextBox.InvokeRequired)
            {
                _richTextBox.Invoke(new Action(() => Write(value)));
            }
            else
            {
                _richTextBox.AppendText(value);
                _richTextBox.ScrollToCaret();
            }
        }

        public override void WriteLine(string value)
        {
            Write(value + Environment.NewLine);
        }

        public void CompilerWriteLine(string value, string messageType)
        {
            if (messageType == "Error")
            {
                _richTextBox.ForeColor = Color.Red;
            }
            if (messageType == "Warning")
            {
                _richTextBox.ForeColor = Color.Yellow;
            }
            if(messageType == "Success")
            {
                _richTextBox.ForeColor = Color.Green;
            }
            Write($@"{messageType}:");
            _richTextBox.ForeColor = Color.White;
            Write(value + Environment.NewLine);
        }
    }
}
