#include "WebSocketClient.h"

namespace Network {
  WebSocketClient::WebSocketClient(const QUrl &url, QObject *parent) : QObject(parent), serverUrl(url),
                                                                       handshakeDone(false) {
    connect(&socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);

    connect(&socket, &QWebSocket::textMessageReceived, this, [=](const QString &message) {
      QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
      if (!doc.isObject()) return;
      QJsonObject obj = doc.object();
      if (obj["type"].toString() == "HandshakeAck") {
        std::string serverKeyStr = obj["serverPubKey"].toString().toStdString();
        std::cout << "Received HandshakeAck with server public key: " << serverKeyStr << std::endl;
        QByteArray serverKeyBytes = QByteArray::fromBase64(QString::fromStdString(serverKeyStr).toUtf8());
        std::vector<uint8_t> serverKeyVec(serverKeyBytes.begin(), serverKeyBytes.end());

        Converter::HexConverter::printBytesAsHex("serverKeyVec Hex", serverKeyVec);

        auto tmpSharedSecret = keyPairEnv->deriveSharedSecret(serverKeyVec);
        hashingEnv->plainData = tmpSharedSecret;
        hashingEnv->startHashing();

        sharedSecret = hashingEnv->hashValue;

        std::cout << "Handshake acknowledged. Shared secret derived." << std::endl;
        Converter::HexConverter::printBytesAsHex("SharedSecret", sharedSecret);

        handshakeDone = true;
        testPacket();
      } else {
        std::cout << "Received unknown message type: " << obj["type"].toString().toStdString() << std::endl;
      }
    });

    socket.open(serverUrl);

    encEnv = std::make_unique<Crypto::EncryptionEnv>(Crypto::EncAlgorithm::AES256);
    keyPairEnv = std::make_unique<Crypto::KeyEnv>(Crypto::KeyType::X25519Keypair);
    keyPairEnv->startKeyPairGeneration();
    ivEnv = std::make_unique<Crypto::KeyEnv>(Crypto::KeyType::KeyIv);
    hashingEnv = std::make_unique<Crypto::HashingEnv>(Crypto::HashAlgorithm::BLAKE2s256);
  }

  void WebSocketClient::onConnected() {
    std::cout << "Connected to server: " << serverUrl.toString().toStdString() << std::endl;
    sendHandshakeData();
  }

  void WebSocketClient::sendHandshakeData() {
    auto base64ClientPubKey = keyPairEnv->getPublicBase64();
    std::cout << "Client Key: " << base64ClientPubKey << std::endl;

    QJsonObject handshakePkg;
    handshakePkg["type"] = "Handshake";
    handshakePkg["pkg"] = QString::fromStdString(base64ClientPubKey);

    socket.sendTextMessage(Packages::convertPkgToJsonStr(handshakePkg));
  }

  void WebSocketClient::testPacket() {
    if (!handshakeDone) return;
    Packages pack;
    for (int i = 0; i < 1; ++i) {
      std::cout << "Sending test packet " << i + 1 << std::endl;
      QJsonObject messageData = pack.testMessage();

      // Serialize messageData to JSON string (this is the plaintext)
      QString messageJson = Packages::convertPkgToJsonStr(messageData);
      std::string messageStr = messageJson.toStdString();
      std::vector<uint8_t> messageVec(messageStr.begin(), messageStr.end());

      // Generate IV
      std::vector<uint8_t> key, iv;
      ivEnv->setKeyIvSizes(1, 12);
      ivEnv->startKeyIvGeneration(key, iv);

      // Prepare encryption
      std::vector<uint8_t> authTag(16, 0);
      encEnv->iv = iv;
      encEnv->authTag = authTag;
      encEnv->key = sharedSecret;
      encEnv->plaintext = messageVec;
      encEnv->ciphertext.clear();

      encEnv->startEncryption();

      // Add Authtag to ciphertext for gos decryption
      encEnv->ciphertext.insert(encEnv->ciphertext.end(), encEnv->authTag.begin(), encEnv->authTag.end());

      QByteArray base64Ciphertext = QByteArray::fromStdString(
        std::string(encEnv->ciphertext.begin(), encEnv->ciphertext.end())
      ).toBase64();
      QByteArray base64IV = QByteArray::fromStdString(
        std::string(iv.begin(), iv.end())
      ).toBase64();

      // Build package: { "type": "MessagePkg", "pkg": "<base64-ciphertext>", "IV": "<base64-iv>" }
      QJsonObject pkg;
      pkg["type"] = "MessagePkg";
      pkg["pkg"] = QString(base64Ciphertext);
      pkg["IV"] = QString(base64IV);

      socket.sendTextMessage(Packages::convertPkgToJsonStr(pkg));
      sleep(1);
    }
  }
}
