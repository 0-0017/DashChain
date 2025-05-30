/*-- Wallet.cpp ------------------------------------------------------------
   This file implements Wallet member functions.
-------------------------------------------------------------------------*/
#include "Wallet.h"

/* Wallet Generation w/ Key Pairs */
Wallet::Wallet()
    : keyPair(generateECDSAKeyPair()),
    pubKeyP(extract_public_key())
{
    locktimeUTXO = 5;
    versionUTXO = 1.0;
    address = genAddress();
    txCount = 0;
    balance = 0;
}

EVP_PKEY_ptr Wallet::generateECDSAKeyPair() {
    /*
     * The libctx and prop-q can be set if required, they are included here
     * to show how they are passed to EVP_PKEY_CTX_new_from_name().
     */

    OSSL_LIB_CTX* libctx = nullptr;
    const char* propq = nullptr;
    EVP_PKEY_CTX* genctx = nullptr;
    EVP_PKEY_ptr key(nullptr, EVP_PKEY_free);
    OSSL_PARAM params[3];
    const char* curvename = "P-256";
    int use_cofactordh = 1;

    genctx = EVP_PKEY_CTX_new_from_name(libctx, "EC", propq);
    if (genctx == nullptr) {
        std::cout << "EVP_PKEY_CTX_new_from_name() failed\n";
        return nullptr;
    }

    if (EVP_PKEY_keygen_init(genctx) <= 0) {
        std::cout << "EVP_PKEY_keygen_init() failed\n";
        EVP_PKEY_CTX_free(genctx);
        return nullptr;
    }

    params[0] = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, (char*)curvename, 0);
    params[1] = OSSL_PARAM_construct_int(OSSL_PKEY_PARAM_USE_COFACTOR_ECDH, &use_cofactordh);
    params[2] = OSSL_PARAM_construct_end();
    if (!EVP_PKEY_CTX_set_params(genctx, params)) {
        std::cout << "EVP_PKEY_CTX_set_params() failed\n";
        EVP_PKEY_CTX_free(genctx);
        return nullptr;
    }

    EVP_PKEY* raw_key = nullptr;
    if (EVP_PKEY_generate(genctx, &raw_key) <= 0) {
        std::cout << "EVP_PKEY_generate() failed\n";
        EVP_PKEY_CTX_free(genctx);
        return nullptr;
    }

    key.reset(raw_key, EVP_PKEY_Deleter());
    EVP_PKEY_CTX_free(genctx);
    return key;
}

/* A function that takes a keypair and a message digest and signs it */
unsigned char* Wallet::ecDoSign(const EVP_PKEY_ptr& keypair, const std::vector<uint8_t>& mesdgst) {
    /* Method Variables */
    unsigned char* sig = nullptr;

    const unsigned char* md = mesdgst.data();
    size_t mdlen = mesdgst.size();

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        std::cerr << "ctx creation failed\n";
        return nullptr;
    }

    if (EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, keypair.get()) <= 0) {
        std::cerr << "sign_init Failed\n";
        EVP_MD_CTX_free(ctx);
        return nullptr;
    }

    if (EVP_DigestSignUpdate(ctx, md, mdlen) <= 0) {
        std::cerr << "sign_update Failed\n";
        EVP_MD_CTX_free(ctx);
        return nullptr;
    }

    size_t siglen;
    if (EVP_DigestSignFinal(ctx, nullptr, &siglen) <= 0) {
        std::cerr << "sign_final Failed\n";
        EVP_MD_CTX_free(ctx);
        return nullptr;
    }

    sig = static_cast<unsigned char*>(OPENSSL_malloc(siglen));
    if (sig == nullptr) {
        std::cerr << "Memory allocation failed for sig\n";
        EVP_MD_CTX_free(ctx);
        return nullptr;
    }

    if (EVP_DigestSignFinal(ctx, sig, &siglen) <= 0) {
        std::cerr << "sign_final Failed\n";
        EVP_MD_CTX_free(ctx);
        return nullptr;
    }

    EVP_MD_CTX_free(ctx);
    return sig;
}



/* A function that takes a keypair, a message digest, and a signature and verifies it */
bool Wallet::ecDoVerify(const EVP_PKEY_ptr& pkey, const std::vector<uint8_t>& mesdgst, const std::vector<unsigned char>& signature) const {
    /* Method Variables */
    const unsigned char* md = mesdgst.data();
    size_t mdlen = mesdgst.size();
    const unsigned char* sig = signature.data();
    size_t siglen = signature.size();

    /* Create a digest verify context */
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        std::cerr << "ctx creation failed\n";
        return false;
    }

    /* Initialize the digest verify operation */
    if (EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, pkey.get()) <= 0) {
        std::cerr << "verify_init Failed\n";
        EVP_MD_CTX_free(ctx);
        return false;
    }

    /* Update the digest verify operation with the message digest */
    if (EVP_DigestVerifyUpdate(ctx, md, mdlen) <= 0) {
        std::cerr << "verify_update Failed\n";
        EVP_MD_CTX_free(ctx);
        return false;
    }

    /* Finalize the digest verify operation and get the verification result */
    int ret = EVP_DigestVerifyFinal(ctx, sig, siglen);
    std::cout << "Verified: " << ret << std::endl;

    /* Clean up memory */
    EVP_MD_CTX_free(ctx);

    /* Return the verification result as a boolean */
    return ret == 1;
}

