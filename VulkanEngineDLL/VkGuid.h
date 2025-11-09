#pragma once
#include <cstdint>
#include <cstring>
#include <functional>
#include <objbase.h>
#include <stdexcept>
#include <array>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

class VkGuid
{
private:
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    std::array<uint8_t, 8> Data4;

public:
    VkGuid() : Data1(0), Data2(0), Data3(0), Data4{ 0 } {}

    VkGuid(const char* guidchar)
    {
        GUID guid = {};
        std::string guidString = std::string(guidchar);
        if (guidString.front() != '{')
        {
            guidString = "{" + guidString + "}";
        }

        std::wstring wGuidStr(guidString.begin(), guidString.end());
        HRESULT result = CLSIDFromString(wGuidStr.c_str(), &guid);

        if (FAILED(result))
        {
            std::cerr << "Failed to convert string to GUID. HRESULT: " << std::hex << result << std::endl;
            throw std::runtime_error("Invalid GUID format.");
        }

        Data1 = guid.Data1;
        Data2 = guid.Data2;
        Data3 = guid.Data3;
        std::copy(std::begin(guid.Data4), std::end(guid.Data4), Data4.begin());
    }

    VkGuid(const GUID& guid) {
        Data1 = guid.Data1;
        Data2 = guid.Data2;
        Data3 = guid.Data3;
        std::copy(std::begin(guid.Data4), std::end(guid.Data4), Data4.begin());
    }

    static VkGuid GenerateGUID()
    {
        GUID guid;
        HRESULT result = CoCreateGuid(&guid);
        if (FAILED(result)) {
            throw std::runtime_error("Failed to create GUID");
        }
        return VkGuid(guid);
    }

    bool operator==(const VkGuid& rhs) const {
        return std::memcmp(this, &rhs, sizeof(VkGuid)) == 0;
    }

    bool operator==(const GUID& rhs) const {
        return std::memcmp(this, &rhs, sizeof(GUID)) == 0;
    }

    std::string ToString() const
    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        // Data1: 8 hex digits
        oss << '{'
            << std::setw(8) << Data1 << '-'
            // Data2: 4 hex digits
            << std::setw(4) << Data2 << '-'
            // Data3: 4 hex digits
            << std::setw(4) << Data3 << '-'
            // Data4[0..1]: 2 hex digits each
            << std::setw(2) << static_cast<int>(Data4[0])
            << std::setw(2) << static_cast<int>(Data4[1]) << '-'
            // Data4[2..7]: 2 hex digits each
            << std::setw(2) << static_cast<int>(Data4[2])
            << std::setw(2) << static_cast<int>(Data4[3])
            << std::setw(2) << static_cast<int>(Data4[4])
            << std::setw(2) << static_cast<int>(Data4[5])
            << std::setw(2) << static_cast<int>(Data4[6])
            << std::setw(2) << static_cast<int>(Data4[7])
            << '}';

        return oss.str();
    }

    friend struct std::hash<VkGuid>;
};

namespace std {
    template <>
    struct hash<VkGuid> {
        size_t operator()(const VkGuid& guid) const {
            size_t h1 = hash<uint32_t>{}(guid.Data1);
            size_t h2 = hash<uint16_t>{}(guid.Data2);
            size_t h3 = hash<uint16_t>{}(guid.Data3);
            size_t h4 = 0;
            for (auto byte : guid.Data4) {
                h4 = (h4 << 8) | byte;
            }
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ h4;
        }
    };
}