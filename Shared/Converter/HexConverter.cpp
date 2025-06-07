//
// Created by deanprangenberg on 07.06.25.
//

#include "HexConverter.h"

namespace Converter {
  std::string HexConverter::hexToString(const std::string &hex) {
    std::string result;
    for (size_t i = 0; i < hex.length(); i += 2) {
      std::string byte = hex.substr(i, 2);
      const char chr = static_cast<char>(static_cast<int>(strtol(byte.c_str(), nullptr, 16)));
      result.push_back(chr);
    }
    return result;
  }

  std::string HexConverter::stringToHex(const std::string &input) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (char c: input) {
      ss << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
    }
    return ss.str();
  }

  std::vector<uint8_t> HexConverter::hexToBytes(const std::string &hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
      std::string byteString = hex.substr(i, 2);
      auto byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
      bytes.push_back(byte);
    }
    return bytes;
  }

  std::string HexConverter::bytesToHex(const std::vector<uint8_t> &bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const uint8_t b: bytes) {
      ss << std::setw(2) << static_cast<int>(b);
    }
    return ss.str();
  }

  void HexConverter::printBytesAsHexErr(const std::string &label, const std::vector<uint8_t> &data) {
    std::cerr << label << " [" << data.size() << "] = ";
    for (const auto b: data) std::cerr << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    std::cerr << std::dec << std::endl;
  }

  void HexConverter::printBytesAsHex(const std::string &label, const std::vector<uint8_t> &data) {
    std::cout << label << " [" << data.size() << "] = ";
    for (const auto b: data) std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    std::cout << std::dec << std::endl;
  }
} // Converter