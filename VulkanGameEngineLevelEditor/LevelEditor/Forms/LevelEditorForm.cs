using GlmSharp;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.Models;
using static VulkanGameEngineLevelEditor.GameEngine.VulkanSystem;

namespace VulkanGameEngineLevelEditor.LevelEditor.Forms
{
    public unsafe partial class LevelEditorForm : Form
    {
        private volatile bool running;
        private volatile bool isResizing;
        private GCHandle _callbackHandle;
        private object lockObject = new object();
        private Thread renderThread { get; set; }

        private bool LeftMouseButtonDown { get; set; } = false;
        private Point LastMousePosition { get; set; }
        private vec2 SelectedGameObjectPosition { get; set; } = new vec2();
        private bool IsDragging { get; set; } = false;
        private uint SelectedSpriteIndex { get; set; } = uint.MaxValue;
        public static Guid GameObjectIdTexture { get; private set; } = new Guid("7047804f-d32e-4cb5-ba95-90783b28d1df");

        [DllImport("kernel32.dll")] static extern bool AllocConsole();
        public LevelEditorForm()
        {
            InitializeComponent();
            AllocConsole();
            GlobalMessenger.AddMessenger(new MessengerModel
            {
                richTextBox = VulkanLoggerBox,
                TextBoxName = VulkanLoggerBox.Name,
                ThreadId = Thread.CurrentThread.ManagedThreadId,
                IsActive = true
            });
            Thread.CurrentThread.Name = "LevelEditor";
            LogVulkanMessageDelegate callback = LogVulkanMessage;
            _callbackHandle = GCHandle.Alloc(callback);
            VulkanSystem.CreateLogMessageCallback(callback);

            this.Text = "Vulkan Level Editor - RenderPassEditorView";
        }

        public void LevelEditorForm_Load(object sender, EventArgs e)
        {
            StartRenderer();
        }

        public static void LogVulkanMessage(string message, int severity)
        {
            Console.WriteLine(message);
            GlobalMessenger.LogMessage(message, (DebugUtilsMessageSeverityFlagsEXT)severity);
        }

        public void StartRenderer()
        {
            running = true;
            renderThread = new Thread(RenderLoop)
            {
                IsBackground = true,
                Name = "VulkanRenderer"
            };
            renderThread.Start();
        }

        private void RenderLoop()
        {
            this.Invoke(new Action(() =>
            {
                GameSystem.StartUp(RenderBox.Handle.ToPointer(), RenderBox.Handle.ToPointer());

                List<RenderPassLoaderModel> renderPassLoaderList = new List<RenderPassLoaderModel>();
                //  GameObjectList = GameObjectSystem.GetGameObjectList().ToList();
                //levelEditorTreeView1.DynamicControlPanel = dynamicControlPanelView1;
                //   levelEditorTreeView1.PopulateWithGameObject(GameObjectList.First().GameObjectId);
            }));

            Stopwatch stopwatch = new Stopwatch();
            stopwatch.Start();
            double lastTime = 0.0;

            while (running)
            {
                if (isResizing)
                {
                    Thread.Sleep(10);
                    continue;
                }

                double currentTime = stopwatch.Elapsed.TotalSeconds;
                double deltaTime = currentTime - lastTime;
                lastTime = currentTime;

                GameSystem.Update((float)deltaTime);
                lock (lockObject)
                {
                    GameSystem.Draw((float)deltaTime);
                }
            }

            GameSystem.Destroy();
        }

        private void RendererBox_MouseDown(object sender, MouseEventArgs e)
        {
            Point currentPos = e.Location;
            if (e.Button == MouseButtons.Left)
            {
                uint pickedId = LevelEditorSystem.SampleRenderPassPixel(GameObjectIdTexture, new ivec2(currentPos.X, currentPos.Y));
                if (pickedId != uint.MaxValue)
                {
                    SelectedSpriteIndex = pickedId;
                    IsDragging = true;
                    LastMousePosition = currentPos;
                }
            }
            else if (e.Button == MouseButtons.Right)
            {
                IsDragging = true;
                LastMousePosition = currentPos;
            }
        }

        private void RendererBox_MouseWheel(object sender, MouseEventArgs e)
        {
            Point mousePos = e.Location;
            float scrollDelta = e.Delta / 1200.0f;
            ref var cameraTransform = ref CameraSystem.UpdateActiveCamera();
            cameraTransform.Zoom += scrollDelta;
        }

        private void RendererBox_MouseMove(object sender, MouseEventArgs e)
        {
            Point currentPos = e.Location;
            int deltaX = currentPos.X - LastMousePosition.X;
            int deltaY = currentPos.Y - LastMousePosition.Y;
            if (e.Button == MouseButtons.Left)
            {
                ref var transform = ref GameObjectSystem.UpdateGameObjectComponent<Transform2DComponent>(SelectedSpriteIndex, ComponentTypeEnum.kTransform2DComponent);
                transform.GameObjectPosition = new vec2(transform.GameObjectPosition.x + deltaX, transform.GameObjectPosition.y - deltaY);
            }
            else if (e.Button == MouseButtons.Right)
            {
                ref var cameraTransform = ref CameraSystem.UpdateActiveCamera();
                cameraTransform.Position = new vec3(cameraTransform.Position.x + deltaX, cameraTransform.Position.y - deltaY, 0.0f);
            }
            LastMousePosition = currentPos;
        }

        private void RendererBox_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                IsDragging = false;
                SelectedSpriteIndex = 0;
            }
            else if (e.Button == MouseButtons.Right)
            {
                IsDragging = false;
            }
        }

        private void listView1_SelectedIndexChanged(object sender, MouseEventArgs e)
        {

        }

        //private void AddListViewItem(string name, System.Drawing.Image icon, string assetType, object data)
        //{
        //    int idx = imageListPalette.Images.Add(icon ?? Properties.Resources.DefaultIcon, Color.Transparent);
        //    var item = new ListViewItem(name)
        //    {
        //        ImageIndex = idx,
        //        Tag = new DragAssetData
        //        {
        //            Type = assetType,
        //            Data = data
        //        }
        //    };

        //    lstPalette.Items.Add(item);
        //}
    }
}
