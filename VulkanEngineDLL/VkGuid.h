#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#ifdef _WIN32
#include <objbase.h>  // GUID, CLSIDFromString, CoCreateGuid
#include <combaseapi.h> // StringFromGUID2
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <uuid/uuid.h>
#include <Endian.h>  // OSSwap* functions
#elif defined(__ANDROID__) || defined(__linux__)
#include <cctype>
#include <cstdio>
#endif

class VkGuid
{
public:
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    std::array<uint8_t, 8> Data4;

    // Default constructor - zero GUID
    VkGuid() : Data1(0), Data2(0), Data3(0), Data4{ 0 } {}

    VkGuid(const GUID& guid) {
        Data1 = guid.Data1;
        Data2 = guid.Data2;
        Data3 = guid.Data3;
        std::copy(std::begin(guid.Data4), std::end(guid.Data4), Data4.begin());
    }

    // From string (with or without braces)
    explicit VkGuid(const std::string& guidString) {
        std::string s = guidString;
        if (!s.empty() && s.front() != '{') {
            s = "{" + s + "}";
        }

#ifdef _WIN32
        std::wstring ws(s.begin(), s.end());
        GUID guid{};
        HRESULT hr = CLSIDFromString(ws.c_str(), &guid);
        if (FAILED(hr)) {
            throw std::runtime_error("Invalid GUID format");
        }
        Data1 = guid.Data1;
        Data2 = guid.Data2;
        Data3 = guid.Data3;
        std::copy(std::begin(guid.Data4), std::end(guid.Data4), Data4.begin());

#elif defined(__APPLE__)
        CFStringRef cfStr = CFStringCreateWithCString(nullptr, s.c_str(), kCFStringEncodingUTF8);
        if (!cfStr) throw std::runtime_error("Failed to create CFString");
        CFUUIDRef uuidRef = CFUUIDCreateFromString(nullptr, cfStr);
        CFRelease(cfStr);
        if (!uuidRef) throw std::runtime_error("Invalid GUID string");

        CFUUIDBytes bytes = CFUUIDGetUUIDBytes(uuidRef);
        CFRelease(uuidRef);

        // Convert big-endian UUID to Windows mixed-endian layout
        Data1 = OSSwapBigToHostInt32(*(uint32_t*)&bytes.byte0);
        Data2 = OSSwapBigToHostInt16(*(uint16_t*)&bytes.byte4);
        Data3 = OSSwapBigToHostInt16(*(uint16_t*)&bytes.byte6);
        Data4[0] = bytes.byte8;
        Data4[1] = bytes.byte9;
        Data4[2] = bytes.byte10;
        Data4[3] = bytes.byte11;
        Data4[4] = bytes.byte12;
        Data4[5] = bytes.byte13;
        Data4[6] = bytes.byte14;
        Data4[7] = bytes.byte15;

#else // Android / Linux
        std::string clean;
        for (char c : s) {
            if (c == '{' || c == '}' || c == '-') continue;
            clean += std::toupper(c);
        }
        if (clean.size() != 32) throw std::runtime_error("GUID must be 32 hex digits");

        auto parse = [&](size_t pos) -> uint8_t {
            return static_cast<uint8_t>(strtol(clean.substr(pos, 2).c_str(), nullptr, 16));
            };

        Data1 = (parse(0) << 24) | (parse(2) << 16) | (parse(4) << 8) | parse(6);
        Data2 = (parse(8) << 8) | parse(10);
        Data3 = (parse(12) << 8) | parse(14);
        for (int i = 0; i < 8; ++i)
            Data4[i] = parse(16 + i * 2);
#endif
    }

    // Generate new random GUID
    static VkGuid Generate() {
#ifdef _WIN32
        GUID guid;
        if (FAILED(CoCreateGuid(&guid))) {
            throw std::runtime_error("CoCreateGuid failed");
        }
        return VkGuid(guid);
#elif defined(__APPLE__)
        CFUUIDRef uuid = CFUUIDCreate(nullptr);
        CFUUIDBytes bytes = CFUUIDGetUUIDBytes(uuid);
        CFRelease(uuid);
        VkGuid g;
        g.Data1 = OSSwapBigToHostInt32(*(uint32_t*)&bytes.byte0);
        g.Data2 = OSSwapBigToHostInt16(*(uint16_t*)&bytes.byte4);
        g.Data3 = OSSwapBigToHostInt16(*(uint16_t*)&bytes.byte6);
        g.Data4[0] = bytes.byte8;  g.Data4[1] = bytes.byte9;
        g.Data4[2] = bytes.byte10; g.Data4[3] = bytes.byte11;
        g.Data4[4] = bytes.byte12; g.Data4[5] = bytes.byte13;
        g.Data4[6] = bytes.byte14; g.Data4[7] = bytes.byte15;
        return g;
#else
        // Fallback: use time + rand (not cryptographically secure)
        VkGuid g;
        uint64_t rand1 = static_cast<uint64_t>(std::rand()) << 32 | std::rand();
        uint64_t rand2 = static_cast<uint64_t>(std::rand()) << 32 | std::rand();
        std::memcpy(&g.Data1, &rand1, 4);
        std::memcpy(&g.Data2, &rand2, 2);
        std::memcpy(&g.Data3, ((uint8_t*)&rand2) + 2, 2);
        std::memcpy(g.Data4.data(), ((uint8_t*)&rand1) + 4, 8);
        g.Data4[0] &= 0x0F; g.Data4[0] |= 0x40; // Version 4
        g.Data4[2] &= 0x3F; g.Data4[2] |= 0x80; // Variant
        return g;
#endif
    }

    // Empty GUID
    static VkGuid Empty() { return VkGuid(); }

    // To string: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    std::string ToString() const {
        char buf[39] = {};
#ifdef _WIN32
        GUID guid{ Data1, Data2, Data3 };
        std::copy(Data4.begin(), Data4.end(), guid.Data4);
        WCHAR wbuf[39];
        if (StringFromGUID2(guid, wbuf, 39) > 0) {
            // Convert WCHAR to char
            for (int i = 0; i < 38; ++i) buf[i] = static_cast<char>(wbuf[i]);
            buf[38] = '\0';
            return std::string(buf);
        }
        // Fallback
#endif
        snprintf(buf, sizeof(buf),
            "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            Data1, Data2, Data3,
            Data4[0], Data4[1], Data4[2], Data4[3],
            Data4[4], Data4[5], Data4[6], Data4[7]);
        return std::string(buf);
    }

    // Equality
    bool operator==(const VkGuid& other) const {
        return Data1 == other.Data1 &&
            Data2 == other.Data2 &&
            Data3 == other.Data3 &&
            Data4 == other.Data4;
    }

    bool operator!=(const VkGuid& other) const {
        return !(*this == other);
    }

    // For unordered_map / unordered_set
    friend struct std::hash<VkGuid>;
};

// Hash specialization
namespace std {
    template <>
    struct hash<VkGuid> {
        size_t operator()(const VkGuid& g) const noexcept {
            size_t h = std::hash<uint32_t>{}(g.Data1);
            h ^= std::hash<uint16_t>{}(g.Data2) << 1;
            h ^= std::hash<uint16_t>{}(g.Data3) << 2;
            for (auto b : g.Data4) {
                h = (h << 8) ^ std::hash<uint8_t>{}(b);
            }
            return h;
        }
    };
}