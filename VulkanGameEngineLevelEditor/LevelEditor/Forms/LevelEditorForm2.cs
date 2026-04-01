//using GlmSharp;
//using Microsoft.CodeAnalysis.CSharp.Syntax;
//using Newtonsoft.Json;
//using Silk.NET.Vulkan;
//using System;
//using System.Collections.Concurrent;
//using System.Collections.Generic;
//using System.Diagnostics;
//using System.Drawing;
//using System.IO;
//using System.Linq;
//using System.Net.Http.Json;
//using System.Numerics;
//using System.Runtime.InteropServices;
//using System.Threading;
//using System.Windows.Forms;
//using System.Xml.Linq;
//using Vulkan;
//using VulkanGameEngineLevelEditor.Compilers;
//using VulkanGameEngineLevelEditor.GameEngine;
//using VulkanGameEngineLevelEditor.GameEngine.Structs;
//using VulkanGameEngineLevelEditor.LevelEditor;
//using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
//using VulkanGameEngineLevelEditor.Models;
//using static System.Runtime.InteropServices.JavaScript.JSType;
//using static System.Windows.Forms.LinkLabel;
//using static System.Windows.Forms.VisualStyles.VisualStyleElement;

//namespace VulkanGameEngineLevelEditor
//{
//    public enum LevelEditorModeEnum
//    {
//        kLevelEditorMode,
//        kRenderPassEditorMode
//    }

//    public unsafe partial class LevelEditorForm : Form
//    {
//        LevelEditorModeEnum LevelEditorMode = LevelEditorModeEnum.kLevelEditorMode;
//        private volatile bool running;
//        private volatile bool levelEditorRunning;
//        private volatile bool isResizing;
//        private Stopwatch stopwatch = new Stopwatch();
//        //  public SystemMessenger textBoxWriter;
//        private Thread renderThread { get; set; }
//        private MessengerModel _messenger;
//        private GCHandle _callbackHandle;

//        private object lockObject = new object();
//        private object sharedData;
//        public List<System.String> ShaderList = new List<string>();
//        public static Guid GameObjectIdTexture { get; private set; } = new Guid("7047804f-d32e-4cb5-ba95-90783b28d1df");

//        private bool LeftMouseButtonDown { get; set; } = false;
//        private Point LastMousePosition { get; set; }
//        private vec2 SelectedGameObjectPosition { get; set; } = new vec2();
//        private bool IsDragging { get; set; } = false;
//        private uint SelectedSpriteIndex { get; set; } = uint.MaxValue;
//        public List<GameObject> GameObjectList { get; set; }


//        // BlockingCollection<Dictionary<int, GameObject>> gameObjectData = new BlockingCollection<Dictionary<int, GameObject>>();
//        [DllImport("kernel32.dll")] static extern bool AllocConsole();

//        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void LogVulkanMessageDelegate(string message, int severity);

//        public LevelEditorForm()
//        {
//            InitializeComponent();
//            AllocConsole();
//            GlobalMessenger.AddMessenger(new MessengerModel
//            {
//                richTextBox = richTextBox2,
//                TextBoxName = richTextBox2.Name,
//                ThreadId = Thread.CurrentThread.ManagedThreadId,
//                IsActive = true
//            });

//            this.Load += Form1_Load;

//            Thread.CurrentThread.Name = "LevelEditor";

//            //    textBoxWriter = new SystemMessenger(richTextBox2);
//            //     ShaderCompiler.systemMessenger = textBoxWriter;


//            string originalDir = Directory.GetCurrentDirectory();

//            LogVulkanMessageDelegate callback = LogVulkanMessage;
//            _callbackHandle = GCHandle.Alloc(callback);
//            VulkanSystem.CreateLogMessageCallback(callback);

//            this.Text = "Vulkan Level Editor - RenderPassEditorView";
//        }

//        private void Form1_Load(object sender, EventArgs e)
//        {
//            StartRenderer();
//        }

