//
// Created by king dash on 5/23/25.
//

#include "Consensus.h"

Consensus::Consensus()
{
    timestamp = util::TimeStamp();
    lastUpd = timestamp;
    votingPeriod = 0;
    windowPeriod = 240;
    minBalance = 0;
    decayFactor = .90;
    maxDelegates = 10;
}

bool Consensus::delIDExist(const std::string &ID) const {
    return std::ranges::find(delegates, ID) != delegates.end();
}

std::string Consensus::genDelegateID(){
    std::string delID;
    do {
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
        delID = ("D" + random + util::toString(util::TimeStamp()));

    }while (delIDExist(delID));

    delegateID.push_back(delID);
    util::logCall("CONSENSUS", "genDelegateID()", true);
    return delID;
}

std::vector<std::string>  Consensus::getDelegates() {
    std::lock_guard<std::mutex> lock(delegatesMutex);
    return delegates;
}

void Consensus::setDelegates(const std::vector<std::string>& dels) {
    std::lock_guard<std::mutex> lock(delegatesMutex);
    delegates = dels;
}

std::vector<std::string> Consensus::getDelegateIDs() {
    return delegateID;
}

void Consensus::setDelegateIDs(const std::vector<std::string> &delIDs) {
    std::lock_guard<std::mutex> lock(delegatesMutex);
    for (auto& id : delIDs) {
        if (delIDExist(id)) {
            continue;
        }
        else {
            delegateID.push_back(id);
        }
    }
}

void Consensus::addDelegateID(const std::string& delegate_id) {
    delegateID.push_back(delegate_id);
}

std::vector<std::tuple<std::string, std::string, float>> Consensus::getVotesQueue() {
    return votesQueue;
}

std::tuple<bool, std::string> Consensus::requestDelegate(const double balance) {
    std::lock_guard<std::mutex> lock(delegatesMutex);
    if (balance >= minBalance) {
        const std::string id = genDelegateID();
        delegates.push_back(id);
        util::logCall("CONSENSUS", "requestDelegate()", true);
        return {true, id};
    }
    else {
        util::logCall("CONSENSUS", "requestDelegate()", false, "Already Confirmed Delegate || Balance Too Low");
        std::cout << "Peer Already Confirmed Delegate || Balance Too Low!\n";
        return {false, ""};
    }
}

void Consensus::updatedVotes(const std::vector<std::tuple<std::string, std::string, float>>& votes) {
    for (auto& vote : votes) {
        votesQueue.emplace_back(vote);
    }
    util::logCall("CONSENSUS", "updatedVotes()", true);
}

void Consensus::setTimestamp(const unsigned long long ts) {
    timestamp = ts;
}
unsigned long long Consensus::getTimestamp() const {
    return timestamp;
}

void Consensus::setLastUpd(const unsigned long long lu) {
    lastUpd = lu;
}
unsigned long long Consensus::getLastUpd() const {
    return lastUpd;
}

unsigned long Consensus::getVotingPeriod() const {
    return votingPeriod;
}
void Consensus::setVotingPeriod(const unsigned long vp) {
    votingPeriod = vp;
}

unsigned long Consensus::getWindowPeriod() const {
    return windowPeriod;
}

void Consensus::setWindowPeriod(const unsigned long wp) {
    windowPeriod = wp;
}

unsigned long Consensus::getMaxDelegates() const {
    return maxDelegates;
}

void Consensus::setMaxDelegates(const unsigned long md) {
    maxDelegates = md;
}

float Consensus::getDecayFactor() const {
    return decayFactor;
}

void Consensus::setDecayFactor(const float df) {
    decayFactor = df;
}

float Consensus::getMinBalance() const {
    return minBalance;
}

void Consensus::setMinBalance(const float mb) {
    minBalance = mb;
}

std::string Consensus::getCurrentDelegate() {
    std::lock_guard<std::mutex> lock(delegatesMutex);
    if (delegates.empty()) {
        std::string error = "error";
        return error;
    }
    std::string currentDelegate = std::move(delegates.front());
    delegates.erase(delegates.begin());
    return currentDelegate;
}

