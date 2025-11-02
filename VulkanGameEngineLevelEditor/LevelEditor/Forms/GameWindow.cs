using AutoMapper;
using Microsoft.Extensions.DependencyInjection;
using Silk.NET.Vulkan;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor
{
    public unsafe partial class GameWindow : Form
    {
        private Vk vk = Vk.GetApi();
        private volatile bool running;
        private volatile bool levelEditorRunning;
        private volatile bool isResizing;
        private Stopwatch stopwatch = new Stopwatch();
        private Extent2D VulkanSwapChainResolution { get; set; }
        private Thread renderThread { get; set; }
        public MessengerModel RenderPassMessager { get; set; }
        private object lockObject = new object();


        [DllImport("kernel32.dll")]
        static extern bool AllocConsole();

        public GameWindow()
        {
            InitializeComponent();
            AllocConsole();

            this.Load += Form1_Load;
            this.KeyDown += KeyPress_Down;
            this.KeyUp += KeyPress_Up;
            // this.KeyDown += Form1_KeyDown;

            VulkanSwapChainResolution = new Extent2D() { Width = 1280, Height = 720 };
            Thread.CurrentThread.Name = "LevelEditor";
        }

        private void KeyPress_Down(object sender, KeyEventArgs e)
        {
   
        }

        private void KeyPress_Up(object sender, KeyEventArgs e)
        {
        
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            StartRenderer();
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
                void* afds = this.pictureBox1.Handle.ToPointer();
                GameSystem.StartUp(this.pictureBox1.Handle.ToPointer(), null);
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

        private void LevelEditorForm_Resize(object sender, EventArgs e)
        {
            if (running && !this.WindowState.HasFlag(FormWindowState.Minimized))
            {
                
                    isResizing = true;
                    RenderSystem.RebuildRendererFlag = true;
                   // RenderSystem.RecreateSwapchain(LevelSystem.spriteRenderPass2DId, LevelSystem.levelLayout.LevelLayoutId, 0.0f, new GlmSharp.ivec2(pictureBox1.Width, pictureBox1.Height));
                    isResizing = false;
                
            }
        }
    }
}
