/*-- transactions.cpp------------------------------------------------------------
   This file implements transaction member functions.
---------------------------------------------------------------------------*/
#include "transactions.h"

transactions::transactions(std::string sa, std::vector<std::string> ra, std::vector<double> amm, double fe, unsigned short lk,
    float v, std::vector<std::string> delegate, std::vector<std::string> delegatesID, std::vector<std::tuple<std::string, std::string, float>> votes,
    unsigned long long timestamp, std::string tid)
    : timestamp(timestamp),
    txid(tid),
    sendAddr(sa),
    recieveAddr(ra),
    ammount(std::move(amm)),
    fee(fe),
    locktime(lk),
    version(v),
    delegates(std::move(delegate)),
    delegateID(std::move(delegatesID)),
    votesQueue(std::move(votes))
{
    util::logCall("TRANSACTIONS", "transactions()", true);
}

/* Copy */
transactions::transactions(const transactions& copy)
    :timestamp(copy.timestamp),
    txid(copy.txid),
    sendAddr(copy.sendAddr),
    recieveAddr(copy.recieveAddr),
    ammount(copy.ammount),
    fee(copy.fee),
    locktime(copy.locktime),
    version(copy.version),
    delegates(copy.delegates),
    delegateID(copy.delegateID),
    votesQueue(copy.votesQueue)
{
}


transactions::~transactions() {
    util::logCall("TRANSACTIONS", "~transactions()", true);
};

unsigned long long transactions::getTimeStamp() const {
    return timestamp;
}

unsigned long long transactions::setTimeStamp(){
    return ut.TimeStamp();
}

const std::string transactions::getTxid() const {
    return txid;
}

std::string transactions::setTxid(){
    /* Create a random number generator */
    std::random_device rd; // Seed for random number generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<unsigned int> distrib(0, 4294967295); // Define the range

    /* Generate a random number */
    const unsigned int random_number = distrib(gen);
    const unsigned int random_numberA = distrib(gen);
    std::string random, randomA;

    /* Achieve Uniformity For First Random Number */
    if (random_number < 10) {
        random = "000000000" + util::toString(random_number);
    }
    else if (random_number < 100) {
        random = "00000000" + util::toString(random_number);
    }
    else if (random_number < 1000) {
        random = "0000000" + util::toString(random_number);
    }
    else if (random_number < 10000) {
        random = "000000" + util::toString(random_number);
    }
    else if (random_number < 100000) {
        random = "00000" + util::toString(random_number);
    }
    else if (random_number < 1000000) {
        random = "0000" + util::toString(random_number);
    }
    else if (random_number < 10000000) {
        random = "000" + util::toString(random_number);
    }
    else if (random_number < 100000000) {
        random = "00" + util::toString(random_number);
    }
    else if (random_number < 1000000000) {
        random = "0" + util::toString(random_number);
    }
    else {
        random = util::toString(random_number);
    }

    /* Achieve Uniformity For Second Random Number */
    if (random_numberA < 10) {
        randomA = "000000000" + util::toString(random_number);
    }
    else if (random_number < 100) {
        randomA = "00000000" + util::toString(random_number);
    }
    else if (random_number < 1000) {
        randomA = "0000000" + util::toString(random_number);
    }
    else if (random_number < 10000) {
        randomA = "000000" + util::toString(random_number);
    }
    else if (random_number < 100000) {
        randomA = "00000" + util::toString(random_number);
    }
    else if (random_number < 1000000) {
        randomA = "0000" + util::toString(random_number);
    }
    else if (random_number < 10000000) {
        randomA = "000" + util::toString(random_number);
    }
    else if (random_number < 100000000) {
        randomA = "00" + util::toString(random_number);
    }
    else if (random_number < 1000000000) {
        randomA = "0" + util::toString(random_number);
    }
    else {
        randomA = util::toString(random_number);
    }

    random += randomA;
    std::string tx_id = ("0X0017" + random + util::toString(util::TimeStamp()));
    util::logCall("TRANSACTIONS", "setTxid()", true);
    return tx_id;
}

