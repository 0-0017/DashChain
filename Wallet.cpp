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
    util::logCall("WALLET", "Wallet()", true);
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
        util::logCall("WALLET", "generateECDSAKeyPair()", false, "EVP_PKEY_CTX_new_from_name() failed");
        std::cout << "EVP_PKEY_CTX_new_from_name() failed\n";
        return nullptr;
    }

    if (EVP_PKEY_keygen_init(genctx) <= 0) {
        util::logCall("WALLET", "generateECDSAKeyPair()", false, "EVP_PKEY_keygen_init() failed");
        std::cout << "EVP_PKEY_keygen_init() failed\n";
        EVP_PKEY_CTX_free(genctx);
        return nullptr;
    }

    params[0] = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, (char*)curvename, 0);
    params[1] = OSSL_PARAM_construct_int(OSSL_PKEY_PARAM_USE_COFACTOR_ECDH, &use_cofactordh);
    params[2] = OSSL_PARAM_construct_end();
    if (!EVP_PKEY_CTX_set_params(genctx, params)) {
        util::logCall("WALLET", "generateECDSAKeyPair()", false, "EVP_PKEY_CTX_set_params() failed");
        std::cout << "EVP_PKEY_CTX_set_params() failed\n";
        EVP_PKEY_CTX_free(genctx);
        return nullptr;
    }

    EVP_PKEY* raw_key = nullptr;
    if (EVP_PKEY_generate(genctx, &raw_key) <= 0) {
        util::logCall("WALLET", "generateECDSAKeyPair()", false, "EVP_PKEY_generate() failed");
        std::cout << "EVP_PKEY_generate() failed\n";
        EVP_PKEY_CTX_free(genctx);
        return nullptr;
    }

    key.reset(raw_key, EVP_PKEY_Deleter());
    EVP_PKEY_CTX_free(genctx);
    util::logCall("WALLET", "generateECDSAKeyPair()", true);
    return key;
}

/* A function that takes a keypair and a message digest and signs it */
bool Wallet::ecDoSign(const std::vector<unsigned char> &hash, std::vector<unsigned char> &signature) const {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) return false;

    if (!EVP_DigestSignInit(mdctx, nullptr, EVP_sha3_512(), nullptr, keyPair.get())) {
        EVP_MD_CTX_free(mdctx);
        util::logCall("WALLET", "ecDoSign()", false, "init() failed");
        return false;
    }

    size_t sig_len = 0;
    if (!EVP_DigestSign(mdctx, nullptr, &sig_len, hash.data(), hash.size())) {
        EVP_MD_CTX_free(mdctx);
        util::logCall("WALLET", "ecDoSign()", false, "signSize() failed");
        return false;
    }

    signature.resize(sig_len);
    if (EVP_DigestSign(mdctx, signature.data(), &sig_len, hash.data(), hash.size())) {
        signature.resize(sig_len);
        EVP_MD_CTX_free(mdctx);
        util::logCall("WALLET", "ecDoSign()", true);
        return true;
    }

    EVP_MD_CTX_free(mdctx);
    util::logCall("WALLET", "ecDoSign()", false, "sign() failed");
    return false;
}



/* A function that takes a keypair, a message digest, and a signature and verifies it */
bool Wallet::ecDoVerify(const EVP_PKEY_ptr& pubKey, const std::vector<unsigned char> &hash, const std::vector<unsigned char> &signature){
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) return false;

    if (!EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha3_512(), nullptr, pubKey.get())) {
        util::logCall("WALLET", "ecDoVerify()", false, "init() failed");
        EVP_MD_CTX_free(mdctx);
        util::logCall("WALLET", "ecDoVerify()", false, "verify() failed");
        return false;
    }

    bool result = EVP_DigestVerify(mdctx, signature.data(), signature.size(), hash.data(), hash.size()) == 1;
    EVP_MD_CTX_free(mdctx);
    util::logCall("WALLET", "ecDoVerify()", true);
    return result;
}

EVP_PKEY_ptr Wallet::extract_public_key(){
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        util::logCall("WALLET", "extract_public_key()", false, "Error creating BIO");
        std::cerr << "Error creating BIO\n";
        return nullptr;
    }

    // Write the public key to the BIO
    if (PEM_write_bio_PUBKEY(bio, keyPair.get()) != 1) {
        util::logCall("WALLET", "extract_public_key()", false, "Error writing public key to BIO");
        std::cerr << "Error writing public key to BIO\n";
        BIO_free(bio);
        return nullptr;
    }

    // Read the public key from the BIO
    EVP_PKEY_ptr temp;
    temp.reset(PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr), EVP_PKEY_free);
    if (!temp) {
        util::logCall("WALLET", "extract_public_key()", false, "Error reading public key from BIO");
        std::cerr << "Error reading public key from BIO\n";
    }

    BIO_free(bio);
    util::logCall("WALLET", "extract_public_key()", true);
    return temp;
}