//        public static void LogVulkanMessage(string message, int severity)
//        {
//            Console.WriteLine(message);
//            GlobalMessenger.LogMessage(message, (DebugUtilsMessageSeverityFlagsEXT)severity);
//        }

//        public void StartRenderer()
//        {
//            running = true;
//            renderThread = new Thread(RenderLoop)
//            {
//                IsBackground = true,
//                Name = "VulkanRenderer"
//            };
//            renderThread.Start();
//        }

//        private void RenderLoop()
//        {
//            this.Invoke(new Action(() =>
//            {
//                void* afds = this.RendererBox.Handle.ToPointer();
//                // GameSystem.StartUp(this.RendererBox.Handle.ToPointer(), this.richTextBox2.Handle.ToPointer());

//                List<RenderPassLoaderModel> renderPassLoaderList = new List<RenderPassLoaderModel>();
//                GameObjectList = GameObjectSystem.GetGameObjectList().ToList();
//                //levelEditorTreeView1.DynamicControlPanel = dynamicControlPanelView1;
//                //   levelEditorTreeView1.PopulateWithGameObject(GameObjectList.First().GameObjectId);
//            }));

//            Stopwatch stopwatch = new Stopwatch();
//            stopwatch.Start();
//            double lastTime = 0.0;

//            while (running)
//            {
//                if (isResizing)
//                {
//                    Thread.Sleep(10);
//                    continue;
//                }

//                double currentTime = stopwatch.Elapsed.TotalSeconds;
//                double deltaTime = currentTime - lastTime;
//                lastTime = currentTime;

//                GameSystem.Update((float)deltaTime);
//                lock (lockObject)
//                {
//                    GameSystem.Draw((float)deltaTime);
//                }
//            }

//            GameSystem.Destroy();
//        }

//        private void AddGameObject(GameObject gameObject)
//        {

//        }

//        private void toolStripButton1_Click(object sender, EventArgs e)
//        {
//            LevelEditorMode = LevelEditorModeEnum.kLevelEditorMode;

//            this.Text = "Vulkan Level Editor - LevelEditorView";
//            //levelEditorTreeView1.RootObject = new GameObject();
//            //levelEditorTreeView1.dynamicControlPanelView = dynamicControlPanelView1;
//        }
//        private void toolStripButton2_Click(object sender, EventArgs e)
//        {
//            LevelEditorMode = LevelEditorModeEnum.kRenderPassEditorMode;

//            this.Text = "Vulkan Level Editor - RenderPassEditorView";
//            //levelEditorTreeView1.RootObject = new RenderPassLoaderModel(@"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\RenderPass\DefaultRenderPass.json");
//            //levelEditorTreeView1.dynamicControlPanelView = dynamicControlPanelView1;
//        }

//        private void SaveRenderPass_Click(object sender, EventArgs e)
//        {
//            //var a = JsonConvert.SerializeObject(renderPass, Formatting.Indented);
//            //var ab = 32;
//        }

//        private void richTextBox1_TextChanged(object sender, EventArgs e)
//        {

//        }

//        private void SaveLevel_Click(object sender, EventArgs e)
//        {

//        }

//        private void levelEditorTreeView1_AfterSelect(object sender, TreeViewEventArgs e)
//        {

//        }

//        private void dynamicControlPanelView1_Paint(object sender, PaintEventArgs e)
//        {

//        }

//        private void dynamicControlPanelView1_Paint_1(object sender, PaintEventArgs e)
//        {

//        }

//        private void levelEditorTreeView1_AfterSelect_1(object sender, TreeViewEventArgs e)
//        {

//        }

//        private void LevelEditorForm_Load(object sender, EventArgs e)
//        {

//        }

//        private void LevelEditorForm_Resize(object sender, EventArgs e)
//        {
//            ResizeRenderer();
//        }

