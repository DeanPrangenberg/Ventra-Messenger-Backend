#ifndef DOUBLERATCHET_H
#define DOUBLERATCHET_H

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include "../../Converter/HexConverter.h"
#include "../KeyEnv/KeyEnv.h"
#include "../Encryption/EncryptionEnv.h"
#include "../KDF/KDFEnv.h"
#include "../Hash/HashingEnv.h"

// Construction types: direct INIT, FOLLOW with existing State or DEFERRED manually
enum class ConstructType { INIT, EXISTING, FOLLOWINIT };

enum class SessionType { DUO, MULTI };

struct RatchetHeader {
  std::vector<uint8_t> iv;
  std::vector<uint8_t> authTag;
  std::vector<uint8_t> SenderPubKey;
  std::vector<uint8_t> ReceiverPubKey;
  uint32_t sendMessageNum;
  uint32_t messageLength;
};

struct RatchetState {
  SessionType sessionType;
  std::vector<uint8_t> sharedSecret;
  std::vector<uint8_t> rootKey;
  std::vector<uint8_t> ownPrivKey;
  std::vector<uint8_t> ownPubKey;
  std::vector<uint8_t> theirPubKey;

  std::vector<uint8_t> sendChainKey;
  std::map<uint32_t, std::vector<uint8_t> > send_message_keys;
  uint32_t send_msg_num = 0;

  std::vector<uint8_t> recvChainKey;
  std::map<uint32_t, std::vector<uint8_t> > recv_message_keys;
  uint32_t recv_msg_num = 0;
};

class DoubleRatchet {
public:
  // Construction
  DoubleRatchet(SessionType sessionType,
                ConstructType constructType,
                RatchetState *ratchetState = nullptr,
                std::unique_ptr<Crypto::KeyEnv> keyEnv = nullptr,
                const std::vector<uint8_t> &theirPubKey = {});

  // Manual initialization:
  // 1) Generate keypair
  bool generateKeypair();

  // 2) Derive SharedSecret
  bool deriveSharedSecret(const std::vector<uint8_t> &theirPubKey);

  // 3) Initialize RootKey and SendChain
  bool initRootChain();

  // Root-Key-Update
  // Updates the root key and performs an asymmetric ratchet step using the provided public key.
  bool updateRootKey(const std::vector<uint8_t> &newPubKey);

  // Message package
  std::string packEncMessage(const std::string &plaintext);

  std::string unpackDecMessage(const std::string &package);

  // Access to State
  RatchetState *getState() const;

  std::vector<uint8_t> ownPubKey() const;

  static bool testOneSideDoubleRatchet();

  static bool testMixedDoubleRatchet();

  static bool testMultiChat();

private:
  bool setState(RatchetState *rs);

  bool importKeyEnv(std::unique_ptr<Crypto::KeyEnv> keyEnv);

  bool encryptMessage(const std::vector<uint8_t> &message,
                      std::vector<uint8_t> &ciphertext,
                      std::vector<uint8_t> &authTag,
                      std::vector<uint8_t> &iv
  );

  bool decryptMessage(const std::vector<uint8_t> &ciphertext,
                      std::vector<uint8_t> &message,
                      const std::vector<uint8_t> &authTag,
                      const std::vector<uint8_t> &iv,
                      uint32_t sendMessageNum
  );

  // Internal functions
  bool initNewSession(const std::vector<uint8_t> &theirPub);

  bool initFollowSession(const std::vector<uint8_t> &theirPub);

  bool symmetricRatchetStep();

  bool receiveSymmetricRatchetStep(uint32_t msg_num); // New function for receive chain
  bool asymmetricRatchetStep(const std::vector<uint8_t> &theirPub);

  std::unique_ptr<RatchetState> state;
  std::unique_ptr<Crypto::KeyEnv> ownKeyEnv;
  std::unique_ptr<Crypto::KeyEnv> theirKeyEnv;
  std::unique_ptr<Crypto::EncryptionEnv> encryptionEnv;
  std::unique_ptr<Crypto::EncryptionEnv> decryptionEnv;
  std::unique_ptr<Crypto::KDFEnv> kdfEnv;
  std::unique_ptr<Crypto::HashingEnv> hashingEnv;
};

#endif // DOUBLERATCHET_H

