//
// Created by deanprangenberg on 07.06.25.
//

#ifndef HEXCONVERTER_H
#define HEXCONVERTER_H

#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

namespace Converter {

class HexConverter {
public:
  static std::string hexToString(const std::string &hex);

  static std::string stringToHex(const std::string &input);

  static std::vector<uint8_t> hexToBytes(const std::string &hex);

  static std::string bytesToHex(const std::vector<uint8_t> &bytes);

  static void printBytesAsHex(const std::string &label, const std::vector<uint8_t> &data);

  static void printBytesAsHexErr(const std::string &label, const std::vector<uint8_t> &data);
};

} // Converter

#endif //HEXCONVERTER_H
