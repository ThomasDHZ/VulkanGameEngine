using Newtonsoft.Json;
using Silk.NET.Vulkan;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using System.Xml.Linq;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
using VulkanGameEngineLevelEditor.Models;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace VulkanGameEngineLevelEditor
{
    public enum LevelEditorModeEnum
    {
        kLevelEditorMode,
        kRenderPassEditorMode
    }

    public unsafe partial class LevelEditorForm : Form
    {
        LevelEditorModeEnum LevelEditorMode = LevelEditorModeEnum.kLevelEditorMode;
        private volatile bool running;
        private volatile bool levelEditorRunning;
        private Stopwatch stopwatch = new Stopwatch();
        public RichTextBoxWriter textBoxWriter;
        private Thread renderThread { get; set; }
        private MessengerModel _messenger;
        private GCHandle _callbackHandle;

        private object lockObject = new object();
        private object sharedData;

        BlockingCollection<Dictionary<int, GameObject>> gameObjectData = new BlockingCollection<Dictionary<int, GameObject>>();
        [DllImport("kernel32.dll")] static extern bool AllocConsole();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void LogVulkanMessageDelegate(string message, int severity);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.Cdecl)] public static extern void SetRichTextBoxHandle(IntPtr hwnd);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.Cdecl)] public static extern void SetLogVulkanMessageCallback(LogVulkanMessageDelegate callback);

        public LevelEditorForm()
        {
            InitializeComponent();

            this.Load += Form1_Load;

            Thread.CurrentThread.Name = "LevelEditor";

            textBoxWriter = new RichTextBoxWriter(richTextBox2);


            _messenger = new MessengerModel
            {
                richTextBox = richTextBox2,
                TextBoxName = richTextBox2.Name,
                ThreadId = Thread.CurrentThread.ManagedThreadId,
                IsActive = true
            };

            GlobalMessenger.AddMessenger(_messenger);

            LogVulkanMessageDelegate callback = LogVulkanMessage;
            _callbackHandle = GCHandle.Alloc(callback);
            SetLogVulkanMessageCallback(callback);

            this.Text = "Vulkan Level Editor - RenderPassEditorView";
            var renderPass = new RenderPassLoaderModel(@"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\RenderPass\DefaultRenderPass.json");
            RenderSystem.RenderPassEditor_RenderPass[renderPass.RenderPassId] = renderPass;

            var renderPassLoader = new RenderPassLoaderModel
            {
                renderPipelineModelList = new System.Collections.Generic.List<RenderPipelineLoaderModel>
                {
                    new RenderPipelineLoaderModel(),
                    new RenderPipelineLoaderModel()
                }
            };
            dynamicControlPanelView1.SelectedObject = renderPassLoader;
            levelEditorTreeView1.DynamicControlPanel = dynamicControlPanelView1;
            levelEditorTreeView1.PopulateTreeView(renderPassLoader);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            StartRenderer();
        }
        public static void LogVulkanMessage(string message, int severity)
        {
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
                GameSystem.StartUp(this.pictureBox1.Handle.ToPointer(), this.richTextBox2.Handle.ToPointer());
            }));

            //lock (lockObject)
            //{
            //    sharedData = levelEditorTreeView1;
            //}

            //lock (lockObject)
            //{
            //    if (sharedData is LevelEditorTreeView levelEditorTree)
            //    {
            //        foreach (var gameObject in GameObjectSystem.GameObjectMap.Values)
            //        {
            //            levelEditorTree.AddGameObject(gameObject);
            //        }
            //        levelEditorTree.PopulateTree();
            //    }
            //}

            Stopwatch stopwatch = new Stopwatch();
            stopwatch.Start();
            double lastTime = 0.0;

            while (running)
            {
                double currentTime = stopwatch.Elapsed.TotalSeconds;
                double deltaTime = currentTime - lastTime;
                lastTime = currentTime;

                GameSystem.Update((float)deltaTime);
                GameSystem.Draw((float)deltaTime);
                lock (lockObject)
                {
                    // dynamicControlPanelView1.UpdateOriginalObject();
                }
            }

            GameSystem.Destroy();
        }

        private void toolStripButton1_Click(object sender, EventArgs e)
        {
            LevelEditorMode = LevelEditorModeEnum.kLevelEditorMode;

            this.Text = "Vulkan Level Editor - LevelEditorView";
            //levelEditorTreeView1.RootObject = new GameObject();
            //levelEditorTreeView1.dynamicControlPanelView = dynamicControlPanelView1;
        }
        private void toolStripButton2_Click(object sender, EventArgs e)
        {
            LevelEditorMode = LevelEditorModeEnum.kRenderPassEditorMode;

            this.Text = "Vulkan Level Editor - RenderPassEditorView";
            //levelEditorTreeView1.RootObject = new RenderPassLoaderModel(@"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\RenderPass\DefaultRenderPass.json");
            //levelEditorTreeView1.dynamicControlPanelView = dynamicControlPanelView1;
        }

        private void SaveRenderPass_Click(object sender, EventArgs e)
        {
            //var a = JsonConvert.SerializeObject(renderPass, Formatting.Indented);
            //var ab = 32;
        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void SaveLevel_Click(object sender, EventArgs e)
        {

        }

        private void levelEditorTreeView1_AfterSelect(object sender, TreeViewEventArgs e)
        {

        }

        private void dynamicControlPanelView1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void dynamicControlPanelView1_Paint_1(object sender, PaintEventArgs e)
        {

        }

        private void levelEditorTreeView1_AfterSelect_1(object sender, TreeViewEventArgs e)
        {

        }

        private void LevelEditorForm_Load(object sender, EventArgs e)
        {

        }
    }
}