EVP_PKEY_ptr Wallet::extract_public_key(){
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        std::cerr << "Error creating BIO\n";
        return nullptr;
    }

    // Write the public key to the BIO
    if (PEM_write_bio_PUBKEY(bio, keyPair.get()) != 1) {
        std::cerr << "Error writing public key to BIO\n";
        BIO_free(bio);
        return nullptr;
    }

    // Read the public key from the BIO
    EVP_PKEY_ptr temp;
    temp.reset(PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr), EVP_PKEY_free);
    if (!temp) {
        std::cerr << "Error reading public key from BIO\n";
    }

    BIO_free(bio);
    return temp;
}

std::string Wallet::genAddress() {
    // Get DER size and create buffer
    int len = i2d_PUBKEY(pubKeyP.get(), nullptr);
    std::vector<unsigned char> keyBytes(len);
    unsigned char* bufferStart = keyBytes.data();  // Save the start pointer
    unsigned char* temp = bufferStart;  // Use temp for i2d_PUBKEY to advance
    i2d_PUBKEY(pubKeyP.get(), &temp);

    // Use the beginning of the DER encoding for hashing:
    size_t derSize = keyBytes.size();  // which is len
    // Get RIPEMD-160 digest size properly using EVP_MD_fetch
    OSSL_LIB_CTX* libctx = nullptr;
    EVP_MD* md = EVP_MD_fetch(libctx, "RIPEMD-160", nullptr);
    if (!md) {
        std::cerr << "EVP_MD_fetch failed for RIPEMD-160\n";
        return "";
    }
    size_t ripemdDigestSize = EVP_MD_size(md);
    EVP_MD_free(md);

    // IMPORTANT: Pass bufferStart (not temp) to ripemd()
    unsigned char* ripemdHash = utility.ripemd(bufferStart, derSize);
    if (!ripemdHash) {
        return "";
    }
    std::vector<uint8_t> publicKeyHash = utility.shaHash(ripemdHash, ripemdDigestSize);
    // Free the memory allocated by ripemd() once used
    OPENSSL_free(ripemdHash);

    /* Step 3: Prepend version bytes and compute Base58 */
    std::vector<uint8_t> extPubKeyHash = { 0x00, 0x17 };
    extPubKeyHash.insert(extPubKeyHash.end(), publicKeyHash.begin(), publicKeyHash.end());

    std::string addr = utility.base58_encode(extPubKeyHash.data(), extPubKeyHash.size());
    std::cout << "Wallet Address: " << addr << std::endl;
    return addr;
}

utxout Wallet::outUTXO(double feee, const std::vector<std::string>& rwa, const std::vector<EVP_PKEY_ptr>& rks, const std::vector<double>& amm,
    const std::vector<std::string> &delegates, const std::vector<std::string> &delegateID, const std::vector<std::tuple<std::string, std::string, float>> &votesQueue) {
    /* Structure UTXO */
    transactions utxo(address, rwa, pubKeyP, rks, amm, feee, locktimeUTXO, versionUTXO, delegates, delegateID, votesQueue);

    /* Check for balance */
    setBalance();
    double bal = getBalance();
    double check = 0;
    std::vector<double> checkbal = utxo.getAmmount();
    for (double amount : checkbal) {
        check += amount;
    }
    check += feee;

    if (check > bal) {
        std::cout << "Amount Exceeds Balance!\n";
        return {};
    }

    /* Sign UTXO */
    std::shared_ptr<unsigned char> utxoHash(utxo.serialize());
    size_t dataSize = 0;
    std::memcpy(&dataSize, utxoHash.get(), sizeof(size_t));
    std::vector<uint8_t> utxoHashed = utility.shaHash(utxoHash.get(), dataSize);
    unsigned char* utxoSignedHash = ecDoSign(keyPair, utxoHashed);

    /* Setup tx & Signed Message for mempool */
    utxout out;
    size_t testsz = 0;
    unsigned char* testSer = utxo.serialize();
    std::memcpy(&testsz, testSer, sizeof(size_t));
    size_t utxo_size = utxo.getSize();
    size_t sh_size = getSignSize(utxoHashed);

    if (testsz == utxo_size) {
        out.txSize = testsz;
        out.shSize = sh_size;
        out.utxo = std::shared_ptr<unsigned char>(testSer, std::default_delete<unsigned char[]>());
        out.utxoSignedHash = std::shared_ptr<unsigned char>(utxoSignedHash, std::default_delete<unsigned char[]>());
    }
    else {
        std::cout << "TX OUT SIZE INVALID\n";
        return {};
    }

    /* Update UTXO */
    double utxoup = 0;
    std::vector<transactions> newUTXO;
    for (const auto& tx: UTXO) {
        std::vector<double> vecam = tx.getAmmount();
        for (double amount : vecam) {
            utxoup += amount;
        }

        if (check <= utxoup) {
            if (utxoup == check) {
                check = 0.0;
                continue;
            }
            else {
                std::vector<std::string> del;
                std::vector<std::string> delID;
                std::vector<std::tuple<std::string, std::string, float>> votesQ;
                std::string mwa = address;
                std::vector<std::string> mra;
                mra.push_back(mwa);
                std::vector<EVP_PKEY_ptr> mrpk;
                std::vector<double> namm;
                double mamm = (utxoup - check);
                namm.push_back(mamm);
                transactions ttx(address, mra, pubKeyP, mrpk, namm, 0, locktimeUTXO, versionUTXO, del,
                    delID, votesQ, util::TimeStamp());
                newUTXO.emplace_back(ttx);
                check = 0.0;
                continue;
            }
        }
        else {
            check -= utxoup;
            continue;
        }
    }
    UTXO = std::move(newUTXO);
    UTXO.shrink_to_fit();
    setBalance();

    return out;
}

