using System.Collections.Generic;
using System.Linq;
using System.Threading;
using VulkanGameEngineLevelEditor.Models;
using Silk.NET.Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public static class GlobalMessenger
    {
        private static readonly List<(string Message, DebugUtilsMessageSeverityFlagsEXT Severity)> _messageQueue = new List<(string, DebugUtilsMessageSeverityFlagsEXT)>();
        private static readonly object _lockObject = new();
        private static readonly ManualResetEvent _messageAvailable = new ManualResetEvent(false);
        private static readonly List<MessengerModel> _messengers = new List<MessengerModel>();

        static GlobalMessenger()
        {
            Thread messageThread = new Thread(ProcessMessages)
            {
                IsBackground = true
            };
            messageThread.Start();
        }

        public static void AddMessenger(MessengerModel model)
        {
            lock (_lockObject)
            {
                _messengers.Add(model);
            }
        }

        public static void LogMessage(string message, DebugUtilsMessageSeverityFlagsEXT severity)
        {
            lock (_lockObject)
            {
                _messageQueue.Add((message, severity));
                _messageAvailable.Set();
            }
        }

        private static void ProcessMessages()
        {
            while (true)
            {
                _messageAvailable.WaitOne();
                (string Message, DebugUtilsMessageSeverityFlagsEXT Severity)[] messages;
                lock (_lockObject)
                {
                    messages = _messageQueue.ToArray();
                    _messageQueue.Clear();
                    if (_messageQueue.Count == 0)
                    {
                        _messageAvailable.Reset();
                    }
                }

                var activeMessenger = _messengers.FirstOrDefault(m => m.IsActive);
                if (activeMessenger != null)
                {
                    foreach (var (message, severity) in messages)
                    {
                        activeMessenger.LogMessage(message, severity);
                    }
                }
            }
        }
    }
}