//
// Created by deanprangenberg on 22.05.25.
//

#include "GenerateID.h"

#include "../Hash/HashingEnv.h"

namespace Crypto {
  QString GenerateID::uuid() {
      QUuid uuid = QUuid::createUuid();
      return uuid.toString(QUuid::WithBraces);
  }

  QString GenerateID::hash(const QString input, size_t len) {
      QString output;
      QString currentInput = input;

      while (output.length() < len * 2) {
          HashingEnv hashingEnv(HashAlgorithm::BLAKE2b512);

          std::string transStr = currentInput.toStdString();
          std::vector<uint8_t> inputValue(transStr.begin(), transStr.end());

          hashingEnv.plainData = inputValue;
          hashingEnv.startHashing();

          QString hashPart;
          for (size_t i = 0; i < 64; ++i) {
              hashPart += QString::number(hashingEnv.hashValue[i], 16).rightJustified(2, '0');
          }

          output += hashPart;

          currentInput = input + hashPart;
      }

      return output.left(len * 2);
  }
} // Crypto