void Wallet::inUTXO(const transactions& txin) {
    UTXO.emplace_back(txin);
    setBalance();
}

bool Wallet::verifyTx(const utxout& out) {
    /* Declare variables */
    transactions utxo = transactions::deserialize(out.utxo.get());
    EVP_PKEY_ptr sendpk = utxo.getSendPkey();

    /* Verify Hash */
    std::shared_ptr<unsigned char> utxoHash(utxo.serialize());
    size_t dataSize = 0;
    std::memcpy(&dataSize, utxoHash.get(), sizeof(size_t));
    std::vector<uint8_t> utxoHashed = utility.shaHash(utxoHash.get(), dataSize);
    unsigned char* utxoSignedHash = ecDoSign(sendpk, utxoHashed);

    if (std::memcmp(utxoSignedHash, out.utxoSignedHash.get(), out.shSize) != 0) {
        std::cout << "UTXO INVALID!\n";
        return false;
    }

    /* Verify UTXO amount > zero */
    double totalbal = 0.0;
    std::vector<double> tempBal = utxo.getAmmount();
    for (double amount : tempBal) {
        totalbal += amount;
    }
    if (totalbal <= 0) {
        std::cout << "UTXO amount is invalid (must be greater than 0)!\n";
        return false;
    }

    return true;
}

void Wallet::setBalance() {
    double amm = 0;
    size_t txsize = UTXO.size();
    for (int i = 0; i < txsize; i++) {
        amm += UTXO[i].totalAmm();
    }

    balance = amm;
}

double Wallet::getBalance() const {
    return balance;
}

std::string Wallet::getWalletAddr() const {
    return address;
}

EVP_PKEY_ptr Wallet::getPubKey() const {
    return pubKeyP;
}

unsigned short Wallet::getLockTime() const {
    return locktimeUTXO;
}

void Wallet::setLockTime(unsigned short lk) {
    locktimeUTXO = lk;
}

float Wallet::getVersion() const {
    return versionUTXO;
}

void Wallet::setVersion(float vs) {
    versionUTXO = vs;
}

unsigned char* Wallet::serialize_utxout(const utxout& obj) const {

    /* Calculate sizes */
    size_t tSize = 0;

    tSize += sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + obj.txSize + obj.shSize;

    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    /* serialize utxout */

    /* Serialize tSize itself */
    std::memcpy(buffer + offset, &tSize, sizeof(tSize));
    offset += sizeof(tSize);

    /* Serialize txSize itself */
    std::memcpy(buffer + offset, &obj.txSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize shSize itself */
    std::memcpy(buffer + offset, &obj.shSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize utxo itself */
    std::memcpy(buffer + offset, obj.utxo.get(), obj.txSize);
    offset += obj.txSize;

    /* Serialize utxo itself */
    std::memcpy(buffer + offset, obj.utxoSignedHash.get(), obj.shSize);
    offset += obj.shSize;

    return buffer;
}

utxout Wallet::deserialize_utxout(const unsigned char* buffer) const {
    utxout obj;
    size_t offset = 0;

    /* Deserialize tSize (not used in this function, but read to advance the offset) */
    size_t tSize;
    std::memcpy(&tSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize txSize */
    std::memcpy(&obj.txSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize shSize */
    std::memcpy(&obj.shSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize utxo */
    obj.utxo = std::shared_ptr<unsigned char>(new unsigned char[obj.txSize], std::default_delete<unsigned char[]>());
    std::memcpy(obj.utxo.get(), buffer + offset, obj.txSize);
    offset += obj.txSize;

    /* Deserialize utxoSignedHash */
    obj.utxoSignedHash = std::shared_ptr<unsigned char>(new unsigned char[obj.shSize], std::default_delete<unsigned char[]>());
    std::memcpy(obj.utxoSignedHash.get(), buffer + offset, obj.shSize);
    offset += obj.shSize;

    return obj;
}