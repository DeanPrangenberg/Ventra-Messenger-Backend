//
// Created by deanprangenberg on 22.05.25.
//

#ifndef GENERATEID_H
#define GENERATEID_H

#include <QString>
#include "../Hash/BLAKE2b512.h"
#include <QUuid>

namespace Crypto {
class GenerateID {
public:
  static QString uuid();
  static QString hash(const QString input, size_t len);
};

} // Crypto

#endif //GENERATEID_H
