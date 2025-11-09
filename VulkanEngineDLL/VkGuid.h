#pragma once
#include <cstdint>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <array>
#include <sstream>
#include <iomanip>
#include <string>
#include <random>

#ifdef _WIN32
#include <objbase.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef __linux__
#include <uuid/uuid.h>
#endif

class VkGuid
{
private:
    // Helper to parse a two-char hex string to uint8_t
    static uint8_t parse_hex_pair(const std::string& hex_pair) {
        if (hex_pair.length() != 2) {
            throw std::runtime_error("Invalid hex pair length");
        }
        std::istringstream iss(hex_pair);
        uint8_t value;
        iss >> std::hex >> value;
        if (iss.fail()) {
            throw std::runtime_error("Invalid hex characters");
        }
        return value;
    }

    // Portable string parsing (big-endian byte order)
    void from_string(const std::string& guid_str) {
        std::string s = guid_str;
        // Strip braces if present
        if (!s.empty() && s.front() == '{') {
            s.erase(0, 1);
        }
        if (!s.empty() && s.back() == '}') {
            s.pop_back();
        }
        if (s.length() != 36) {
            throw std::runtime_error("Invalid GUID string length");
        }
        // Validate hyphen positions
        if (s[8] != '-' || s[13] != '-' || s[18] != '-' || s[23] != '-') {
            throw std::runtime_error("Invalid GUID format (hyphens missing)");
        }
        // Extract parts
        std::array<std::string, 5> parts = {
            s.substr(0, 8),
            s.substr(9, 4),
            s.substr(14, 4),
            s.substr(19, 4),
            s.substr(24, 12)
        };
        size_t byte_idx = 0;
        for (const auto& part : parts) {
            for (size_t j = 0; j < part.length(); j += 2) {
                Data[byte_idx++] = parse_hex_pair(part.substr(j, 2));
            }
        }
    }

public:
    std::array<uint8_t, 16> Data;

#ifdef _WIN32
    // Convert Windows GUID (little-endian fields) to big-endian byte array
    static void guid_to_bytes(const GUID& guid, std::array<uint8_t, 16>& bytes) {
        uint32_t d1 = guid.Data1;
        bytes[0] = static_cast<uint8_t>(d1 >> 24);
        bytes[1] = static_cast<uint8_t>(d1 >> 16);
        bytes[2] = static_cast<uint8_t>(d1 >> 8);
        bytes[3] = static_cast<uint8_t>(d1);

        uint16_t d2 = guid.Data2;
        bytes[4] = static_cast<uint8_t>(d2 >> 8);
        bytes[5] = static_cast<uint8_t>(d2);

        uint16_t d3 = guid.Data3;
        bytes[6] = static_cast<uint8_t>(d3 >> 8);
        bytes[7] = static_cast<uint8_t>(d3);

        for (int i = 0; i < 8; ++i) {
            bytes[8 + i] = guid.Data4[i];
        }
    }
#endif

    VkGuid() : Data{ 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48 } {}

    VkGuid(const char* guidchar) {
        from_string(guidchar);
    }

#ifdef _WIN32
    VkGuid(const GUID& guid) {
        guid_to_bytes(guid, Data);
    }
#endif

    static VkGuid GenerateGUID() {
        VkGuid guid;
#ifdef _WIN32
        GUID win_guid;
        HRESULT result = CoCreateGuid(&win_guid);
        if (FAILED(result)) {
            throw std::runtime_error("Failed to create GUID");
        }
        guid_to_bytes(win_guid, guid.Data);
#elif defined(__APPLE__)
        CFUUIDRef cfuuid = CFUUIDCreate(kCFAllocatorDefault);
        if (!cfuuid) {
            throw std::runtime_error("Failed to create UUID");
        }
        CFUUIDBytes bytes = CFUUIDGetUUIDBytes(cfuuid);
        for (int i = 0; i < 16; ++i) {
            guid.Data[i] = bytes.byte[i];
        }
        CFRelease(cfuuid);
#elif defined(__linux__) && !defined(__ANDROID__)
        uuid_t uu;
        uuid_generate(uu);
        std::copy(uu, uu + 16, guid.Data.begin());
#else
        // Fallback to random (UUID v4)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint8_t> dis(0, 255);
        for (auto& byte : guid.Data) {
            byte = dis(gen);
        }
        // Set version 4 and variant bits
        guid.Data[6] = (guid.Data[6] & 0x0F) | 0x40;
        guid.Data[8] = (guid.Data[8] & 0x3F) | 0x80;
#endif
        return guid;
    }

    bool operator==(const VkGuid& rhs) const {
        return std::memcmp(this, &rhs, sizeof(VkGuid)) == 0;
    }

#ifdef _WIN32
    bool operator==(const GUID& rhs) const {
        std::array<uint8_t, 16> rhs_bytes;
        guid_to_bytes(rhs, rhs_bytes);
        return Data == rhs_bytes;
    }
#endif

    std::string ToString() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << '{';
        for (size_t i = 0; i < 16; ++i) {
            if (i == 4 || i == 6 || i == 8 || i == 10) {
                oss << '-';
            }
            oss << std::setw(2) << static_cast<int>(Data[i]);
        }
        oss << '}';
        return oss.str();
    }
};

namespace std {
    template <>
    struct hash<VkGuid> {
        size_t operator()(const VkGuid& guid) const {
            // Reconstruct big-endian fields for compatibility with original hash logic
            uint32_t d1_be = (static_cast<uint32_t>(guid.Data[0]) << 24) |
                (static_cast<uint32_t>(guid.Data[1]) << 16) |
                (static_cast<uint32_t>(guid.Data[2]) << 8) |
                static_cast<uint32_t>(guid.Data[3]);
            uint16_t d2_be = (static_cast<uint16_t>(guid.Data[4]) << 8) | guid.Data[5];
            uint16_t d3_be = (static_cast<uint16_t>(guid.Data[6]) << 8) | guid.Data[7];
            size_t h4 = 0;
            for (int i = 0; i < 8; ++i) {
                h4 = (h4 << 8) | guid.Data[8 + i];
            }
            size_t h1 = hash<uint32_t>{}(d1_be);
            size_t h2 = hash<uint16_t>{}(d2_be);
            size_t h3 = hash<uint16_t>{}(d3_be);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ h4;
        }
    };
}