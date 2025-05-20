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

    return (unsigned long long)secondsSinceEpoch;
}

#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <iostream>
#include <vector>

std::vector<uint8_t> util::shaHash(const unsigned char* data, size_t dataSize) {
    /* Viability Test*/
    if (!data || dataSize == 0) {
        std::cerr << "Invalid input to SHA hashing\n";
        return {};
    }

    // Fetch the optimized SHA-2 digest algorithm
    OSSL_LIB_CTX* libctx = nullptr;
    const char* algorithmName = "SHA-256";
    EVP_MD* md = EVP_MD_fetch(libctx, algorithmName, nullptr);
    if (!md) {
        std::cerr << "EVP_MD_fetch failed for algorithm: " << algorithmName << "\n";
        return {};
    }

    // Create a digest context
    std::vector<uint8_t> digest(EVP_MD_size(md));
    unsigned int digestLength = EVP_MD_size(md);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        std::cerr << "Failed to create EVP_MD_CTX\n";
        EVP_MD_free(md);
        return {};
    }

    // Initialize Digest Context
    if (EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
        std::cerr << "SHA Digest initialization failed\n";
        goto cleanup;
    }

    // Update Digest Context with input data
    if (EVP_DigestUpdate(ctx, data, dataSize) != 1) {
        std::cerr << "SHA Digest update failed\n";
        goto cleanup;
    }

    // Finalize the digest and retrieve the hash output
    if (EVP_DigestFinal_ex(ctx, digest.data(), &digestLength) != 1) {
        std::cerr << "SHA Digest finalization failed\n";
        goto cleanup;
    }

    // Cleanup
    EVP_MD_CTX_free(ctx);
    EVP_MD_free(md);
    return digest;

cleanup:
    EVP_MD_CTX_free(ctx);
    EVP_MD_free(md);
    return {};
}

unsigned char* util::ripemd(const unsigned char* pubKey, size_t ucSize){
    if (!pubKey || ucSize == 0) {
        std::cerr << "Invalid input to RIPEMD-160\n";
        return nullptr;
    }

    // Fetch the optimized RIPEMD-160 digest algorithm
    OSSL_LIB_CTX* libctx = nullptr;
    EVP_MD* md = EVP_MD_fetch(libctx, "RIPEMD-160", nullptr);
    if (!md) {
        std::cerr << "EVP_MD_fetch for RIPEMD-160 failed\n";
        return nullptr;
    }

    // Allocate buffer for digest output
    size_t digestSize = EVP_MD_size(md);
    unsigned char* hash = static_cast<unsigned char*>(OPENSSL_malloc(digestSize));
    if (!hash) {
        std::cerr << "Memory allocation for RIPEMD hash failed\n";
        EVP_MD_free(md);
        return nullptr;
    }

    // Create a digest context
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        std::cerr << "EVP_MD_CTX_new failed\n";
        OPENSSL_free(hash);
        EVP_MD_free(md);
        return nullptr;
    }

    // Initialize Digest Context for RIPEMD-160
    if (EVP_DigestInit_ex2(ctx, md, nullptr) != 1) {
        std::cerr << "RIPEMD-160 Digest initialization failed\n";
        goto cleanup;
    }

    // Update Digest Context with input data
    if (EVP_DigestUpdate(ctx, pubKey, ucSize) != 1) {
        std::cerr << "RIPEMD-160 Digest update failed\n";
        goto cleanup;
    }

    // Finalize and retrieve the hash output
    if (EVP_DigestFinal_ex(ctx, hash, nullptr) != 1) {
        std::cerr << "RIPEMD-160 Digest finalization failed\n";
        goto cleanup;
    }

    // Cleanup
    EVP_MD_CTX_free(ctx);
    EVP_MD_free(md);
    return hash;

cleanup:
    EVP_MD_CTX_free(ctx);
    EVP_MD_free(md);
    OPENSSL_free(hash);
    return nullptr;

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

    return result;
}