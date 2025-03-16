/*-- transactions.cpp------------------------------------------------------------
   This file implements transaction member functions.
---------------------------------------------------------------------------*/
#include "transactions.h"

transactions::transactions() {
    timestamp = 0;
    txid = nullptr;
    sendPkey = nullptr;  // Initialize sendPkey to nullptr
    locktime = 7;
    version = 1.0;
}

transactions::~transactions() {
    /* Free EVP_PKEY pointers */
    if (sendPkey != nullptr) {
        EVP_PKEY_free(sendPkey);
    }
    for (auto& pkey : recievePkeys) {
        if (pkey != nullptr) {
            EVP_PKEY_free(pkey);
        }
    }
}

unsigned long long transactions::getTimeStamp() const {
    return timestamp;
}

void transactions::setTimeStamp(unsigned long long ts) {
    timestamp = ts;
}

unsigned char* transactions::getTxid() const {
    return txid;
}

void transactions::setTxid(unsigned char* tx) {
    txid = tx;
}

std::string transactions::getSendAddr() const {
    return sendAddr;
}

void transactions::setSendAddr(std::string sa) {
    sendAddr = sa;
}

std::vector<std::string> transactions::getRecieveAddr() const {
    return recieveAddr;
}

void transactions::setRecieveAddr(std::vector<std::string> addy) {
    recieveAddr = addy;
}

std::vector<double> transactions::getAmmount() const {
    return ammount;
}

void transactions::setAmmount(std::vector<double> amm) {
    ammount = amm;
}

EVP_PKEY* transactions::getSendPkey() const {
    return sendPkey;
}

void transactions::setSendPkey(EVP_PKEY* sk) {
    sendPkey = sk;
}

std::vector<EVP_PKEY*> transactions::getRecievePkeys() const {
    return recievePkeys;
}

void transactions::setRecievePkeys(std::vector<EVP_PKEY*> rpk) {
    recievePkeys = rpk;
}

double transactions::getFee() const {
    return fee;
}

void transactions::setFee(double fe) {
    fee = fe;
}

unsigned short transactions::getLockTime() const {
    return locktime;
}

void transactions::setLockTime(unsigned short lt) {
    locktime = lt;
}

float transactions::getVersion() const {
    return version;
}

void transactions::setVersion(float v) {
    version = v;
}

