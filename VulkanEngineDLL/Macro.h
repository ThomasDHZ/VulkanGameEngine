#pragma once
#include <stdio.h>
#include "Platform.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define IO_READ_CHUNK_SIZE 2097152
#define IO_READ_ERROR_GENERAL "Error reading file: %s. error: %d\n"
#define IO_READ_ERROR_MEMORY "Not enough free memory to read file: %s\n"

#define ERROR_EXIT(...) { \
    fprintf(stderr, __VA_ARGS__); \
    SLEEP(100); \
    exit(1); \
}

#define ERROR_RETURN(R, ...) { \
    fprintf(stderr, __VA_ARGS__); \
    SLEEP(100); \
    return (R); \
}

#define RENDERER_ERROR(errorMessage) { \
    fprintf(stderr, "Error in %s at line %d: %s: %s\n", \
            __FILE__, __LINE__, __func__, (errorMessage)); \
    SLEEP(100); \
}
