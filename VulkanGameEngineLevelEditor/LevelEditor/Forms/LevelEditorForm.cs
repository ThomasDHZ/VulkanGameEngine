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
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
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
            kAssetTypeTexture,
            kAssetTypeScene,
            kAssetTypeLight
        };

        public class DragAssetData
        {
            public AssetDataTypeEnum AssetType { get; set; }
            public System.String JsonPath { get; set; }
        };

        private float KeyBoardCameraSpeed = 25.0f;
        private volatile bool running;
        private volatile bool isResizing;
        private GCHandle _callbackHandle;
        private object lockObject = new object();
        private System.Threading.Thread renderThread { get; set; }
        private ivec2 RenderResolution = new ivec2(3840, 2160);
        private bool LeftMouseButtonDown { get; set; } = false;
        private System.Drawing.Point LastMousePosition { get; set; }
        private vec2 SelectedGameObjectPosition { get; set; } = new vec2();
        private bool IsDragging { get; set; } = false;
        private uint SelectedSpriteIndex { get; set; } = uint.MaxValue;
        public static Guid GameObjectIdTexture { get; private set; } = new Guid("7047804f-d32e-4cb5-ba95-90783b28d1df");

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool AllocConsole();

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetStdHandle(int nStdHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool SetStdHandle(int nStdHandle, IntPtr hHandle);

        private const int STD_OUTPUT_HANDLE = -11;
        private const int STD_ERROR_HANDLE = -12;

        private static void InitializeConsole()
        {
            if (!AllocConsole())
            {
                return;
            }

            try
            {
                IntPtr outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
                IntPtr errHandle = GetStdHandle(STD_ERROR_HANDLE);

                var stdout = new System.IO.FileStream(outHandle, System.IO.FileAccess.Write, false);
                var stderr = new System.IO.FileStream(errHandle, System.IO.FileAccess.Write, false);

                var writerOut = new System.IO.StreamWriter(stdout) { AutoFlush = true };
                var writerErr = new System.IO.StreamWriter(stderr) { AutoFlush = true };

                Console.SetOut(writerOut);
                Console.SetError(writerErr);

                Console.WriteLine("=== Console successfully initialized ===");
                Console.WriteLine("Console output should now work from all threads.");
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to initialize console redirection:\n{ex.Message}");
            }
        }

        public LevelEditorForm()
        {
            InitializeConsole();
            InitializeComponent();
            MessageLogger.RichTextBox = VulkanLoggerBox;

            System.Threading.Thread.CurrentThread.Name = "LevelEditor";

            LogVulkanMessageDelegate callback = LogVulkanMessage;
            _callbackHandle = GCHandle.Alloc(callback);
            VulkanSystem.CreateLogMessageCallback(callback);

            this.Text = "Vulkan Level Editor - RenderPassEditorView";

            List<System.String> gameObjectPrefabList = Directory.GetFiles(@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\GameObjects").ToList();
            foreach (var gameObjectPrefab in gameObjectPrefabList)
            {
                AddListItem(GameObjectListView, gameObjectPrefab.GetFileName(), AssetDataTypeEnum.kAssetTypeGameObject, gameObjectPrefab);
            }

            List<System.String> sceneLevelList = Directory.GetFiles(@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\Levels").ToList();
            foreach (var sceneLevel in sceneLevelList)
            {
                AddListItem(SceneListView, sceneLevel.GetFileName(), AssetDataTypeEnum.kAssetTypeScene, sceneLevel);
            }

            List<System.String> lightList = Directory.GetFiles(@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\Lights").ToList();
            foreach (var light in lightList)
            {
                AddListItem(LightListView, light.GetFileName(), AssetDataTypeEnum.kAssetTypeLight, light);
            }

            RenderBox.KeyDown += RenderBox_KeyDown;
            RenderBox.KeyUp += RenderBox_KeyUp;

            System.Drawing.Icon saveIcon = SystemIcons.GetStockIcon(StockIconId.Drive35);
            SaveButton.Image = saveIcon.ToBitmap();
            SaveButton.ImageAlign = ContentAlignment.MiddleCenter;
            SaveButton.TextImageRelation = TextImageRelation.ImageBeforeText;
        }

        public void LevelEditorForm_Load(object sender, EventArgs e)
        {
            StartRenderer();
        }

        public static void LogVulkanMessage(string message, int severity)
        {
            Console.WriteLine(message);
            MessageLogger.LogMessage(message, (DebugUtilsMessageSeverityFlagsEXT)severity);
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
            }));

            Stopwatch stopwatch = new Stopwatch();
            stopwatch.Start();
            double lastTime = 0.0;

            treeView1.PopulateWithGameObjects();
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
            RenderBox.Focus();
            if (e.Button != MouseButtons.Left)
            {
                IsDragging = true;
                LastMousePosition = e.Location;
                return;
            }

            Point controlPos = e.Location;

            float scaleX = (float)RenderResolution.x / RenderBox.ClientSize.Width;
            float scaleY = (float)RenderResolution.y / RenderBox.ClientSize.Height;

            int sampleX = (int)(controlPos.X * scaleX);
            int sampleY = (int)(controlPos.Y * scaleY);

            uint pickedId = LevelEditorSystem.SampleRenderPassPixel(GameObjectIdTexture, new ivec2(sampleX, sampleY));

            if (pickedId != uint.MaxValue)
            {
                SelectedSpriteIndex = pickedId;
                propertiesPanel1.SetSelectedEntity(pickedId);
                treeView1.SelectGameObject(pickedId);

                IsDragging = true;
                LastMousePosition = controlPos;
            }
        }

        private void RendererBox_MouseWheel(object sender, MouseEventArgs e)
        {
            //System.Drawing.Point mousePos = e.Location;
            //float scrollDelta = e.Delta / 1200.0f;
            //ref var cameraTransform = ref CameraSystem.UpdateActiveCamera();
            //cameraTransform.Zoom += scrollDelta;
        }

        private void RendererBox_MouseMove(object sender, MouseEventArgs e)
        {
            if (SelectedSpriteIndex == uint.MaxValue) return;

            Point currentPos = e.Location;

            if (e.Button == MouseButtons.Left)
            {
                int deltaX = currentPos.X - LastMousePosition.X;
                int deltaY = currentPos.Y - LastMousePosition.Y;

                ref var transform = ref GameObjectSystem.UpdateGameObjectComponent<Transform2DComponent>(SelectedSpriteIndex, ComponentTypeEnum.kTransform2DComponent);

                transform.GameObjectPosition = new vec2(transform.GameObjectPosition.x + deltaX, transform.GameObjectPosition.y - deltaY);

                // if (Math.Abs(deltaX) > 1 || Math.Abs(deltaY) > 1)
            }
            else if (e.Button == MouseButtons.Right)
            {
                int deltaX = currentPos.X - LastMousePosition.X;
                int deltaY = currentPos.Y - LastMousePosition.Y;

                ref var cameraTransform = ref CameraSystem.UpdateActiveCamera();
                cameraTransform.Position = new vec3(cameraTransform.Position.x - deltaX, cameraTransform.Position.y + deltaY, 0.0f);
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

        private void AddListItem(ListView listView, string name, AssetDataTypeEnum assetType, System.String jsonPath, System.Drawing.Image icon = null)
        {
            System.Drawing.Image displayIcon = icon == null ? SystemIcons.Application.ToBitmap() : icon;
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
            listView.Items.Add(item);
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
            if (e.Data.GetData(typeof(DragAssetData)) is not DragAssetData asset) return;

            Point clientPos = RenderBox.PointToClient(new Point(e.X, e.Y));
            ivec2 worldPos = ClientToWorld(clientPos);
            ivec2 dropPos = new ivec2(worldPos.x / 2, worldPos.y / 2);
            if (asset.AssetType == AssetDataTypeEnum.kAssetTypeGameObject)
            {
                uint newGoId = GameObjectSystem.CreateGameObject(asset.JsonPath, dropPos);
                treeView1.AddGameObject(newGoId);
            }
            else if(asset.AssetType == AssetDataTypeEnum.kAssetTypeLight)
            {
                uint newLightId = LightSystem.LoadLight(asset.JsonPath);
                treeView1.AddDirectionalLightObject(newLightId);
            }
        }

        private void GameObjectListView_ItemDrag(object sender, ItemDragEventArgs e)
        {
            if (e.Item is ListViewItem item && item.Tag is DragAssetData asset)
            {
                GameObjectListView.DoDragDrop(asset, DragDropEffects.Copy);
            }
        }

        private void LightListView_ItemDrag(object sender, ItemDragEventArgs e)
        {
            if (e.Item is ListViewItem item && item.Tag is DragAssetData asset)
            {
                GameObjectListView.DoDragDrop(asset, DragDropEffects.Copy);
            }
        }

        public ivec2 ClientToWorld(System.Drawing.Point clientPos)
        {
            ref Camera camera = ref CameraSystem.UpdateActiveCamera();

            float camX = camera.Position.x;
            float camY = camera.Position.y;

            float scaleX = RenderResolution.x / (float)RenderBox.ClientSize.Width;
            float scaleY = RenderResolution.y / (float)RenderBox.ClientSize.Height;

            float renderX = clientPos.X * scaleX;
            float renderY = (RenderBox.ClientSize.Height - clientPos.Y) * scaleY;

            float worldX = camX + renderX;
            float worldY = camY + renderY;

            return new ivec2((int)Math.Round(worldX), (int)Math.Round(worldY));
        }

        public System.Drawing.Point WorldToClient(vec2 worldPos)
        {
            ref Camera camera = ref CameraSystem.UpdateActiveCamera();

            float clientX = (worldPos.x - camera.Position.x) / camera.Zoom;
            float clientY = (worldPos.y - camera.Position.y) / camera.Zoom;

            clientY = RenderBox.ClientSize.Height - clientY;

            return new System.Drawing.Point((int)MathF.Round(clientX), (int)MathF.Round(clientY));
        }

        private void treeView1_AfterSelect(object sender, TreeViewEventArgs e)
        {
            int a = 34;
        }

        private void RenderBox_KeyDown(object sender, KeyEventArgs e)
        {
            ref var cameraTransform = ref CameraSystem.UpdateActiveCamera();
            if (e.KeyCode == Keys.W) cameraTransform.Position = new vec3(cameraTransform.Position.x, cameraTransform.Position.y + KeyBoardCameraSpeed, 0.0f);
            if (e.KeyCode == Keys.A) cameraTransform.Position = new vec3(cameraTransform.Position.x - KeyBoardCameraSpeed, cameraTransform.Position.y, 0.0f);
            if (e.KeyCode == Keys.D) cameraTransform.Position = new vec3(cameraTransform.Position.x + KeyBoardCameraSpeed, cameraTransform.Position.y, 0.0f);
            if (e.KeyCode == Keys.S) cameraTransform.Position = new vec3(cameraTransform.Position.x, cameraTransform.Position.y - KeyBoardCameraSpeed, 0.0f);
            if ((e.KeyCode == Keys.Delete || e.KeyCode == Keys.Back) &&
                SelectedSpriteIndex != uint.MaxValue)
            {
                GameObjectSystem.DestroyGameObject(SelectedSpriteIndex);
            }
        }

        private void RenderBox_KeyUp(object sender, KeyEventArgs e)
        {
        }

        private void SaveButton_Click(object sender, EventArgs e)
        {

        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
    }
}
