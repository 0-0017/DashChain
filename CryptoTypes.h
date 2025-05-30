#ifndef CRYPTOTYPES_H
#define CRYPTOTYPES_H

#include <openssl/evp.h>
#include <memory>
#include <stdexcept>

// Define a shared pointer type for EVP_PKEY
struct evp_pkey_st;  // Forward declaration for EVP_PKEY
using EVP_PKEY_ptr = std::shared_ptr<EVP_PKEY>;

// Custom deleter for EVP_PKEY pointers
struct EVP_PKEY_Deleter {
    void operator()(EVP_PKEY* pkey) const {
        if (pkey) {
            EVP_PKEY_free(pkey);
        }
    }
};

// Factory function to create EVP_PKEY shared pointers
inline EVP_PKEY_ptr createEVP_PKEY() {
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) {
        throw std::runtime_error("Failed to create EVP_PKEY");
    }
    return EVP_PKEY_ptr(pkey, EVP_PKEY_Deleter());
}

#endif // CRYPTOTYPES_H