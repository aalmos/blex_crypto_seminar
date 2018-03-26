#include "crypto_helpers.h"

#include <iomanip>
#include <sstream>
#include <openssl/rand.h>
#include <openssl/sha.h>

namespace crypto_helpers {
std::string Sha256(const std::string &input) {
  std::string hash;
  hash.resize(SHA256_DIGEST_LENGTH);
  SHA256_CTX sha256{};
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, input.c_str(), input.size());
  SHA256_Final(reinterpret_cast<uint8_t*>(hash.data()),
               &sha256);
  return hash;
}

std::string Sha256HexString(const std::string &input) {
  return BytesToHexString(Sha256(input));
}

std::string GenerateRandomBytes(size_t size) {
  std::string result;
  result.resize(size);
  RAND_bytes(reinterpret_cast<uint8_t *>(result.data()),
             static_cast<int>(size));
  return result;
}

std::string BytesToHexString(const std::string &input) {
  std::stringstream ss;
  for (char c : input) {
    ss << std::hex
       << std::setw(2)
       << std::setfill('0')
       << (uint32_t)reinterpret_cast<uint8_t&>(c);
  }
  return ss.str();
}
}