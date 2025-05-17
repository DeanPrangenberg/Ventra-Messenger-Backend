//
// Created by deanprange on 3/16/25.
//

#ifndef HELPERUTILS_H
#define HELPERUTILS_H

#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

class HelperUtils {
public:
  static std::string hexToString(const std::string &hex);

  static std::string stringToHex(const std::string &input);

  static std::vector<uint8_t> hexToBytes(const std::string &hex);

  static std::string bytesToHex(const std::vector<uint8_t> &bytes);

  static void printBytes(const std::vector<uint8_t> &bytes);

  static void printBytesErr(const std::vector<uint8_t> &bytes);

  static void printBytesAsHex(const std::string &label, const std::vector<uint8_t> &data);

  static void printBytesAsHexErr(const std::string &label, const std::vector<uint8_t> &data);
};


#endif //HELPERUTILS_H