bool transactions::inputsValid() const {
    if (timestamp != NULL) {
        if (sendPkey != nullptr) {
            if (locktime != NULL) {
                if (version != NULL) {
                    int ammSize = ammount.size();
                    for (int i = 0; i < ammSize; i++) {
                        if (ammount[i] == NULL) {
                            return false;
                        }
                        else {
                            return true;
                        }
                    }
                }
                else {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

bool transactions::outputsValid() const {
    if (!recievePkeys.empty()) {
        if (!recieveAddr.empty()) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

double transactions::totalAmm() const {
    double amm = 0;
    short ammsize = ammount.size();

    for (int i = 0; i < ammsize; i++) {
        amm += ammount[i];
    }

    return amm;
}

/* Serialize method */
unsigned char* transactions::serialize() const {
    /*
        There Will be 6 uint32_t, 3 short & 1 size_t Variables at the start of every Buffer
        This Will Account For The Sized Of: 
            uint32_t (In Order)             short (In Order)            size_t (In Order)
            1. sendAddr                                                    tSize
            2. txid
            3. ammount                      short numAmm;
            4. sendPkey
            5. recieveAddr                  short recAddAmm
            6. recievePkeys                 short prkAmm;
    */

    /* Variables */
    // Sizes of: Locktime, Version, Fee & Timestamp (In That Order)
    size_t tSize = 0;
    tSize = sizeof(unsigned short) + sizeof(float) + sizeof(double) + sizeof(unsigned long long);

    /* Variable Vars */
    short numAmm = 0, recAddAmm = 0, prkAmm = 0;
    uint32_t sendSize = 0, txidSize = 0, ammSize = 0, spkSize = 0, recAddSize = 0, rpkSize = 0;

    sendSize = sendAddr.size() * sizeof(char); //Calculate size of sendAddr String

    /* Calculate txid Size */
    size_t pointerSize = sizeof(txid);
    size_t dataSize = sizeof(unsigned long long);
    txidSize = pointerSize + dataSize;

    numAmm = ammount.size();
    ammSize = ammount.size() * sizeof(double); // Calculate size of ammount vector

    /* Calculate sendPkey Size */
    int len = i2d_PUBKEY(sendPkey, nullptr);
    spkSize = len;

    /* Calculate recieveAddr Size */
    recAddAmm = recieveAddr.size();
    for (const auto& addr : recieveAddr) {
        recAddSize += addr.size() + 1; // Size for length + actual string content
    }

    /* Calculate recievePkeys size */
    for (const auto& pkey : recievePkeys) {
        int lenn = i2d_PUBKEY(pkey, nullptr);
        recAddSize += lenn;
    }

    /* Create Buffer */
    tSize = tSize + sendSize + txidSize + ammSize + spkSize + recAddSize + rpkSize;
    tSize = tSize + sizeof(short) + sizeof(short) + sizeof(short) + sizeof(size_t);
    tSize = tSize + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

    /* Allocate memory for buffer */
    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    /* Serialize tSize itself */
    std::memcpy(buffer + offset, &tSize, sizeof(tSize));
    offset += sizeof(tSize);

    /* Serialize numAmm itself */
    std::memcpy(buffer + offset, &numAmm, sizeof(numAmm));
    offset += sizeof(numAmm);

    /* Serialize recAddAmm itself */
    std::memcpy(buffer + offset, &recAddAmm, sizeof(recAddAmm));
    offset += sizeof(recAddAmm);

    /* Serialize prkAmm itself */
    std::memcpy(buffer + offset, &prkAmm, sizeof(prkAmm));
    offset += sizeof(prkAmm);

    /* Serialize sendSize itself */
    std::memcpy(buffer + offset, &sendSize, sizeof(sendSize));
    offset += sizeof(sendSize);

    /* Serialize txidSize itself */
    std::memcpy(buffer + offset, &txidSize, sizeof(txidSize));
    offset += sizeof(txidSize);

    /* Serialize ammSize itself */
    std::memcpy(buffer + offset, &ammSize, sizeof(ammSize));
    offset += sizeof(ammSize);

    /* Serialize spkSize itself */
    std::memcpy(buffer + offset, &spkSize, sizeof(spkSize));
    offset += sizeof(spkSize);

    /* Serialize recAddSize itself */
    std::memcpy(buffer + offset, &recAddSize, sizeof(recAddSize));
    offset += sizeof(recAddSize);

    /* Serialize rpkSize itself */
    std::memcpy(buffer + offset, &rpkSize, sizeof(rpkSize));
    offset += sizeof(rpkSize);

    /* Serialize locktime (2 bytes) */
    std::memcpy(buffer + offset, &locktime, sizeof(locktime));
    offset += sizeof(locktime);

    /* Serialize version (4 bytes) */
    std::memcpy(buffer + offset, &version, sizeof(version));
    offset += sizeof(version);

    /* Serialize fee (8 bytes) */
    std::memcpy(buffer + offset, &fee, sizeof(fee));
    offset += sizeof(fee);

    /* Serialize timestamp (8 bytes) */
    std::memcpy(buffer + offset, &timestamp, sizeof(timestamp));
    offset += sizeof(timestamp);

    /* Serialize sendAddr (variable) */
    std::memcpy(buffer + offset, sendAddr.data(), sendSize);
    offset += sendSize;

    /* Serialize ammount (variable) */
    std::memcpy(buffer + offset, ammount.data(), ammSize);
    offset += ammSize;

    /* Serialize txid (variable) */
    std::memcpy(buffer + offset, txid, txidSize);
    offset += txidSize;

    /* Serialize sendPkey using OpenSSL */
    int lennn = i2d_PUBKEY(sendPkey, nullptr);
    std::vector<unsigned char> keyBytes(lennn);
    unsigned char* temp = keyBytes.data();
    i2d_PUBKEY(sendPkey, &temp);
    std::memcpy(buffer + offset, keyBytes.data(), keyBytes.size());
    offset += keyBytes.size();

    /* Serialize recieveAddr (variable) */
    unsigned char* tbuff = new unsigned char[recAddSize];
    size_t toff = 0;
    for (const auto& str : recieveAddr) {
        std::memcpy(tbuff + toff, str.c_str(), str.size() + 1); // +1 to copy null terminator
        toff += str.size() + 1;  // Update offset for the next string
    }
    std::memcpy(buffer + offset, tbuff, toff);
    offset += toff;

    /* Serialize recievePkeys */
    for (const auto& pkey : recievePkeys) {
        int llen = i2d_PUBKEY(pkey, nullptr);
        std::vector<unsigned char> pkeyBytes(llen);
        unsigned char* temp = pkeyBytes.data();
        i2d_PUBKEY(pkey, &temp);
        std::memcpy(buffer + offset, pkeyBytes.data(), pkeyBytes.size()); // Copy the serialized key
        offset += pkeyBytes.size(); // Move the offset forward
    }

    return buffer; // Return the serialized buffer
}

/* Deserialize method */
transactions transactions::deserialize(const unsigned char* data) {
    transactions tx;
    size_t offset = 0;

    /* Deserialize tSize */
    size_t tSize;
    std::memcpy(&tSize, data + offset, sizeof(tSize));
    offset += sizeof(tSize);

    /* Deserialize numAmm */
    short numAmm;
    std::memcpy(&numAmm, data + offset, sizeof(numAmm));
    offset += sizeof(numAmm);

    /* Deserialize recAddAmm */
    short recAddAmm;
    std::memcpy(&recAddAmm, data + offset, sizeof(recAddAmm));
    offset += sizeof(recAddAmm);

    /* Deserialize prkAmm */
    short prkAmm;
    std::memcpy(&prkAmm, data + offset, sizeof(prkAmm));
    offset += sizeof(prkAmm);

    /* Deserialize sendSize */
    uint32_t sendSize;
    std::memcpy(&sendSize, data + offset, sizeof(sendSize));
    offset += sizeof(sendSize);

    /* Deserialize txidSize */
    uint32_t txidSize;
    std::memcpy(&txidSize, data + offset, sizeof(txidSize));
    offset += sizeof(txidSize);

    /* Deserialize ammSize */
    uint32_t ammSize;
    std::memcpy(&ammSize, data + offset, sizeof(ammSize));
    offset += sizeof(ammSize);

    /* Deserialize spkSize */
    uint32_t spkSize;
    std::memcpy(&spkSize, data + offset, sizeof(spkSize));
    offset += sizeof(spkSize);

    /* Deserialize recAddSize */
    uint32_t recAddSize;
    std::memcpy(&recAddSize, data + offset, sizeof(recAddSize));
    offset += sizeof(recAddSize);

    /* Deserialize rpkSize */
    uint32_t rpkSize;
    std::memcpy(&rpkSize, data + offset, sizeof(rpkSize));
    offset += sizeof(rpkSize);

    /* Deserialize locktime */
    std::memcpy(&tx.locktime, data + offset, sizeof(tx.locktime));
    offset += sizeof(tx.locktime);

    /* Deserialize version */
    std::memcpy(&tx.version, data + offset, sizeof(tx.version));
    offset += sizeof(tx.version);

    /* Deserialize fee */
    std::memcpy(&tx.fee, data + offset, sizeof(tx.fee));
    offset += sizeof(tx.fee);

    /* Deserialize timestamp */
    std::memcpy(&tx.timestamp, data + offset, sizeof(tx.timestamp));
    offset += sizeof(tx.timestamp);

    /* Deserialize sendAddr */
    tx.sendAddr.resize(sendSize);
    std::memcpy(tx.sendAddr.data(), data + offset, sendSize);
    offset += sendSize;

    /* Deserialize ammount */
    tx.ammount.resize(numAmm);
    std::memcpy(tx.ammount.data(), data + offset, ammSize);
    offset += ammSize;

    /* Deserialize txid */
    std::memcpy(tx.txid, data + offset, txidSize);
    offset += txidSize;

    /* Deserialize sendPkey using OpenSSL */
    const unsigned char* temp = data + offset;
    tx.sendPkey = d2i_PUBKEY(nullptr, &temp, spkSize);
    offset += spkSize;

    /* Deserialize recieveAddr */
    tx.recieveAddr.resize(recAddAmm);
    for (auto& addr : tx.recieveAddr) {
        size_t len = std::strlen(reinterpret_cast<const char*>(data + offset)) + 1;
        addr.assign(reinterpret_cast<const char*>(data + offset), len - 1);
        offset += len;
    }

    /* Deserialize recievePkeys */
    tx.recievePkeys.resize(prkAmm);
    for (auto& pkey : tx.recievePkeys) {
        const unsigned char* temp = data + offset;
        pkey = d2i_PUBKEY(nullptr, &temp, rpkSize);
        offset += rpkSize;
    }

    return tx;
}
