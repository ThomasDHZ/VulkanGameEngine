using CSScripting;
using GlmSharp;
using Newtonsoft.Json;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;
using VulkanGameEngineLevelEditor.GameEngine;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.Models;
using static System.Runtime.InteropServices.JavaScript.JSType;
using static VulkanGameEngineLevelEditor.GameEngine.VulkanSystem;
using Point = System.Drawing.Point;

namespace VulkanGameEngineLevelEditor.LevelEditor.Forms
{

    public unsafe partial class LevelEditorForm : Form
    {
        public enum AssetDataTypeEnum
        {
            kAssetTypeGameObject,
            kAssetTypeMaterial,
            kAssetTypeTexture
        };

        public class DragAssetData
        {
            public AssetDataTypeEnum AssetType { get; set; }
            public System.String JsonPath { get; set; }
        };
        private volatile bool running;
        private volatile bool isResizing;
        private GCHandle _callbackHandle;
        private object lockObject = new object();
        private System.Threading.Thread renderThread { get; set; }

        private bool LeftMouseButtonDown { get; set; } = false;
        private System.Drawing.Point LastMousePosition { get; set; }
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
                ThreadId = System.Threading.Thread.CurrentThread.ManagedThreadId,
                IsActive = true
            });
            System.Threading.Thread.CurrentThread.Name = "LevelEditor";
            LogVulkanMessageDelegate callback = LogVulkanMessage;
            _callbackHandle = GCHandle.Alloc(callback);
            VulkanSystem.CreateLogMessageCallback(callback);

            this.Text = "Vulkan Level Editor - RenderPassEditorView";

            List<System.String> gameObjectPrefabList = Directory.GetFiles(@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\GameObjects").ToList();
            foreach (var gameObjectPrefab in gameObjectPrefabList)
            {
                AddListItem(gameObjectPrefab.GetFileName(), AssetDataTypeEnum.kAssetTypeGameObject, gameObjectPrefab);
            }
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
            renderThread = new System.Threading.Thread(RenderLoop)
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

               // List<RenderPassLoaderModel> renderPassLoaderList = new List<RenderPassLoaderModel>();
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
                    System.Threading.Thread.Sleep(10);
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
            System.Drawing.Point currentPos = e.Location;
            if (e.Button == MouseButtons.Left)
            {
                ivec2 worldPos = ClientToWorld(currentPos);
                string a = @$"X: {worldPos.x} \n Y: {worldPos.y}";
                MessageBox.Show(a);

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
            System.Drawing.Point mousePos = e.Location;
            float scrollDelta = e.Delta / 1200.0f;
            ref var cameraTransform = ref CameraSystem.UpdateActiveCamera();
            cameraTransform.Zoom -= scrollDelta;
        }

        private void RendererBox_MouseMove(object sender, MouseEventArgs e)
        {
            if (SelectedSpriteIndex == uint.MaxValue) return;

            System.Drawing.Point currentPos = e.Location;
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

        private void AddListItem(string name, AssetDataTypeEnum assetType, System.String jsonPath)
        {
            System.Drawing.Image displayIcon = SystemIcons.Application.ToBitmap();
            int index = imageList1.Images.Add(displayIcon, System.Drawing.Color.Transparent);
            var item = new ListViewItem(name)
            {
                ImageIndex = index,
                Tag = new DragAssetData
                {
                    AssetType = assetType,
                    JsonPath = jsonPath
                }
            };
            GameObjectListView.Items.Add(item);
        }

        private void RenderBox_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(typeof(DragAssetData)))
            {
                e.Effect = DragDropEffects.Copy;
            }
            else
            {
                e.Effect = DragDropEffects.None;
            }
        }

        private void RenderBox_DragDrop(object sender, DragEventArgs e)
        {
            if (e.Data.GetData(typeof(DragAssetData)) is not DragAssetData asset ||
                asset.AssetType != AssetDataTypeEnum.kAssetTypeGameObject)
                return;

            Point clientPos = RenderBox.PointToClient(new Point(e.X, e.Y));
            ivec2 worldPos = ClientToWorld(clientPos);

            ivec2 spriteSize = new ivec2(464, 688);

            // Diagnostic version - let's try negative Y offset
            worldPos.x += spriteSize.x / 2;
            worldPos.y += 0;

            uint newGoId = GameObjectSystem.CreateGameObject(asset.JsonPath, worldPos);
        }

        private void GameObjectListView_ItemDrag(object sender, ItemDragEventArgs e)
        {
            if (e.Item is ListViewItem item && item.Tag is DragAssetData asset)
            {
                GameObjectListView.DoDragDrop(asset, DragDropEffects.Copy);
            }
        }

        public ivec2 ClientToWorld(System.Drawing.Point clientPos)
        {
            ref Camera camera = ref CameraSystem.UpdateActiveCamera();

            float centerX = RenderBox.ClientSize.Width * 0.5f;
            float centerY = RenderBox.ClientSize.Height * 0.5f;
            float worldX = (clientPos.X - centerX) * camera.Zoom + camera.Position.x;
            float worldY = (clientPos.Y - centerY) * camera.Zoom + camera.Position.y;
            return new ivec2((int)Math.Round(worldX), (int)Math.Round(worldY));
        }

        public System.Drawing.Point WorldToClient(vec2 worldPos)
        {
            ref Camera camera = ref CameraSystem.UpdateActiveCamera();

            float centerX = RenderBox.ClientSize.Width * 0.5f;
            float centerY = RenderBox.ClientSize.Height * 0.5f;
            float clientX = (worldPos.x - camera.Position.x) / camera.Zoom + centerX;
            float clientY = (worldPos.y - camera.Position.y) / camera.Zoom + centerY;
            return new System.Drawing.Point((int)MathF.Round(clientX), (int)MathF.Round(clientY));
        }
    }
}
