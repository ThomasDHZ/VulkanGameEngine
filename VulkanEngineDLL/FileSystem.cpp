#include "FileSystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "MemorySystem.h"
#include "from_json.h"
#include <sys/stat.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#ifdef __ANDROID__
#define STBI_NO_STDIO
#endif
#include "stb_image.h"

FileSystem fileSystem = FileSystem();


Vector<String> FileSystem::GetFilesFromDirectory(const String& fileDirectory)
{
    Vector<String> fileList;
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(fileDirectory))
        {
            if (std::filesystem::exists(fileDirectory) && std::filesystem::is_directory(fileDirectory))
            {
                fileList.push_back(entry.path().string());
            }
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        std::cerr << "Filesystem error: " << ex.what() << std::endl;
    }

    return fileList;
}

Vector<String> FileSystem::GetFilesFromDirectory(const String& fileDirectory, const Vector<String>& fileExtensionList)
{
    Vector<String> fileList;
    try
    {
        if (std::filesystem::exists(fileDirectory) && std::filesystem::is_directory(fileDirectory))
        {
            for (const auto& entry : std::filesystem::directory_iterator(fileDirectory))
            {
                if (entry.is_regular_file())
                {
                    auto ext = entry.path().extension().string();
                    if (!ext.empty() && ext.front() == '.')
                    {
                        ext.erase(0, 1);
                    }

                    for (const auto& allowedExt : fileExtensionList)
                    {
                        if (ext == allowedExt)
                        {
                            fileList.push_back(entry.path().string());
                            break;
                        }
                    }
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        std::cerr << "Filesystem error: " << ex.what() << std::endl;
    }

    return fileList;
}

nlohmann::json FileSystem::LoadJsonFile(const String& filePath)
{
#if defined(__ANDROID__)
    if (!g_AssetManager)
    {
        throw std::runtime_error("Asset Manager is not initialized!");
    }

    AAsset* asset = AAssetManager_open(g_AssetManager, filePath.c_str(), AASSET_MODE_BUFFER);
    if (!asset)
    {
        throw std::runtime_error("Failed to open Android asset: " + filePath);
    }

    size_t size = AAsset_getLength(asset);
    Vector<byte> buffer(size);
    AAsset_read(asset, buffer.data(), size);
    AAsset_close(asset);
#else
    std::ifstream file(std::filesystem::current_path() / filePath, std::ios::binary | std::ios::ate);
    size_t size = file.tellg();
    file.seekg(0);
    Vector<byte> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    if (!file)
    {
        std::cerr << "ERROR: Failed to read full shader file: " << filePath << std::endl;
    }
#endif
	String rawJson = String(buffer.begin(), buffer.end());
    return nlohmann::json::parse(rawJson);
}

Vector<byte> FileSystem::LoadImageFile(const String& filePath, int& width, int& height, int& channelCount)
{
    byte* data = nullptr;
    int w = 0, h = 0, comp = 0;

#ifdef PLATFORM_ANDROID
    AAsset* asset = AAssetManager_open(g_AssetManager, filePath.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        return {};
    }

    size_t size = AAsset_getLength(asset);
    const void* buffer = AAsset_getBuffer(asset);
    data = stbi_load_from_memory((const stbi_uc*)buffer, (int)size, &w, &h, &comp, 4);
    AAsset_close(asset);

   /* if (!data) {
        __android_log_print(ANDROID_LOG_ERROR, "FileSystem", "STB failed: %s", stbi_failure_reason());
        return {};
    }*/
#else
    data = stbi_load(filePath.c_str(), &w, &h, &comp, 4);
#endif

    width = w;
    height = h;
    channelCount = ColorChannelUsed::ChannelRGBA;

    Vector<byte> result(data, data + (w * h * 4));
    stbi_image_free(data);
    return result;
}

String FileSystem::File_GetFileExtention(const char* fileName)
{
    const char* dot = strrchr(fileName, '.');
    if (!dot || dot == fileName)
    {
        return NULL;
    }

    char* extension = (char*)malloc(strlen(dot));
    if (extension == NULL)
    {
        return NULL;
    }

    strcpy(extension, dot + 1);
    return extension;
}

const char* FileSystem::ReadFile(const String& filePath)
{
    FileState fileState = { .Valid = 0 };

    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp || ferror(fp))
    {
        ERROR_RETURN(fileState.Data, IO_READ_ERROR_GENERAL, filePath.c_str(), errno);
    }


    char* data = NULL;
    char* tmp;
    size_t used = 0;
    size_t size = 0;
    size_t n;

    while (true) {
        if (used + IO_READ_CHUNK_SIZE + 1 > size)
        {
            size = used + IO_READ_CHUNK_SIZE + 1;

            if (size <= used)
            {
                free(data);
                ERROR_RETURN(fileState.Data, "Input file too large: %s\n", filePath.c_str());
            }

            tmp = (char*)realloc(data, size);
            if (!tmp)
            {
                free(data);
                ERROR_RETURN(fileState.Data, IO_READ_ERROR_MEMORY, filePath.c_str());
            }
            data = tmp;
        }

        n = fread(data + used, 1, IO_READ_CHUNK_SIZE, fp);
        if (n == 0)
            break;

        used += n;
    }

    if (ferror(fp)) {
        free(data);
        ERROR_RETURN(fileState.Data, IO_READ_ERROR_GENERAL, filePath.c_str(), errno);
    }

    tmp = (char*)realloc(data, used + 1);
    if (!tmp) {
        free(data);
        ERROR_RETURN(fileState.Data, IO_READ_ERROR_MEMORY, filePath.c_str());
    }
    data = tmp;
    data[used] = 0;

    fileState.Data = data;
    fileState.Size = used;
    fileState.Valid = true;

    return fileState.Data;
}

nlohmann::json FileSystem::LoadConfig(const String& configPath)
{
    #ifdef PLATFORM_ANDROID
    if (!g_AssetManager) {
        throw std::runtime_error("AssetManager not initialized");
    }

    AAsset* asset = AAssetManager_open(g_AssetManager, configPath.c_str(), AASSET_MODE_BUFFER);
    if (!asset)
    {
        throw std::runtime_error("Failed to open asset: " + configPath);
    }

    size_t size = AAsset_getLength(asset);
    if (size == 0)
    {
        AAsset_close(asset);
        return nlohmann::json{};
    }

    String buffer;
    buffer.resize(size);

    ssize_t read_bytes = AAsset_read(asset, buffer.data(), size);
    AAsset_close(asset);

    if (read_bytes <= 0)
    {
        throw std::runtime_error("Failed to read asset: " + configPath);
    }

    if (static_cast<size_t>(read_bytes) < size) {
        buffer.resize(read_bytes);
    }

    return nlohmann::json::parse(buffer);
    #else
    std::ifstream file(configPath);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open " + configPath);
            }
            nlohmann::json j;
            file >> j;
            return j;
    #endif
}

Vector<byte> FileSystem::LoadAssetFile(const String& filePath)
{
#if defined(__ANDROID__)
    if (!g_AssetManager)
    {
        throw std::runtime_error("Asset Manager is not initialized!");
    }

    AAsset* asset = AAssetManager_open(g_AssetManager, filePath.c_str(), AASSET_MODE_BUFFER);
    if (!asset)
    {
        throw std::runtime_error("Failed to open Android asset: " + filePath);
    }

    size_t size = AAsset_getLength(asset);
    Vector<byte> buffer(size);
    AAsset_read(asset, buffer.data(), size);
    AAsset_close(asset);
#else
    std::ifstream file(std::filesystem::current_path() / filePath, std::ios::binary | std::ios::ate);
    size_t size = file.tellg();
    file.seekg(0);
    Vector<byte> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    if (!file)
    {
        std::cerr << "ERROR: Failed to read full shader file: " << filePath << std::endl;
    }
#endif
    return buffer;
}

bool FileSystem::WriteFile(void* fileInfo, size_t size, const String& path)
{
    FILE* filePath = fopen(path.c_str(), "wb");
    if (!filePath || ferror(filePath))
    {
        ERROR_RETURN(1, "Cannot write files: %s.\n", path.c_str());
    }

    size_t chunks_written = fwrite(fileInfo, size, 1, filePath);

    fclose(filePath);
    if (chunks_written != 1)
    {
        ERROR_RETURN(1, "Write error expected 1 chunk, got %zu.\n", chunks_written);
    }

    return 0;
}

String FileSystem::GetFileExtention(const char* fileName)
{
    const char* dot = strrchr(fileName, '.');
    if (!dot || dot == fileName)
    {
        return NULL;
    }

    char* extension = (char*)malloc(strlen(dot));
    if (extension == NULL)
    {
        return NULL;
    }

    strcpy(extension, dot + 1);
    return extension;
}

String FileSystem::GetFileNameFromPath(const String& filePath)
{
    static char baseFileName[256];
    const char* lastDot = strrchr(filePath.c_str(), '.');
    const char* lastSlashBack = strrchr(filePath.c_str(), '\\');
    const char* lastSlashSlash = strrchr(filePath.c_str(), '/');

    const char* lastSlash = lastSlashBack > lastSlashSlash ? lastSlashBack : lastSlashSlash;
    const char* ch = "\\";
    const char* lastBackslash = strrchr(filePath.c_str(), '\\');

    size_t startPos = 0;
    if (lastSlash != NULL) {
        startPos = lastSlash - filePath.c_str() + 1;
    }

    size_t endPos;
    if (lastDot != NULL && lastDot > (filePath.c_str() + startPos)) {
        endPos = lastDot - filePath.c_str();
    }
    else {
        endPos = strlen(filePath.c_str());
    }

    size_t filenameLength = endPos - startPos;
    if (filenameLength >= sizeof(baseFileName))
        filenameLength = sizeof(baseFileName) - 1;

    strncpy(baseFileName, filePath.c_str() + startPos, filenameLength);
    baseFileName[filenameLength] = '\0';

    return baseFileName;
}

time_t FileSystem::LastModifiedTime(const String& filePath)
{
    struct stat buffer;
    if (stat(filePath.c_str(), &buffer) == 0)
    {
        return buffer.st_size;
    }

    return -1;
}

String FileSystem::RemoveFileExtention(const String& filePath)
{
    const char* lastDot = strrchr(filePath.c_str(), '.');
    size_t length = lastDot ? (lastDot - filePath.c_str()) : strlen(filePath.c_str());

    char* baseFileName = (char*)malloc(length + 1);
    if (baseFileName)
    {
        strncpy(baseFileName, filePath.c_str(), length);
        baseFileName[length];
    }

    return baseFileName;
}

bool FileSystem::FileExists(const String& filePath)
{
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}

#if defined(__ANDROID__)
void FileSystem::LoadAndroidAssetManager(AAssetManager* androidAssetManager)
{
    g_AssetManager = androidAssetManager;
}
#endif