void Consensus::updateDelegates() {
    if (const unsigned long long timeNow = util::TimeStamp(); timeNow - lastUpd >= votingPeriod) {
        std::lock_guard<std::mutex> lock(delegatesMutex);

        /* Process Vote Decay */
        std::map<std::pair<std::string, std::string>, std::vector<float>> votesContain; // temp container

        /* Populate Temp Container <Map> 7 track Occurrences */
        std::map<std::pair<std::string, std::string>, size_t> first_Occur;
        for (size_t i = 0; i < votesQueue.size(); i++) {
            auto[a, b, vote] = votesQueue[i];
            votesContain[{a, b}].emplace_back(vote);

            /* Track First Occurrence */
            if (!first_Occur.contains({a, b})) {
                first_Occur[{a, b}] = i;
            }
        }

        /* Vote Decay */
        for (const auto& [key, value] : votesContain) {
            if (value.size() > 1) {
                votesQueue.erase(votesQueue.begin() + first_Occur[key]);
            }
            else {
                auto& entry = votesQueue[first_Occur[key]];
                std::get<2>(entry) *= decayFactor;
            }
        }

        /* Process Vote Window <sort> */
        std::ranges::sort(votesQueue, [](const auto& a, const auto& b) {
            return std::get<2>(a) > std::get<2>(b);
        });

        /* Create Helper Vector for top voted delegates*/
        if (votesQueue.size() > maxDelegates) {
            /* votes in queue surpasses maximum delegates*/
            std::vector<std::string> topDelegates;
            topDelegates.reserve(maxDelegates);
            for (unsigned short i = 0; i < maxDelegates; i++) {
                topDelegates.emplace_back(std::get<1>(votesQueue[i]));
            }

            /* Create vector for Current Window */
            std::vector<std::string> currentWindow;
            currentWindow.reserve(maxDelegates * votingPeriod);

            /* Add top Delegates To Window Period */
            for (unsigned short round = 0; round < windowPeriod; round++) {
                currentWindow.insert(currentWindow.end(), topDelegates.begin(), topDelegates.end());
            }

            /* Add Window Period To Delegates Queue */
            delegates.reserve(delegates.size() + currentWindow.size());
            delegates.insert(delegates.end(), currentWindow.begin(), currentWindow.end());
        }
        else {
            /* votes in queue <= maximum delegates*/
            std::vector<std::string> topDelegates;
            topDelegates.reserve(votesQueue.size());
            for (unsigned short i = 0; i < votesQueue.size(); i++) {
                topDelegates.emplace_back(std::get<1>(votesQueue[i]));
            }

            /* votes in queue is less than or = maximum allowed delegates */
            std::vector<std::string> currentWindow;
            currentWindow.reserve(votesQueue.size() * votingPeriod);

            /* Add top Delegates To Window Period */
            for (unsigned short round = 0; round < windowPeriod; round++) {
                currentWindow.insert(currentWindow.end(), topDelegates.begin(), topDelegates.end());
            }

            /* Add Window Period To Delegates Queue */
            lastUpd = util::TimeStamp();
            delegates.reserve(delegates.size() + currentWindow.size());
            delegates.insert(delegates.end(), currentWindow.begin(), currentWindow.end());
        }
    }
}