//        private void ResizeRenderer()
//        {
//            if (running && !this.WindowState.HasFlag(FormWindowState.Minimized))
//            {
//                lock (lockObject)
//                {
//                    isResizing = true;
//                    RenderSystem.RebuildRendererFlag = true;
//                    // RenderSystem.RecreateSwapchain(LevelSystem.spriteRenderPass2DId, LevelSystem.levelLayout.LevelLayoutId, 0.0f, new GlmSharp.ivec2(RendererBox.Width, RendererBox.Height));
//                    isResizing = false;
//                }
//            }
//        }

//        private void RendererBox_Resize(object sender, EventArgs e)
//        {
//            ResizeRenderer();
//        }

//        private void buildShadersToolStripMenuItem_Click(object sender, EventArgs e)
//        {
//            //ShaderCompiler.CompileAllShaders($@"{ConstConfig.BaseDirectoryPath}Shaders");
//        }

//        public void QuickUpdateRenderPass()
//        {
//            //try
//            //{
//            //    if (levelEditorTreeView1.r is GameObject)
//            //    {
//            //        int a = 34;
//            //    }
//            //    else if (levelEditorTreeView1.RootObject is RenderPassLoaderModel ||
//            //             levelEditorTreeView1.RootObject is List<RenderPassLoaderModel>)
//            //    {
//            //        var renderPassJsonMap = new Dictionary<Guid, string>();
//            //        var pipelineJsonMap = new Dictionary<Guid, List<string>>();
//            //        foreach (var renderPassJsonModel in levelEditorTreeView1.RootObject as List<RenderPassLoaderModel>)
//            //        {
//            //            var pipelineJsonList = new List<string>();
//            //            renderPassJsonMap[renderPassJsonModel.RenderPassId] = JsonConvert.SerializeObject(renderPassJsonModel);
//            //            foreach (var pipelineModel in renderPassJsonModel.renderPipelineModelList)
//            //            {
//            //                pipelineJsonList.Add(JsonConvert.SerializeObject(pipelineModel));
//            //            }
//            //            pipelineJsonMap[renderPassJsonModel.RenderPassId] = pipelineJsonList;
//            //        }

//            //        if (running && !this.WindowState.HasFlag(FormWindowState.Minimized))
//            //        {
//            //            lock (lockObject)
//            //            {
//            //                isResizing = true;
//            //                RenderSystem.RebuildRendererFlag = true;
//            //                // RenderSystem.UpdateRenderPasses(renderPassJsonMap, pipelineJsonMap, ShaderSystem.GetGlobalShaderPushConstant("sceneData"));
//            //                isResizing = false;
//            //            }
//            //        }
//            //    }
//            //}
//            //catch (Exception ex)
//            //{
//            //    MessageBox.Show(@$"Failed to build renderpass {ex.Message}.");
//            //}
//        }

//        public void buildRenderPassToolStripMenuItem_Click(object sender, EventArgs e)
//        {
//            //try
//            //{
//            //    if (levelEditorTreeView1.RootObject is RenderPassLoaderModel ||
//            //        levelEditorTreeView1.RootObject is List<RenderPassLoaderModel>)
//            //    {
//            //        var renderPassPathList = new List<string>();
//            //        var renderPassLoaderModelList = levelEditorTreeView1.RootObject as List<RenderPassLoaderModel>;
//            //        foreach (var renderPassJsonModel in levelEditorTreeView1.RootObject as List<RenderPassLoaderModel>)
//            //        {
//            //            var renderPassJson = JsonConvert.SerializeObject(renderPassJsonModel);
//            //            File.WriteAllText($@"{ConstConfig.BaseDirectoryPath}RenderPass\{renderPassJsonModel.Name}.json", renderPassJson);
//            //            renderPassPathList.Add($@"{ConstConfig.BaseDirectoryPath}RenderPass\{renderPassJsonModel.Name}.json");
//            //            foreach (var pipelineModel in renderPassJsonModel.renderPipelineModelList)
//            //            {
//            //                var pipelineJson = JsonConvert.SerializeObject(pipelineModel);
//            //                // File.WriteAllText($@"{ConstConfig.BaseDirectoryPath}Pipelines\{pipelineModel.Name}.json", pipelineJson);
//            //            }
//            //        }
//            //    }
//            //}
//            //catch (Exception ex)
//            //{
//            //    MessageBox.Show(@$"Failed to build renderpass {ex.Message}.");
//            //}
//        }

