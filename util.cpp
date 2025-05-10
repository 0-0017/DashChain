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

std::vector<uint8_t> util::shaHash(const unsigned char* data, bool isString){
    const EVP_MD* algorithm = nullptr;
    size_t dSize;

    /* If unsigned char || string */
    if (isString) {
        algorithm = EVP_sha3_256(); // Use SHA-3 for string data
        dSize = sizeof(data);
    }
    else {
        algorithm = EVP_sha256(); // Use SHA-256 for non-string data
        dSize = sizeof(data);
    }

    uint8_t digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length;
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, algorithm, nullptr);
    EVP_DigestUpdate(context, data, dSize);
    EVP_DigestFinal_ex(context, digest, &digest_length);
    EVP_MD_CTX_destroy(context);

    /* Convert the digest to a vector of unsigned chars */
    std::vector<uint8_t> hash_value(digest, digest + digest_length);
    return hash_value;
}

unsigned char* util::ripemd(const unsigned char* pubKey, size_t ucSize){

    /* Function Variables */
    size_t size = ucSize;
    unsigned char* d = new unsigned char[size];
    std::memcpy(d, pubKey, size);
    unsigned int mdSize;
    unsigned char* md = (unsigned char*)OPENSSL_malloc(EVP_MD_size(EVP_ripemd160()));
    unsigned long err = 0; // Variable to store the error code //TEMPP


    /* RIPEMD-160 cryptographic hash function with a 160 bit output. */
    // Create a digest verify context
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        std::cerr << "ctx creation failed\n";
        goto cleanup;
    }

    if (EVP_DigestInit_ex(ctx, EVP_ripemd160(), NULL) < 1) {
        std::cerr << "Initiation failed\n";
        goto cleanup;
    }

    /* Update the hash with the data */
    if (EVP_DigestUpdate(ctx, d, size) < 1) {
        std::cerr << "Update failed\n";
        goto cleanup;
    }

    mdSize = sizeof(md);
    /* Finalize the digest sign operation */
    if (EVP_DigestFinal_ex(ctx, md, &mdSize) < 1) {
        std::cerr << "Hash Failed\n";
        goto cleanup;
    }

    std::cout << "RIPEMD MSG: " << md << std::endl;
    return md;

cleanup:
    EVP_MD_CTX_free(ctx);
    return md;
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
    std::vector<unsigned char> temp(size);
    std::copy(bytes, bytes + size, temp.begin());

    /* Count leading zeros */
    size_t leadingZeros = 0;
    while (leadingZeros < temp.size() && temp[leadingZeros] == 0) {
        leadingZeros++;
    }

    /* Allocate enough space in big-endian base58 representation */
    std::vector<unsigned char> base58(size * 138 / 100 + 1); // log(256) / log(58), rounded up

    /* Process the bytes */
    size_t index = base58.size();
    for (size_t i = leadingZeros; i < temp.size(); ++i) {
        unsigned int carry = temp[i];
        for (size_t j = base58.size(); j > 0; --j) {
            carry += 256 * base58[j - 1];
            base58[j - 1] = carry % 58;
            carry /= 58;
        }
    }

    /* Skip leading zeros in base58 result */
    size_t start = base58.size() - index;
    result.reserve(leadingZeros + start);
    result.assign(leadingZeros, '1');
    for (size_t i = start; i < base58.size(); ++i) {
        result.push_back(base58[i] != 0 ? base58_chars[base58[i]] : '1');
    }

    return result;
}