std::unique_ptr<unsigned char[]> Consensus::serializeConsensus() {
    size_t tSize = sizeof(size_t) + sizeof(unsigned long long) + sizeof(unsigned long long) + sizeof(unsigned long);
    tSize += sizeof(unsigned short) + sizeof(unsigned short) + sizeof(float) + sizeof(float);

    std::unique_ptr<unsigned char[]> Buffer(new unsigned char[tSize]);

    size_t offset = 0;
    std::memcpy(Buffer.get() + offset, &tSize, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(Buffer.get() + offset, &timestamp, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    std::memcpy(Buffer.get() + offset, &lastUpd, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    std::memcpy(Buffer.get() + offset, &votingPeriod, sizeof(unsigned long));
    offset += sizeof(unsigned long);

    std::memcpy(Buffer.get() + offset, &windowPeriod, sizeof(unsigned short));
    offset += sizeof(unsigned short);

    std::memcpy(Buffer.get() + offset, &maxDelegates, sizeof(unsigned short));
    offset += sizeof(unsigned short);

    std::memcpy(Buffer.get() + offset, &decayFactor, sizeof(float));
    offset += sizeof(float);

    std::memcpy(Buffer.get() + offset, &minBalance, sizeof(float));

    return Buffer;
}

std::tuple<unsigned long long, unsigned long long, unsigned long,
unsigned short, unsigned short, float, float> Consensus::deserializeConsensus(const std::unique_ptr<unsigned char[]>& data) {

    /* Variables */
    unsigned long long timestamp;
    unsigned long long lastUpd;
    unsigned long votingPeriod;
    unsigned short windowPeriod;
    unsigned short maxDelegates;
    float decayFactor;
    float minBalance;
    size_t tSize;

    size_t offset = 0;
    std::memcpy(&tSize, data.get(), sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&timestamp, data.get() + offset, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    std::memcpy(&lastUpd, data.get() + offset, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    std::memcpy(&votingPeriod, data.get() + offset, sizeof(unsigned long));
    offset += sizeof(unsigned long);

    std::memcpy(&windowPeriod, data.get() + offset, sizeof(unsigned short));
    offset += sizeof(unsigned short);

    std::memcpy(&maxDelegates, data.get() + offset, sizeof(unsigned short));
    offset += sizeof(unsigned short);

    std::memcpy(&decayFactor, data.get() + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&minBalance, data.get() + offset, sizeof(float));

    std::tuple<unsigned long long, unsigned long long, unsigned long, unsigned short, unsigned short, float,
    float>consensus(timestamp, lastUpd, votingPeriod, windowPeriod, maxDelegates, decayFactor, minBalance);

    return consensus;
}

std::unique_ptr<unsigned char[]> Consensus::serializeVector(const std::vector<std::tuple<std::string, std::string, float>>& vec) {
    std::vector<unsigned char> buffer;
    size_t totalSize = sizeof(size_t);  // Reserve space for total buffer size

    size_t numTuples = vec.size();
    buffer.insert(buffer.end(), reinterpret_cast<unsigned char*>(&numTuples), reinterpret_cast<unsigned char*>(&numTuples) + sizeof(numTuples));
    totalSize += sizeof(numTuples);

    auto appendString = [&](const std::string& str) {
        uint32_t length = str.length();
        totalSize += sizeof(length) + length;
        buffer.insert(buffer.end(), reinterpret_cast<unsigned char*>(&length), reinterpret_cast<unsigned char*>(&length) + sizeof(length));
        buffer.insert(buffer.end(), str.begin(), str.end());
    };

    for (const auto& tup : vec) {
        appendString(std::get<0>(tup));
        appendString(std::get<1>(tup));

        float value = std::get<2>(tup);
        buffer.insert(buffer.end(), reinterpret_cast<unsigned char*>(&value), reinterpret_cast<unsigned char*>(&value) + sizeof(value));
        totalSize += sizeof(value);
    }

    // Allocate heap memory for serialization
    std::unique_ptr<unsigned char[]> serializedData(new unsigned char[totalSize]);
    std::memcpy(serializedData.get(), &totalSize, sizeof(totalSize));  // Store total buffer size
    std::memcpy(serializedData.get() + sizeof(totalSize), buffer.data(), buffer.size());

    return serializedData;
}

std::vector<std::tuple<std::string, std::string, float>> Consensus::deserializeVector(const std::unique_ptr<unsigned char[]>& data) {
    size_t offset = sizeof(size_t);  // Start after total size
    size_t numTuples;
    std::memcpy(&numTuples, data.get() + offset, sizeof(numTuples));
    offset += sizeof(numTuples);

    std::vector<std::tuple<std::string, std::string, float>> vec;

    auto extractString = [&](std::string& str) {
        uint32_t length;
        std::memcpy(&length, data.get() + offset, sizeof(length));
        offset += sizeof(length);
        str.assign(reinterpret_cast<const char*>(data.get() + offset), length);
        offset += length;
    };

    for (size_t i = 0; i < numTuples; ++i) {
        std::string str1, str2;
        extractString(str1);
        extractString(str2);

        float value;
        std::memcpy(&value, data.get() + offset, sizeof(value));
        offset += sizeof(value);

        vec.emplace_back(str1, str2, value);
    }

    return vec;
}