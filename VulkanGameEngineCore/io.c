#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "Macro.h"

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

char* File_GetFileNameFromPath(const char* fileName)
{
    char baseFileName[256];
    const char* lastDot = strrchr(fileName, '.');
    const char* firstSlash = strchr(fileName, '/');
    if (lastDot != NULL)
    {
        size_t length = lastDot - fileName;
        if (firstSlash != NULL &&
            firstSlash < lastDot)
        {
            length -= (firstSlash - fileName + 1);
            strncpy(baseFileName, firstSlash + 1, length);
        }
        else
        {
            strncpy(baseFileName, fileName, length);
        }
        baseFileName[length] = '\0';
    }
    else
    {
        if (firstSlash != NULL)
        {
            strncpy(baseFileName, firstSlash + 1, strlen(firstSlash + 1));
        }
        else
        {
            strncpy(baseFileName, fileName, strlen(fileName));
        }
        baseFileName[strlen(baseFileName)] = '\0';
    }
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
