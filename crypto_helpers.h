#ifndef CRYPTO_HELPERS_H
#define CRYPTO_HELPERS_H

#include <string>

namespace crypto_helpers {
std::string Sha256(const std::string& input);

std::string Sha256HexString(const std::string& input);

std::string GenerateRandomBytes(size_t size);

std::string BytesToHexString(const std::string& input);
}

#endif //CRYPTO_HELPERS_H