std::string Wallet::genAddress() const {
    // Get DER and create buffer
    unsigned char* temp = nullptr;  // Use temp for i2d_PUBKEY to advance
    int len = i2d_PUBKEY(pubKeyP.get(), &temp);

    /* 512 SHA_HASH*/
    std::string msg(reinterpret_cast<char*>(temp), len);
    std::vector<unsigned char> md;
    if (util::shaHash(msg, md)) {
        std::vector<unsigned char> ripe;
        if (util::ripemd(md, ripe)) {

            /* Base 58 Encode */
            const std::string output = util::base58_encode(ripe.data(), ripe.size());
            std::string addr = "Ox17" + output;

            /* Return Address */
            std::cout << "Wallet Address: " << addr << std::endl;
            util::logCall("WALLET", "genAddress()", true);
            return addr;
        }
        else {
            util::logCall("WALLET", "genAddress()", false, "RIPEMD computation failed!");
            std::cerr << "RIPEMD-320 computation failed!\n";
        }
    }
    else {
        util::logCall("WALLET", "genAddress()", false, "SHA3-512 computation failed!");
        std::cerr << "SHA3-512 computation failed!\n";
    }

    util::logCall("WALLET", "genAddress()", false, "RIPEMD failed!");
    return "";
}

utxout Wallet::outUTXO(double feee, const std::vector<std::string>& rwa, const std::vector<double>& amm, const std::vector<std::string> &delegates,
    const std::vector<std::string> &delegateID, const std::vector<std::tuple<std::string, std::string, float>> &votesQueue) {
    /* Structure UTXO */
    transactions utxo(address, rwa, amm, feee, locktimeUTXO, versionUTXO, delegates, delegateID, votesQueue);

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
        util::logCall("WALLET", "outUTXO()", false, "Amount Exceeds Balance");
        std::cout << "Amount Exceeds Balance!\n";
        return {};
    }

    /* Sign UTXO */
    utxout out;
    std::vector<unsigned char> hash;
    std::vector<unsigned char> sig;
    std::string msg = util::toString(utxo.serialize());

    if (util::shaHash(msg, hash)) {
        if (ecDoSign(hash, sig)) {
            /* Setup tx & Signed Message for mempool */
            size_t testsz = 0;
            unsigned char* testSer = utxo.serialize();
            std::memcpy(&testsz, testSer, sizeof(size_t));
            size_t ts_size = utxo.getSize();

            if (testsz == ts_size) {
                out.txSize = msg.size();
                out.shSize = sig.size();
                out.pkeySize = i2d_PUBKEY(pubKeyP.get(), nullptr);
                out.utxo = msg;
                out.utxoSignedHash = sig;
                out.pubkey = pubKeyP;
            }
            else {
                util::logCall("WALLET", "outUTXO()", false, "TX OUT SIZE INVALID");
                std::cout << "TX OUT SIZE INVALID\n";
                return {};
            }
        }
        else {
            util::logCall("WALLET", "outUTXO()", false, "Signature computation failed");
            std::cerr << "Signature computation failed!\n";
        }
    }
    else {
        util::logCall("WALLET", "outUTXO()", false, "Hash computation failed");
        std::cerr << "Hash computation failed!\n";
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
                std::vector<double> namm;
                double mamm = (utxoup - check);
                namm.push_back(mamm);
                transactions ttx(address, mra, namm, 0, locktimeUTXO, versionUTXO, del,
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

    util::logCall("WALLET", "outUTXO()", true);
    return out;
}

void Wallet::inUTXO(const transactions& txin) {
    UTXO.emplace_back(txin);
    setBalance();
    util::logCall("WALLET", "inUTXO()", true);
}

bool Wallet::verifyTx(const utxout& out) {
    /* Declare variables */
    transactions utxo = transactions::deserialize(util::toUnsignedChar(out.utxo));
    std::vector<unsigned char> hash;
    if (util::shaHash(out.utxo, hash)) {
        /* Verify Hash */
        if (ecDoVerify(out.pubkey, hash, out.utxoSignedHash)) {
            /* Verify UTXO amount > zero */
            double totalbal = 0.0;
            std::vector<double> tempBal = utxo.getAmmount();
            for (double amount : tempBal) {
                totalbal += amount;
            }
            if (totalbal <= 0) {
                util::logCall("WALLET", "verifyTx()", false, "UTXO amount is invalid (must be greater than 0)");
                std::cout << "UTXO amount is invalid (must be greater than 0)!\n";
                return false;
            }

            util::logCall("WALLET", "verifyTx()", true);
            return true;
        }
        else {
            util::logCall("WALLET", "verifyTx()", false, "INVALID SIGNATURE");
            std::cerr << "INVALID SIGNATURE\n";
            return false;
        }
    }
    else {
        util::logCall("WALLET", "verifyTx()", false, "Hash Failed");
        std::cerr << "Hash Failed\n";
        return false;
    }
}

void Wallet::listTxs() {
    for (auto& tx: UTXO) {
        tx.display();
    }
    util::logCall("WALLET", "listTxs()", true);
}

void Wallet::setBalance() {
    double amm = 0;
    size_t txsize = UTXO.size();
    for (int i = 0; i < txsize; i++) {
        amm += UTXO[i].totalAmm();
    }

    balance = amm;
    util::logCall("WALLET", "setBalance()", true);
}

double Wallet::getBalance() const {
    util::logCall("WALLET", "getBalance()", true);
    return balance;
}

std::string Wallet::getWalletAddr() const {
    util::logCall("WALLET", "getWalletAddr()", true);
    return address;
}

EVP_PKEY_ptr Wallet::getPubKey() const {
    util::logCall("WALLET", "getPubKey()", true);
    return pubKeyP;
}

unsigned short Wallet::getLockTime() const {
    util::logCall("WALLET", "getLockTime()", true);
    return locktimeUTXO;
}

void Wallet::setLockTime(unsigned short lk) {
    util::logCall("WALLET", "setLockTime()", true);
    locktimeUTXO = lk;
}

float Wallet::getVersion() const {
    util::logCall("WALLET", "getVersion()", true);
    return versionUTXO;
}

void Wallet::setVersion(float vs) {
    util::logCall("WALLET", "setVersion()", true);
    versionUTXO = vs;
}

unsigned char* Wallet::serialize_utxout(const utxout& obj) const {

    /* Calculate sizes */
    size_t tSize = 0;

    /* Calculate size and serialze public key */
    unsigned char *buf = nullptr;
    const int len = i2d_PUBKEY(obj.pubkey.get(), &buf);


    tSize += sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(int) + obj.utxo.size() + obj.utxoSignedHash.size() + len;

    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    /* serialize utxout */

    /* Serialize tSize itself */
    std::memcpy(buffer + offset, &tSize, sizeof(tSize));
    offset += sizeof(tSize);

    /* Serialize txSize itself */
    size_t utxoSize = obj.utxo.size();
    std::memcpy(buffer + offset, &utxoSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize shSize itself */
    size_t shSize = obj.utxoSignedHash.size();
    std::memcpy(buffer + offset, &shSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize key length itself */
    std::memcpy(buffer + offset, &len, sizeof(int));
    offset += sizeof(int);

    /* Serialize utxo itself */
    std::memcpy(buffer + offset, obj.utxo.data(), obj.utxo.size());
    offset += obj.utxo.size();

    /* Serialize signed hash itself */
    std::memcpy(buffer + offset, obj.utxoSignedHash.data(), obj.utxoSignedHash.size());
    offset += obj.utxoSignedHash.size();

    /* Serialize public key itself */
    std::memcpy(buffer + offset, buf, len);

    util::logCall("WALLET", "serialize_utxout()", true);
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

    /* Deserialize public key size */
    std::memcpy(&obj.pkeySize, buffer + offset, sizeof(int));
    offset += sizeof(int);

    /* Deserialize utxo */
    obj.utxo.assign(reinterpret_cast<const char*>(buffer + offset), obj.txSize);
    offset += obj.txSize;

    /* Deserialize utxoSignedHash */
    obj.utxoSignedHash.assign(buffer + offset, buffer + offset + obj.shSize);
    offset += obj.shSize;

    /* Deserialize Public Key */
    const unsigned char* pk = buffer + offset;
    EVP_PKEY* raw_key = d2i_PUBKEY(nullptr, &pk, obj.pkeySize);
    obj.pubkey = EVP_PKEY_ptr(raw_key, EVP_PKEY_Deleter());

    util::logCall("WALLET", "deserialize_utxout()", true);
    return obj;
}