//        private void RendererBox_MouseDown(object sender, MouseEventArgs e)
//        {
//            Point currentPos = e.Location;
//            if (e.Button == MouseButtons.Left)
//            {
//                uint pickedId = LevelEditorSystem.SampleRenderPassPixel(GameObjectIdTexture, new ivec2(currentPos.X, currentPos.Y));
//                if (pickedId != uint.MaxValue)
//                {
//                    SelectedSpriteIndex = pickedId;
//                    IsDragging = true;
//                    LastMousePosition = currentPos;
//                    RendererBox.Capture = true;
//                }
//            }
//            else if (e.Button == MouseButtons.Right)
//            {
//                IsDragging = true;
//                LastMousePosition = currentPos;
//                RendererBox.Capture = true;
//            }
//        }

//        private void RendererBox_MouseWheel(object sender, MouseEventArgs e)
//        {
//            Point mousePos = e.Location;
//            float scrollDelta = e.Delta / 1200.0f;
//            ref var cameraTransform = ref CameraSystem.UpdateActiveCamera();
//            cameraTransform.Zoom += scrollDelta;
//        }

//        private void RendererBox_MouseMove(object sender, MouseEventArgs e)
//        {
//            Point currentPos = e.Location;
//            int deltaX = currentPos.X - LastMousePosition.X;
//            int deltaY = currentPos.Y - LastMousePosition.Y;
//            if (e.Button == MouseButtons.Left)
//            {
//                ref var transform = ref GameObjectSystem.UpdateGameObjectComponent<Transform2DComponent>(SelectedSpriteIndex, ComponentTypeEnum.kTransform2DComponent);
//                transform.GameObjectPosition = new vec2(transform.GameObjectPosition.x + deltaX, transform.GameObjectPosition.y - deltaY);
//            }
//            else if (e.Button == MouseButtons.Right)
//            {
//                ref var cameraTransform = ref CameraSystem.UpdateActiveCamera();
//                cameraTransform.Position = new vec3(cameraTransform.Position.x + deltaX, cameraTransform.Position.y - deltaY, 0.0f);
//            }
//            LastMousePosition = currentPos;
//        }

//        private void RendererBox_MouseUp(object sender, MouseEventArgs e)
//        {
//            if (e.Button == MouseButtons.Left)
//            {
//                IsDragging = false;
//                SelectedSpriteIndex = 0;
//                RendererBox.Capture = false;
//            }
//            else if (e.Button == MouseButtons.Right)
//            {
//                IsDragging = false;
//                RendererBox.Capture = false;
//            }
//        }

//        private void levelEditorTreeView1_AfterSelect_2(object sender, TreeViewEventArgs e)
//        {

//        }

//        private void addGameObjectToolStripMenuItem_Click(object sender, EventArgs e)
//        {
//            uint gameObject = GameObjectSystem.CreateGameObject(new vec2(0.0f));
//            levelEditorTreeView1.Nodes.Add(gameObject.ToString());
//        }

//        private void RendererBox_Click(object sender, EventArgs e)
//        {

//        }

//        private void tabPage1_Click(object sender, EventArgs e)
//        {

//        }

//        private void tabPage2_Click(object sender, EventArgs e)
//        {

//        }

//        private void richTextBox1_TextChanged_1(object sender, EventArgs e)
//        {

//        }

//        private void tabPage1_Click_1(object sender, EventArgs e)
//        {

//        }

//        private void richTextBox2_TextChanged(object sender, EventArgs e)
//        {

//        }

//        private void bottomSplitter_SplitterMoved(object sender, SplitterEventArgs e)
//        {

//        }

//        private void tabPage1_Click_2(object sender, EventArgs e)
//        {

//        }
//    }
//}