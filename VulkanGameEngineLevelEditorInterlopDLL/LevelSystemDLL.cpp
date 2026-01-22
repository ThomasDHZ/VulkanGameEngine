//#include "LevelSystemDLL.h"
//
//VkCommandBuffer LevelSystem_RenderBloomPass(VkGuid& renderPassId)
//{
//    return levelSystem.RenderBloomPass(renderPassId);
//}
//
//VkCommandBuffer LevelSystem_RenderFrameBuffer(VkGuid& renderPassId)
//{
//    return levelSystem.RenderFrameBuffer(renderPassId);
//}
//
//VkCommandBuffer LevelSystem_RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
//{
//    return levelSystem.RenderLevel(renderPassId, levelId, deltaTime);
//}
//
//void LevelSystem_LoadLevel(const char* levelPath)
//{
//    levelSystem.LoadLevel(levelPath);
//}
//
//void LevelSystem_Update(float deltaTime)
//{
//    levelSystem.Update(deltaTime);
//}
//
//void LevelSystem_DestroyLevel()
//{
//    levelSystem.DestroyLevel();
//}
//
//LevelLayout LevelSystem_GetLevelLayout()
//{
//    return levelSystem.GetLevelLayout();
//}
//
//LevelLayer* LevelSystem_GetLevelLayerList(int& outCount)
//{
//    Vector<LevelLayer> levelLayerList = levelSystem.GetLevelLayerList();
//
//    outCount = static_cast<int>(levelLayerList.size());
//    return memorySystem.AddPtrBuffer<LevelLayer>(levelLayerList.data(), levelLayerList.size(), __FILE__, __LINE__, __func__);
//}
//
//uint** LevelSystem_GetLevelTileMapList(int& outCount)
//{
//    //Vector<Vector<uint>> levelTileMapList = GetLevelTileMapList();
//    //outCount = static_cast<int>(levelLayerList.size());
//    //return memorySystem.AddPtrBuffer<Sprite>(levelLayerList.data(), levelLayerList.size(), __FILE__, __LINE__, __func__);
//    return nullptr;
//}
//
//LevelTileSet* LevelSystem_GetLevelTileSetList(int& outCount)
//{
//    Vector<LevelTileSet> levelTileSetList = levelSystem.GetLevelTileSetList();
//
//    outCount = static_cast<int>(levelTileSetList.size());
//    return memorySystem.AddPtrBuffer<LevelTileSet>(levelTileSetList.data(), levelTileSetList.size(), __FILE__, __LINE__, __func__);
//}