#include "FileSystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "MemorySystem.h"
#include "from_json.h"

FileSystem fileSystem = FileSystem();

bool File_Exists(const char* fileName)
{
    struct stat buffer;
    return (stat(fileName, &buffer));
}

time_t File_LastModifiedTime(const char* fileName)
{
    struct stat buffer;
    if (stat(fileName, &buffer) == 0)
    {
        return buffer.st_size;
    }

    return -1;
}

char* File_RemoveFileExtention(const char* fileName)
{
    const char* lastDot = strrchr(fileName, '.');
    size_t length = lastDot ? (lastDot - fileName) : strlen(fileName);

    char* baseFileName = (char*)malloc(length + 1);
    if (baseFileName)
    {
        strncpy(baseFileName, fileName, length);
        baseFileName[length];
    }

    return baseFileName;
}

char* File_GetFileExtention(const char* fileName)
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

char* File_GetFileNameFromPath(const char* filePath)
{
    static char baseFileName[256];
    const char* lastDot = strrchr(filePath, '.');
    const char* lastSlashBack = strrchr(filePath, '\\');
    const char* lastSlashSlash = strrchr(filePath, '/');

    const char* lastSlash = lastSlashBack > lastSlashSlash ? lastSlashBack : lastSlashSlash;
    const char* ch = "\\";
    const char* lastBackslash = strrchr(filePath, '\\');

    size_t startPos = 0;
    if (lastSlash != NULL) {
        startPos = lastSlash - filePath + 1;
    }

    size_t endPos;
    if (lastDot != NULL && lastDot > (filePath + startPos)) {
        endPos = lastDot - filePath;
    }
    else {
        endPos = strlen(filePath);
    }

    size_t filenameLength = endPos - startPos;
    if (filenameLength >= sizeof(baseFileName))
        filenameLength = sizeof(baseFileName) - 1;

    strncpy(baseFileName, filePath + startPos, filenameLength);
    baseFileName[filenameLength] = '\0';

    return baseFileName;
}

FileState File_Read(const char* path)
{
    FileState fileState = { .Valid = 0 };

    FILE* fp = fopen(path, "rb");
    if (!fp || ferror(fp))
    {
        ERROR_RETURN(fileState, IO_READ_ERROR_GENERAL, path, errno);
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
                ERROR_RETURN(fileState, "Input file too large: %s\n", path);
            }

            tmp = (char*)realloc(data, size);
            if (!tmp)
            {
                free(data);
                ERROR_RETURN(fileState, IO_READ_ERROR_MEMORY, path);
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
        ERROR_RETURN(fileState, IO_READ_ERROR_GENERAL, path, errno);
    }

    tmp = (char*)realloc(data, used + 1);
    if (!tmp) {
        free(data);
        ERROR_RETURN(fileState, IO_READ_ERROR_MEMORY, path);
    }
    data = tmp;
    data[used] = 0;

    fileState.Data = data;
    fileState.Size = used;
    fileState.Valid = true;

    return fileState;
}

int File_Write(void* buffer, size_t size, const char* path)
{
    FILE* filePath = fopen(path, "wb");
    if (!filePath || ferror(filePath))
    {
        ERROR_RETURN(1, "Cannot write files: %s.\n", path);
    }

    size_t chunks_written = fwrite(buffer, size, 1, filePath);

    fclose(filePath);
    if (chunks_written != 1)
    {
        ERROR_RETURN(1, "Write error expected 1 chunk, got %zu.\n", chunks_written);
    }

    return 0;
}


nlohmann::json File_LoadJsonFile(const char* filePath)
{
    String rawJson = File_Read(filePath).Data;
    return nlohmann::json::parse(rawJson);
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

nlohmann::json LoadJsonFile(const String& filePath)
{
    String rawJson = File_Read(filePath.c_str()).Data;
    return nlohmann::json::parse(rawJson);
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