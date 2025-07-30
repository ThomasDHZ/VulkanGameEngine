using Microsoft.CodeAnalysis.CSharp.Syntax;
using Newtonsoft.Json;
using Silk.NET.Vulkan;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net.Http.Json;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using System.Xml.Linq;
using Vulkan;
using VulkanGameEngineLevelEditor.Compilers;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
using VulkanGameEngineLevelEditor.Models;
using static System.Runtime.InteropServices.JavaScript.JSType;
using static System.Windows.Forms.LinkLabel;
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
        private volatile bool isResizing;
        private Stopwatch stopwatch = new Stopwatch();
        public SystemMessenger textBoxWriter;
        private Thread renderThread { get; set; }
        private MessengerModel _messenger;
        private GCHandle _callbackHandle;

        private object lockObject = new object();
        private object sharedData;
        public List<System.String> ShaderList = new List<string>();


        BlockingCollection<Dictionary<int, GameObject>> gameObjectData = new BlockingCollection<Dictionary<int, GameObject>>();
        [DllImport("kernel32.dll")] static extern bool AllocConsole();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void LogVulkanMessageDelegate(string message, int severity);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.Cdecl)] public static extern void SetRichTextBoxHandle(IntPtr hwnd);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.Cdecl)] public static extern void SetLogVulkanMessageCallback(LogVulkanMessageDelegate callback);

        public LevelEditorForm()
        {
            InitializeComponent();
            AllocConsole();

            this.Load += Form1_Load;

            Thread.CurrentThread.Name = "LevelEditor";

            textBoxWriter = new SystemMessenger(richTextBox2);
            ShaderCompiler.systemMessenger = textBoxWriter;

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
                void* afds = this.RendererBox.Handle.ToPointer();
                GameSystem.StartUp(this.RendererBox.Handle.ToPointer(), this.richTextBox2.Handle.ToPointer());

                List<RenderPassLoaderModel> renderPassLoaderList = new List<RenderPassLoaderModel>();
                foreach (var renderPassPair in RenderSystem.RenderPassEditor_RenderPass)
                {
                    renderPassLoaderList.Add(renderPassPair.Value);
                }
                renderPassLoaderList[0].renderPipelineModelList[0].VertexInputBindingDescriptionList = ShaderSystem.LoadVertexBindingLayout(renderPassLoaderList[0].renderPipelineModelList[0].VertexShaderSrc).ToList();
                renderPassLoaderList[0].renderPipelineModelList[1].VertexInputBindingDescriptionList = ShaderSystem.LoadVertexBindingLayout(renderPassLoaderList[0].renderPipelineModelList[1].VertexShaderSrc).ToList();
                renderPassLoaderList[1].renderPipelineModelList[0].VertexInputBindingDescriptionList = ShaderSystem.LoadVertexBindingLayout(renderPassLoaderList[1].renderPipelineModelList[0].VertexShaderSrc).ToList();
                renderPassLoaderList[0].renderPipelineModelList[0].VertexInputAttributeDescriptionList = ShaderSystem.LoadVertexAttributesLayout(renderPassLoaderList[0].renderPipelineModelList[0].VertexShaderSrc);
                renderPassLoaderList[0].renderPipelineModelList[1].VertexInputAttributeDescriptionList = ShaderSystem.LoadVertexAttributesLayout(renderPassLoaderList[0].renderPipelineModelList[1].VertexShaderSrc);
                renderPassLoaderList[1].renderPipelineModelList[0].VertexInputAttributeDescriptionList = ShaderSystem.LoadVertexAttributesLayout(renderPassLoaderList[1].renderPipelineModelList[0].VertexShaderSrc);

                renderPassLoaderList[0].renderPipelineModelList[0].PipelineDescriptorModelsList = ShaderSystem.LoadDescriptorSetBindings(renderPassLoaderList[0].renderPipelineModelList[0].VertexShaderSrc);
                renderPassLoaderList[0].renderPipelineModelList[1].PipelineDescriptorModelsList = ShaderSystem.LoadDescriptorSetBindings(renderPassLoaderList[0].renderPipelineModelList[1].VertexShaderSrc);
                renderPassLoaderList[1].renderPipelineModelList[0].PipelineDescriptorModelsList = ShaderSystem.LoadDescriptorSetBindings(renderPassLoaderList[1].renderPipelineModelList[0].VertexShaderSrc);
                renderPassLoaderList[0].renderPipelineModelList[0].LayoutBindingList = ShaderSystem.LoadDescriptorSetLayoutBindings(renderPassLoaderList[0].renderPipelineModelList[0].VertexShaderSrc);
                renderPassLoaderList[0].renderPipelineModelList[1].LayoutBindingList = ShaderSystem.LoadDescriptorSetLayoutBindings(renderPassLoaderList[0].renderPipelineModelList[1].VertexShaderSrc);
                renderPassLoaderList[1].renderPipelineModelList[0].LayoutBindingList = ShaderSystem.LoadDescriptorSetLayoutBindings(renderPassLoaderList[1].renderPipelineModelList[0].VertexShaderSrc);

                DynamicControlPanelView.toolTip = toolTip1;
                dynamicControlPanelView1.SelectedObject = renderPassLoaderList;
                levelEditorTreeView1.DynamicControlPanel = dynamicControlPanelView1;
                levelEditorTreeView1.PopulateTreeView(renderPassLoaderList);
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

        private void LevelEditorForm_Resize(object sender, EventArgs e)
        {
            ResizeRenderer();
        }

        private void ResizeRenderer()
        {
            if (running && !this.WindowState.HasFlag(FormWindowState.Minimized))
            {
                lock (lockObject)
                {
                    isResizing = true;
                    RenderSystem.RebuildRendererFlag = true;
                    RenderSystem.RecreateSwapchain(LevelSystem.spriteRenderPass2DId, LevelSystem.levelLayout.LevelLayoutId, 0.0f, new GlmSharp.ivec2(RendererBox.Width, RendererBox.Height));
                    isResizing = false;
                }
            }
        }

        private void RendererBox_Resize(object sender, EventArgs e)
        {
            ResizeRenderer();
        }

        private void buildShadersToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShaderCompiler.CompileAllShaders($@"{ConstConfig.BaseDirectoryPath}Shaders");
        }

        private void buildRenderPassToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var a = levelEditorTreeView1._rootObject as List<RenderPassLoaderModel>;
            var ds = RenderSystem.RenderPassLoaderJsonMap[a[0].RenderPassId];
            string jsonContent = File.ReadAllText(ds);
            var renderPass = JsonConvert.DeserializeObject<RenderPassLoaderModel>(jsonContent);

            renderPass.ClearValueList[0] = new VkClearValue
            {
                Color = new VkClearColorValue
                {
                    Float32_0 = 1.0f,
                    Float32_1 = 0.0f,
                    Float32_2 = 1.0f,
                    Float32_3 = 1.0f,
                },
                DepthStencil = new VkClearDepthStencilValue()
            };
       
            var renderPassJson = JsonConvert.SerializeObject(renderPass);
            File.WriteAllText($@"{ConstConfig.BaseDirectoryPath}RenderPass\testJson.json", renderPassJson);

            if (running && !this.WindowState.HasFlag(FormWindowState.Minimized))
            {
                lock (lockObject)
                {
                    isResizing = true;
                    RenderSystem.RebuildRendererFlag = true;
                    RenderSystem.UpdateRenderPasses(new List<RenderPassLoaderModel> { renderPass }, new List<RenderPipelineLoaderModel>());
                    isResizing = false;
                }
            }
        }
    }
}