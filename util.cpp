/*-- Util.cpp ------------------------------------------------------------
   This file implements Utility member functions.
-------------------------------------------------------------------------*/
#include "util.h"

unsigned long long util::TimeStamp(){
    /* Get the current time point */
    auto currentTime = std::chrono::system_clock::now();

    /* Convert time point to duration since epoch */
    auto durationSinceEpoch = currentTime.time_since_epoch();

    /* Convert duration to seconds */
    auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch).count();

    logCall("UTIL", "TimeStamp()", true);
    return (unsigned long long)secondsSinceEpoch;
}

#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <iostream>
#include <vector>

bool util::shaHash(const std::string &message, std::vector<unsigned char> &hash) {
    hash.resize(SHA512_DIGEST_LENGTH);
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) return false;

    if (EVP_DigestInit_ex(mdctx, EVP_sha3_512(), nullptr) &&
        EVP_DigestUpdate(mdctx, message.data(), message.size()) &&
        EVP_DigestFinal_ex(mdctx, hash.data(), nullptr)) {
        EVP_MD_CTX_free(mdctx);
        logCall("UTIL", "shaHash()", true);
        return true;
        }

    EVP_MD_CTX_free(mdctx);
    logCall("UTIL", "shaHash()", false, "SHAHASH Failed");
    return false;
}

bool util::ripemd(const std::vector<unsigned char> &input, std::vector<unsigned char> &hash){
    hash.resize(RIPEMD160_DIGEST_LENGTH * 2); // RIPEMD-320 is twice the length of RIPEMD-160
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) return false;

    if (EVP_DigestInit_ex(mdctx, EVP_ripemd160(), nullptr) && // OpenSSL uses RIPEMD-160 alias
        EVP_DigestUpdate(mdctx, input.data(), input.size()) &&
        EVP_DigestFinal_ex(mdctx, hash.data(), nullptr)) {
        EVP_MD_CTX_free(mdctx);
        logCall("UTIL", "ripemd()", true);
        return true;
        }

    EVP_MD_CTX_free(mdctx);
    logCall("UTIL", "ripemd()", false, "RIPEMD Hash Failed");
    return false;
}

std::string util::genRandNum(){
    /* Create a random device and a Mersenne Twister engine */
    std::random_device rd;
    std::mt19937 mt(rd());

    /* Create a uniform distribution of digits from 0 to 9 */
    std::uniform_int_distribution<int> dist(0, 9);

    /* Generate 17 random digits and append them to a string */
    std::string result;
    for (int i = 0; i < 17; i++)
    {
        result += std::to_string(dist(mt));
    }

    logCall("UTIL", "genRandNum()", true);
    return result;
}

std::string util::getPasswordFromUser(){
    std::string passphrase;
    std::cout << "Enter passphrase for private key encryption: ";
    std::getline(std::cin, passphrase);
    return passphrase;
}


std::string util::base58_encode(const unsigned char* bytes, size_t size) {
    const std::string base58_chars = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    std::string result;

    /* Convert big-endian bytes to little-endian */
    std::vector<unsigned char> temp(bytes, bytes + size);
    std::copy(bytes, bytes + size, temp.begin());

    /* Count leading zeros */
    size_t leadingZeros = 0;
    while (leadingZeros < temp.size() && temp[leadingZeros] == 0) {
        leadingZeros++;
    }

    /* Allocate enough space in big-endian base58 representation */
    std::vector<unsigned char> base58(size * 138 / 100 + 1, 0); // log(256) / log(58), rounded up

    /* Process the bytes */
    size_t index = base58.size();
    for (size_t i = 0; i < temp.size(); ++i) {
        unsigned int carry = temp[i];
        for (size_t j = base58.size(); j > 0; --j) {
            carry += 256 * base58[j - 1];
            base58[j - 1] = carry % 58;
            carry /= 58;
        }
    }

    /* Preserve leading zeros in base58 result */
    result.reserve(leadingZeros + base58.size());
    result.assign(leadingZeros, '1');
    for (unsigned char c : base58) {
        if (c != 0) {
            result.push_back(base58_chars[c]);
        }
    }

    logCall("UTIL", "base58_encode()", true);
    return result;
}