std::string transactions::getSendAddr() const {
    return sendAddr;
}

std::vector<std::string> transactions::getRecieveAddr() const {
    return recieveAddr;
}

std::vector<double> transactions::getAmmount() const {
    return ammount;
}

double transactions::getFee() const {
    return fee;
}

unsigned short transactions::getLockTime() const {
    return locktime;
}

float transactions::getVersion() const {
    return version;
}

std::vector<std::string> transactions::getDelegates() const {
    return delegates;
}

std::vector<std::string> transactions::getDelegatesID() const {
    return delegateID;
}

std::vector<std::tuple<std::string, std::string, float>> transactions::getVotes() const {
    return votesQueue;
}

bool transactions::inputsValid() const {
    if (timestamp == 0 || locktime == 0 || version == 0 || txid.empty() || sendAddr.empty()) {
        util::logCall("TRANSACTIONS", "inputsValid()", false, "Inputs Invalid");
        return false;
    }

    util::logCall("TRANSACTIONS", "inputsValid()", true);
    return true; // Return only after checking all values
}

bool transactions::outputsValid() const {
    if (recieveAddr.empty()) {
        util::logCall("TRANSACTIONS", "outputsValid()", false, "RECARRD EMPTY");
        return false;
    }

    for (const double i : ammount) {
        if (i == 0) {
            util::logCall("TRANSACTIONS", "outputsValid()", false, "AMM EMPTY");
            return false;
        }
    }

    util::logCall("TRANSACTIONS", "outputsValid()", true);
    return true; // Return only after checking all values
}

double transactions::totalAmm() const {
    double amm = 0;
    size_t ammsize = ammount.size();

    for (int i = 0; i < ammsize; i++) {
        amm += ammount[i];
    }

    util::logCall("TRANSACTIONS", "totalAmm()", true);
    return amm;
}

void transactions::display() {
    std::cout << "===================================\n";
    std::cout << "         TRANSACTION DETAILS       \n";
    std::cout << "===================================\n";

    std::cout << "\n**General Info**\n";
    std::cout << "Transaction ID   : " << txid << "\n";
    std::cout << "Timestamp        : " << timestamp << "\n";
    std::cout << "Version         : " << version << "\n";
    std::cout << "Locktime        : " << locktime << "\n";
    std::cout << "Transaction Fee  : " << fee << " Dash\n";

    std::cout << "\n**Sender Information**\n";
    std::cout << "Sender Address   : " << sendAddr << "\n";

    std::cout << "\n**Recipients**\n";
    for (size_t i = 0; i < recieveAddr.size(); ++i) {
        std::cout << "Recipient " << (i + 1) << " : " << recieveAddr[i]
            << " | Amount: " << ammount[i] << " Dash\n";
    }

    std::cout << "\n**Delegates & Voting**\n";
    std::cout << "Delegates        : ";
    for (const auto& d : delegates) std::cout << d << " ";
    std::cout << "\n";

    std::cout << "Delegate IDs     : ";
    for (const auto& id : delegateID) std::cout << id << " ";
    std::cout << "\n";

    std::cout << "\n**Votes Queue**\n";
    for (const auto& vote : votesQueue) {
        std::cout << "Voter: " << std::get<0>(vote)
            << " | Delegate: " << std::get<1>(vote)
            << " | Weight: " << std::get<2>(vote) << "\n";
    }

    std::cout << "===================================\n";
}

