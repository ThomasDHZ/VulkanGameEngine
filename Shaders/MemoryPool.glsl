struct MemoryPoolHeader
{
    uint64_t FirstObjectOffset;
    uint     ObjectCount;
    uint     ObjectSize;
};

#define FLOAT_DATA(buffer, offset)   uintBitsToFloat((buffer).Data[(offset)++])
#define UINT_DATA(buffer, offset)    (buffer).Data[(offset)++]
#define INT_DATA(buffer, offset)     int((buffer).Data[(offset)++])

#define VEC2_DATA(buffer, offset)    vec2(FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset))
#define VEC3_DATA(buffer, offset)    vec3(FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset))
#define VEC4_DATA(buffer, offset)    vec4(FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset))

#define IVEC2_DATA(buffer, offset)   ivec2(UINT_DATA(buffer, offset), UINT_DATA(buffer, offset))
#define IVEC3_DATA(buffer, offset)   ivec3(UINT_DATA(buffer, offset), UINT_DATA(buffer, offset), UINT_DATA(buffer, offset))
#define IVEC4_DATA(buffer, offset)   ivec4(UINT_DATA(buffer, offset), UINT_DATA(buffer, offset), UINT_DATA(buffer, offset), UINT_DATA(buffer, offset))

#define MAT2_DATA(buffer, offset)    mat2(FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), \
                                       FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset))

#define MAT3_DATA(buffer, offset)    mat3(FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), \
                                       FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), \
                                       FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset))

#define MAT4_DATA(buffer, offset)    mat4(FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), \
                                       FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), \
                                       FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), \
                                       FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset), FLOAT_DATA(buffer, offset))
