//#include "VkGuid.h"
//#include <sstream>
//#include <iomanip>
//#include <algorithm>
//#include <cctype>
//#include <stdexcept>
//#include <vector>
//#include <limits>
//#include <string_view>
//
//const std::string VkGuid::base64_chars =
//"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//"abcdefghijklmnopqrstuvwxyz"
//"0123456789-_";
//
///* ------------------------------------------------------------------ */
///*  parseHex                                                          */
///* ------------------------------------------------------------------ */
//template <typename T>
//T VkGuid::parseHex(const std::string& hex) {
//    std::stringstream ss;
//    ss << std::hex << hex;
//    T v;
//    ss >> v;
//    if (ss.fail()) throw std::runtime_error("Invalid hex");
//    return v;
//}
//template uint32_t VkGuid::parseHex(const std::string&);
//template uint16_t VkGuid::parseHex(const std::string&);
//template uint8_t  VkGuid::parseHex(const std::string&);
//
///* ------------------------------------------------------------------ */
///*  encode / decode                                                   */
///* ------------------------------------------------------------------ */
//std::string VkGuid::encodeBase64(const std::array<uint8_t, 16>& bytes) {
//    std::string ret; ret.reserve(22);
//    int i = 0, j = 0;
//    uint8_t a3[3], a4[4];
//    for (auto b : bytes) {
//        a3[i++] = b;
//        if (i == 3) {
//            a4[0] = (a3[0] & 0xfc) >> 2;
//            a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
//            a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
//            a4[3] = a3[2] & 0x3f;
//            for (j = 0; j < 4; ++j) ret += base64_chars[a4[j]];
//            i = 0;
//        }
//    }
//    if (i) {
//        for (j = i; j < 3; ++j) a3[j] = 0;
//        a4[0] = (a3[0] & 0xfc) >> 2;
//        a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
//        a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
//        a4[3] = a3[2] & 0x3f;
//        for (j = 0; j < i + 1; ++j) ret += base64_chars[a4[j]];
//    }
//    return ret;
//}
//
//std::array<uint8_t, 16> VkGuid::decodeBase64(const std::string& s) {
//    std::string padded = s;
//    while (padded.size() % 4) padded += '=';
//
//    std::array<uint8_t, 16> bytes{};
//    size_t len = padded.size(), i = 0, j = 0, pos = 0;
//    uint8_t a4[4], a3[3];
//
//    while (len-- && padded[pos] != '=') {
//        size_t idx = base64_chars.find(padded[pos++]);
//        if (idx == std::string::npos) throw std::runtime_error("Bad Base64");
//        a4[i++] = static_cast<uint8_t>(idx);
//        if (i == 4) {
//            a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
//            a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
//            a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
//            for (i = 0; i < 3; ++i) bytes[j++] = a3[i];
//            i = 0;
//        }
//    }
//    if (i) {
//        for (int k = i; k < 4; ++k) a4[k] = 0;
//        for (int k = 0; k < 4; ++k)
//            a4[k] = static_cast<uint8_t>(base64_chars.find(padded[pos - (4 - k)]));
//        a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
//        a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
//        a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
//        for (int k = 0; k < i - 1; ++k) bytes[j++] = a3[k];
//    }
//    if (j != 16) throw std::runtime_error("Invalid Base64 length");
//    return bytes;
//}
//
///* ------------------------------------------------------------------ */
///*  constructors / operators                                          */
///* ------------------------------------------------------------------ */
//VkGuid::VkGuid() : Data1(0), Data2(0), Data3(0), Data4{ 0 } {}
//
//VkGuid::VkGuid(const char* guidchar) {
//    std::string str(guidchar);
//    if (str.empty()) throw std::runtime_error("Empty GUID");
//    if (str.front() != '{') str = "{" + str + "}";
//    if (str.front() == '{') str.erase(0, 1);
//    if (str.back() == '}') str.pop_back();
//
//    std::vector<std::string> parts;
//    std::stringstream ss(str);
//    std::string part;
//    while (std::getline(ss, part, '-')) parts.push_back(part);
//    if (parts.size() != 5) throw std::runtime_error("Invalid GUID format");
//
//    Data1 = parseHex<uint32_t>(parts[0]);
//    Data2 = parseHex<uint16_t>(parts[1]);
//    Data3 = parseHex<uint16_t>(parts[2]);
//
//    if (parts[3].size() != 4 || parts[4].size() != 12)
//        throw std::runtime_error("Invalid Data4 format");
//
//    Data4[0] = parseHex<uint8_t>(parts[3].substr(0, 2));
//    Data4[1] = parseHex<uint8_t>(parts[3].substr(2, 2));
//    for (int i = 0; i < 6; ++i)
//        Data4[i + 2] = parseHex<uint8_t>(parts[4].substr(i * 2, 2));
//}
//
//VkGuid VkGuid::GenerateGUID() {
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<uint32_t> d32(0, std::numeric_limits<uint32_t>::max());
//    std::uniform_int_distribution<uint16_t> d16(0, std::numeric_limits<uint16_t>::max());
//    std::uniform_int_distribution<unsigned int> d8(0, 255);
//
//    VkGuid g;
//    g.Data1 = d32(gen);
//    g.Data2 = d16(gen);
//    g.Data3 = d16(gen);
//    for (auto& b : g.Data4) b = static_cast<uint8_t>(d8(gen));
//
//    g.Data3 = (g.Data3 & 0x0FFF) | 0x4000;   // version 4
//    g.Data4[0] = (g.Data4[0] & 0x3F) | 0x80; // variant 10
//    return g;
//}
//
//bool VkGuid::operator==(const VkGuid& rhs) const {
//    return std::memcmp(this, &rhs, sizeof(VkGuid)) == 0;
//}
//
///* ------------------------------------------------------------------ */
///*  string conversions                                                */
///* ------------------------------------------------------------------ */
//std::string VkGuid::ToString() const {
//    std::ostringstream oss;
//    oss << std::hex << std::setfill('0')
//        << '{' << std::setw(8) << Data1 << '-'
//        << std::setw(4) << Data2 << '-'
//        << std::setw(4) << Data3 << '-'
//        << std::setw(2) << static_cast<int>(Data4[0])
//        << std::setw(2) << static_cast<int>(Data4[1]) << '-'
//        << std::setw(2) << static_cast<int>(Data4[2])
//        << std::setw(2) << static_cast<int>(Data4[3])
//        << std::setw(2) << static_cast<int>(Data4[4])
//        << std::setw(2) << static_cast<int>(Data4[5])
//        << std::setw(2) << static_cast<int>(Data4[6])
//        << std::setw(2) << static_cast<int>(Data4[7])
//        << '}';
//    return oss.str();
//}
//
//std::string VkGuid::ToBase64() const {
//    std::array<uint8_t, 16> bytes{};
//    bytes[0] = (Data1 >> 0) & 0xFF;
//    bytes[1] = (Data1 >> 8) & 0xFF;
//    bytes[2] = (Data1 >> 16) & 0xFF;
//    bytes[3] = (Data1 >> 24) & 0xFF;
//    bytes[4] = (Data2 >> 0) & 0xFF;
//    bytes[5] = (Data2 >> 8) & 0xFF;
//    bytes[6] = (Data3 >> 0) & 0xFF;
//    bytes[7] = (Data3 >> 8) & 0xFF;
//    std::copy(Data4.begin(), Data4.end(), bytes.begin() + 8);
//    return encodeBase64(bytes);
//}
//
//VkGuid VkGuid::FromBase64(const std::string& b64) {
//    auto bytes = decodeBase64(b64);
//    VkGuid g;
//    g.Data1 = (static_cast<uint32_t>(bytes[3]) << 24) |
//        (static_cast<uint32_t>(bytes[2]) << 16) |
//        (static_cast<uint32_t>(bytes[1]) << 8) |
//        static_cast<uint32_t>(bytes[0]);
//    g.Data2 = (static_cast<uint16_t>(bytes[5]) << 8) | static_cast<uint16_t>(bytes[4]);
//    g.Data3 = (static_cast<uint16_t>(bytes[7]) << 8) | static_cast<uint16_t>(bytes[6]);
//    std::copy(bytes.begin() + 8, bytes.end(), g.Data4.begin());
//    return g;
//}
//
///* ------------------------------------------------------------------ */
///*  hash                                                              */
///* ------------------------------------------------------------------ */
//namespace std {
//    template <>
//    struct hash<VkGuid> {
//        size_t operator()(const VkGuid& g) const {
//            size_t h1 = hash<uint32_t>{}(g.Data1);
//            size_t h2 = hash<uint16_t>{}(g.Data2);
//            size_t h3 = hash<uint16_t>{}(g.Data3);
//            size_t h4 = 0;
//            for (auto b : g.Data4) h4 = (h4 << 8) | b;
//            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ h4;
//        }
//    };
//}