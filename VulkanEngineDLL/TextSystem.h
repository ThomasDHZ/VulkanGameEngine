#pragma once

#include <Platform.h>
#include <ft2build.h>
#include FT_FREETYPE_H  

struct TextCharacter 
{
    uint32 TextureID;
    ivec2  Size;  
    ivec2  Bearing;   
    uint32 Advance; 
    vec2   UVMin;
    vec2   UVMax;
};

class TextSystem
{
public:
    static TextSystem& Get();

private:
    TextSystem() = default;
    ~TextSystem() = default;
    TextSystem(const TextSystem&) = delete;
    TextSystem& operator=(const TextSystem&) = delete;
    TextSystem(TextSystem&&) = delete;
    TextSystem& operator=(TextSystem&&) = delete;

    FT_Library m_fontLibrary;
    std::map<char, TextCharacter> CharacterMap;

public:

     void StartUp();
     void SetFont(String font);
     void RenderText(String& text, vec2 textPosition, float scale, vec3 color);
};
extern  TextSystem& textSystem;
inline TextSystem& TextSystem::Get()
{
    static TextSystem instance;
    return instance;
}