/* Serialize method */
unsigned char* transactions::serialize() const {
    /*
        There Will be 6 uint32_t, 3 short & 1 size_t Variables at the start of every Buffer
        This Will Account For The Size Of:
            uint32_t (In Order)             short (In Order)            size_t (In Order)
            1. sendAddr                                                    tSize
            2. txid
            3. ammount                      short numAmm;
            4. sendPkey
            5. recieveAddr                  short recAddAmm
            6. recievePkeys                 short prkAmm;

            NOTE: txid will be serialized as a string for efficiency purposes, reflected in deserialize
    */

    /* Variables */
    // Sizes of: Locktime, Version, Fee & Timestamp (In That Order)
    size_t tSize = 0;

    /* Variable Vars */
    size_t numAmm = 0, recAddAmm = 0, delAmm = 0, delIDAmm = 0, vQueAmm = 0;
    size_t sendSize = 0, txidSize = 0, ammSize = 0, recAddSize = 0, delSize = 0, delIDSize = 0, vQueSize = 0;

    sendSize = sendAddr.size() * sizeof(char); //Calculate size of sendAddr String

    /* Calculate txid Size */
    txidSize = txid.size() * sizeof(char);

    numAmm = ammount.size();
    ammSize = ammount.size() * sizeof(double); // Calculate size of amount vector

    /* Calculate recieveAddr Size */
    recAddAmm = recieveAddr.size();
    for (const auto& addr : recieveAddr) {
        recAddSize += addr.size() + 1; // Size for length + actual string content
    }

    delAmm = delegates.size();
    for (const auto& d: delegates) {
        delSize +=  d.size() + 1;
    }

    delIDAmm = delegateID.size();
    for (const auto& id: delegateID) {
        delIDSize += id.size() + 1;
    }

    vQueAmm = votesQueue.size();
    for (const auto& vote: votesQueue) {
        vQueSize += std::get<0>(vote).size() + 1;
        vQueSize += std::get<1>(vote).size() + 1;
        vQueSize += sizeof(float);
    }

    /* Create Buffer */
    tSize = tSize + sizeof(unsigned long long) + sizeof(double) + sizeof(unsigned short) + sizeof(float);
    tSize = tSize + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t);
    tSize = tSize + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t);
    tSize = tSize + sizeof(size_t) + sizeof(size_t) + sizeof(size_t);
    tSize = tSize + sendSize + txidSize + ammSize + recAddSize + delSize + delIDSize + vQueSize;

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

    /* Serialize delAmm itself */
    std::memcpy(buffer + offset, &delAmm, sizeof(delAmm));
    offset += sizeof(delAmm);

    /* Serialize delIDAmm itself */
    std::memcpy(buffer + offset, &delIDAmm, sizeof(delIDAmm));
    offset += sizeof(delIDAmm);

    /* Serialize vQueAmm itself */
    std::memcpy(buffer + offset, &vQueAmm, sizeof(vQueAmm));
    offset += sizeof(vQueAmm);

    /* Serialize sendSize itself */
    std::memcpy(buffer + offset, &sendSize, sizeof(sendSize));
    offset += sizeof(sendSize);

    /* Serialize txidSize itself */
    std::memcpy(buffer + offset, &txidSize, sizeof(txidSize));
    offset += sizeof(txidSize);

    /* Serialize ammSize itself */
    std::memcpy(buffer + offset, &ammSize, sizeof(ammSize));
    offset += sizeof(ammSize);


    /* Serialize recAddSize itself */
    std::memcpy(buffer + offset, &recAddSize, sizeof(recAddSize));
    offset += sizeof(recAddSize);


    /* Serialize delSize itself */
    std::memcpy(buffer + offset, &delSize, sizeof(delSize));
    offset += sizeof(delSize);

    /* Serialize delIDSize itself */
    std::memcpy(buffer + offset, &delIDSize, sizeof(delIDSize));
    offset += sizeof(delIDSize);

    /* Serialize vQueSize itself */
    std::memcpy(buffer + offset, &vQueSize, sizeof(vQueSize));
    offset += sizeof(vQueSize);

    /* Serialize lock-time (2 bytes) */
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
    std::memcpy(buffer + offset, txid.data(), txidSize);
    offset += txidSize;

    /* Serialize recieveAddr (variable) */
    unsigned char* tbuff = new unsigned char[recAddSize];
    size_t toff = 0;
    for (const auto& str : recieveAddr) {
        std::memcpy(tbuff + toff, str.c_str(), str.size() + 1); // +1 to copy null terminator
        toff += str.size() + 1;  // Update offset for the next string
    }
    std::memcpy(buffer + offset, tbuff, toff);
    offset += toff;

    /* Serialize delegates (variable) */
    unsigned char* tbuff1 = new unsigned char[delSize];
    size_t toff1 = 0;
    for (const auto& s : delegates) {
        std::memcpy(tbuff1 + toff1, s.c_str(), s.size() + 1);
        toff1 += s.size() + 1;
    }
    std::memcpy(buffer + offset, tbuff1, toff1);
    offset += toff1;

    /* Serialize delegateID (variable) */
    unsigned char* tbuff2 = new unsigned char[delIDSize];
    size_t toff2 = 0;
    for (const auto& s : delegateID) {
        std::memcpy(tbuff2 + toff2, s.c_str(), s.size() + 1);
        toff2 += s.size() + 1;
    }
    std::memcpy(buffer + offset, tbuff2, toff2);
    offset += toff2;

    /* Serialize votesQueue (variable) */
    for (const auto& tup : votesQueue) {
        const std::string& s1 = std::get<0>(tup);
        std::memcpy(buffer + offset, s1.c_str(), s1.size() + 1);
        offset += s1.size() + 1;
        const std::string& s2 = std::get<1>(tup);
        std::memcpy(buffer + offset, s2.c_str(), s2.size() + 1);
        offset += s2.size() + 1;
        float flt = std::get<2>(tup);
        std::memcpy(buffer + offset, &flt, sizeof(flt));
        offset += sizeof(flt);
    }

    util::logCall("TRANSACTIONS", "serialize()", true);
    return buffer; // Return the serialized buffer
}

