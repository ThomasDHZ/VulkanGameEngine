#include "LevelEditorSystem.h"
#include <LevelSystem.h>

void LevelEditorSystem_SetSelectedGameObject(uint gameObjectId)
{
    levelSystem.SelectedGameObject = gameObjectId;
}

const uint LevelEditorSystem_SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition)
{
	return renderSystem.SampleRenderPassPixel(textureGuid, mousePosition);
}