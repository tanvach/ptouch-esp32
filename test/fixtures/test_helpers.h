#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace TestHelpers {

// Helper function to convert bytes to hex string
std::string bytes_to_hex(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i > 0) ss << " ";
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str();
}

// Helper function to convert hex string to bytes
std::vector<uint8_t> hex_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

// Helper function to compare byte vectors
bool vectors_equal(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

} // namespace TestHelpers

// Stream operator for vector<uint8_t> to fix ASSERT_EQ compilation
inline std::ostream& operator<<(std::ostream& os, const std::vector<uint8_t>& vec) {
    os << "[" << TestHelpers::bytes_to_hex(vec) << "]";
    return os;
} 