/* Deserialize method */
transactions transactions::deserialize(const unsigned char* data) {
    size_t offset = 0;

    /* Deserialize tSize */
    size_t tSize;
    std::memcpy(&tSize, data + offset, sizeof(tSize));
    offset += sizeof(tSize);

    /* Deserialize numAmm */
    size_t numAmm;
    std::memcpy(&numAmm, data + offset, sizeof(numAmm));
    offset += sizeof(numAmm);

    /* Deserialize recAddAmm */
    size_t recAddAmm;
    std::memcpy(&recAddAmm, data + offset, sizeof(recAddAmm));
    offset += sizeof(recAddAmm);

    /* Deserialize delAmm */
    size_t delAmm;
    std::memcpy(&delAmm, data + offset, sizeof(delAmm));
    offset += sizeof(delAmm);

    /* Deserialize delIDAmm */
    size_t delIDAmm;
    std::memcpy(&delIDAmm, data + offset, sizeof(delIDAmm));
    offset += sizeof(delIDAmm);

    /* Deserialize vQueAmm */
    size_t vQueAmm;
    std::memcpy(&vQueAmm, data + offset, sizeof(vQueAmm));
    offset += sizeof(vQueAmm);

    /* Deserialize sendSize */
    size_t sendSize;
    std::memcpy(&sendSize, data + offset, sizeof(sendSize));
    offset += sizeof(sendSize);

    /* Deserialize txidSize */
    size_t txidSize;
    std::memcpy(&txidSize, data + offset, sizeof(txidSize));
    offset += sizeof(txidSize);

    /* Deserialize ammSize */
    size_t ammSize;
    std::memcpy(&ammSize, data + offset, sizeof(ammSize));
    offset += sizeof(ammSize);

    /* Deserialize recAddSize */
    size_t recAddSize;
    std::memcpy(&recAddSize, data + offset, sizeof(recAddSize));
    offset += sizeof(recAddSize);

    /* Deserialize delSize */
    size_t delSize;
    std::memcpy(&delSize, data + offset, sizeof(delSize));
    offset += sizeof(delSize);

    /* Deserialize delIDSize */
    size_t delIDSize;
    std::memcpy(&delIDSize, data + offset, sizeof(delIDSize));
    offset += sizeof(delIDSize);

    /* Deserialize vQueSize */
    size_t vQueSize;
    std::memcpy(&vQueSize, data + offset, sizeof(vQueSize));
    offset += sizeof(vQueSize);

    /* Deserialize locktime */
    unsigned short* lk = new unsigned short[1];
    size_t lk_size = sizeof(unsigned short);
    std::memcpy(lk, data + offset, lk_size);
    offset += lk_size;

    /* Deserialize version */
    float* vs = new float[1];
    size_t vsSize = sizeof(float);
    std::memcpy(vs, data + offset, vsSize);
    offset += vsSize;

    /* Deserialize fee */
    double* fe = new double[1];
    size_t feSize = sizeof(double);
    std::memcpy(fe, data + offset, feSize);
    offset += feSize;

    /* Deserialize timestamp */
    unsigned long long* ts = new unsigned long long[1];
    size_t tsSize = sizeof(unsigned long long);
    std::memcpy(ts, data + offset, tsSize);
    offset += tsSize;

    /* Deserialize sendAddr */
    std::string sa;
    sa.resize(sendSize);
    std::memcpy(sa.data(), data + offset, sendSize);
    offset += sendSize;

    /* Deserialize ammount */
    std::vector<double> am;
    am.resize(numAmm);
    std::memcpy(am.data(), data + offset, ammSize);
    offset += ammSize;

    /* Deserialize txid */
    std::string temStid;
    temStid.resize(txidSize);
    std::memcpy(temStid.data(), data + offset, txidSize);
    offset += txidSize;

    /* Deserialize recieveAddr */
    std::vector<std::string> tempra;
    tempra.resize(recAddAmm);
    for (auto& addr : tempra) {
        size_t len = std::strlen(reinterpret_cast<const char*>(data + offset)) + 1;
        addr.assign(reinterpret_cast<const char*>(data + offset), len - 1);
        offset += len;
    }

    /* Deserialize delegates */
    std::vector<std::string> delegates;
    delegates.resize(delAmm);
    for (auto& d : delegates) {
        size_t len = std::strlen(reinterpret_cast<const char*>(data + offset)) + 1;
        d.assign(reinterpret_cast<const char*>(data + offset), len - 1);
        offset += len;
    }

    /* Deserialize delegateID */
    std::vector<std::string> delegateID;
    delegateID.resize(delAmm);
    for (auto& id : delegateID) {
        size_t len = std::strlen(reinterpret_cast<const char*>(data + offset)) + 1;
        id.assign(reinterpret_cast<const char*>(data + offset), len - 1);
        offset += len;
    }

    /* Deserialize votesQueue */
    std::vector<std::tuple<std::string, std::string, float>> votesQueue;
    votesQueue.resize(vQueAmm);
    for (size_t i = 0; i < vQueAmm; ++i) {
        const char* strPtr1 = reinterpret_cast<const char*>(data + offset);
        size_t len1 = std::strlen(strPtr1) + 1;
        std::string voteStr1(strPtr1, len1 - 1);
        offset += len1;

        const char* strPtr2 = reinterpret_cast<const char*>(data + offset);
        size_t len2 = std::strlen(strPtr2) + 1;
        std::string voteStr2(strPtr2, len2 - 1);
        offset += len2;

        float voteFloat;
        std::memcpy(&voteFloat, data + offset, sizeof(float));
        offset += sizeof(float);

        votesQueue[i] = std::make_tuple(voteStr1, voteStr2, voteFloat);
    }

    transactions tx(sa, tempra, am, *fe, *lk, *vs, delegates, delegateID, votesQueue, *ts, temStid);

    /* Cleanup */
    delete[] fe;
    delete[] lk;
    delete[] vs;
    delete[] ts;

    util::logCall("TRANSACTIONS", "deserialize()", true);
    return tx;
}