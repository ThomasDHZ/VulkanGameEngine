#include "TextSystem.h"
#include "MeshSystem.h"

TextSystem& textSystem = TextSystem::Get();

void TextSystem::StartUp()
{
    if (FT_Init_FreeType(&m_fontLibrary))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }
}

void TextSystem::SetFont(String font)
{
    FT_Face face;
    if (FT_New_Face(m_fontLibrary, font.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
    }
}

void TextSystem::RenderText(String& text, vec2 textPosition, float scale, vec3 color)
{
    Vector<TextVertex2DLayout> textVertexList;
    textVertexList.reserve(text.size() * 4);

    Vector<uint32> indexList;
    indexList.reserve(text.size() * 6);
    
    for (auto& character : text)
    {
        auto it = CharacterMap.find(character);
        if (it == CharacterMap.end()) continue;

        const TextCharacter& glyph = it->second;
        float xPos = textPosition.x + glyph.Bearing.x * scale;
        float yPos = textPosition.y - (glyph.Size.y - glyph.Bearing.y) * scale;
        float width = glyph.Size.x * scale;
        float height = glyph.Size.y * scale;

        textVertexList.emplace_back(TextVertex2DLayout{ vec2(xPos,         yPos + height), vec2(glyph.UVMin.x, glyph.UVMin.y), color });
        textVertexList.emplace_back(TextVertex2DLayout{ vec2(xPos,         yPos         ), vec2(glyph.UVMin.x, glyph.UVMax.x), color });
        textVertexList.emplace_back(TextVertex2DLayout{ vec2(xPos + width, yPos         ), vec2(glyph.UVMax.x, glyph.UVMax.x), color });
        textVertexList.emplace_back(TextVertex2DLayout{ vec2(xPos + width, yPos + height), vec2(glyph.UVMax.x, glyph.UVMin.y), color });

        indexList.emplace_back(textVertexList.size() + 0);
        indexList.emplace_back(textVertexList.size() + 1);
        indexList.emplace_back(textVertexList.size() + 2);
        indexList.emplace_back(textVertexList.size() + 2);
        indexList.emplace_back(textVertexList.size() + 3);
        indexList.emplace_back(textVertexList.size() + 0);

        textPosition.x += glyph.Advance * scale;
    }
    VertexLayout vertexData =
    {
        .VertexDataSize = textVertexList.size() * sizeof(TextVertex2DLayout),
        .VertexData = textVertexList.data()
    };

    meshSystem.CreateMesh("__Text__", MeshTypeEnum::kMesh_StaticMesh, vertexData);
}
