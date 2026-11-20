struct MemoryPoolHeader
{
    uint64_t FirstObjectOffset;
    uint     ObjectCount;
    uint     ObjectSize;
};

float FloatData(inout uint offset, uint data[])
{
    return uintBitsToFloat(data[offset++]);
}

uint UIntData(inout uint offset, uint data[])
{
    return data[offset++];
}

int IntData(inout uint offset, uint data[])
{
    return int(data[offset++]);
}

vec2 Vec2Data(inout uint offset, uint data[])
{
    return vec2(uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]));
}

vec3 Vec3Data(inout uint offset, uint data[])
{
    return vec3(uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]));
}

vec4 Vec4Data(inout uint offset, uint data[])
{
    return vec4(uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]));
}

ivec2 IVec2Data(inout uint offset, uint data[])
{
    return ivec2(data[offset++],
                 data[offset++]);
}

ivec3 IVec3Data(inout uint offset, uint data[])
{
    return ivec3(data[offset++],
                 data[offset++],
                 data[offset++]);
}

ivec4 IVec4Data(inout uint offset, uint data[])
{
    return ivec4(data[offset++],
                data[offset++],
                data[offset++],
                data[offset++]);
}

mat2 Mat2Data(inout uint offset, uint data[])
{
    return mat2(uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]));
}

mat3 Mat3Data(inout uint offset, uint data[])
{
    return mat3(uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]));
}

mat4 Mat4Data(inout uint offset, uint data[])
{
    return mat4(uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]),
                uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]), uintBitsToFloat(data[offset++]));
}