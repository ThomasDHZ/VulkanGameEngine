﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    public struct GPUImport
    {
        public List<Mesh> MeshList { get; set; }
        public List<Texture> TextureList { get; set; }
        public List<Material> MaterialList { get; set; }

        public GPUImport(List<Mesh> meshList, List<Texture> textureList, List<Material> materialList)
        {
            MeshList = meshList;
            TextureList = textureList;
            MaterialList = materialList;
        }
    };
}
