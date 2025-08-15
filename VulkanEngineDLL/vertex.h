#pragma once
#include "Typedef.h"


struct Vertex2D
{
    vec2 Position = vec2(0.0f);
    vec2 UV = vec2(0.0f);

    Vertex2D()
    {
        Position = vec2(0.0f);
        UV = vec2(0.0f);
    }

    Vertex2D(vec2 position, vec2 uv)
    {
        Position = position;
        UV = uv;
    }
};

//struct VertexAttribute
//{
//    uint size;
//    uint offset;
//    VkFormat VertexType;
//};
//
//class VertexLayout
//{
//    private:
//        uint VertexSize;
//        Vector<VertexAttribute> AttributeList;
//
//        uint GetAttributeSize(VkFormat format)
//        {
//            switch (format)
//            {
//                case VK_FORMAT_R32_UINT: return 4;
//                case VK_FORMAT_R32G32_UINT: return 8;
//                case VK_FORMAT_R32G32B32_UINT: return 12;
//                case VK_FORMAT_R32G32B32A32_UINT: return 16;
//                case VK_FORMAT_R32_SINT: return 4;
//                case VK_FORMAT_R32G32_SINT: return 8;
//                case VK_FORMAT_R32G32B32_SINT: return 12;
//                case VK_FORMAT_R32G32B32A32_SINT: return 16;
//                case VK_FORMAT_R32_SFLOAT: return 4;
//                case VK_FORMAT_R32G32_SFLOAT: return 8;
//                case VK_FORMAT_R32G32B32_SFLOAT: return 12;
//                case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
//                default: throw std::runtime_error("Attribute type not covered yet.");
//            }
//            return 0;
//        }
//
//    public:
//
//        void AddAttribute(VkFormat format)
//        {
//            //spvFile.descriptor_sets.
//
//            //uint attributeSize = GetAttributeSize(format);
//            //uint offset = VertexSize * sizeof(uint);
//            //AttributeList.emplace_back(VertexAttribute
//            //    {
//            //        .size = attributeSize,
//            //        .offset = offset,
//            //        .VertexType = format
//            //    });
//            //VertexSize += attributeSize * sizeof(uint);
//        }
//
//
//        const Vector<VertexAttribute>& GetAttributes() const { return AttributeList; }
//        uint GetStride() const { return VertexSize; }
//};