//
// Created by king dash on 5/23/25.
//

#include "Consensus.h"

Consensus::Consensus()
{
    timestamp = util::TimeStamp();
    lastUpd = timestamp;
    votingPeriod = 0;
    windowPeriod = 1;
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
        std::string random;

        /* Achieve Uniformity */
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

        delID = ("D" + random + util::toString(util::TimeStamp()));
    }while (delIDExist(delID));

    delegateID.push_back(delID);
    return delID;
}

std::vector<std::string>  Consensus::getDelegates() {
    return delegates;
}

std::vector<std::string> Consensus::getDelegateIDs() {
    return delegateID;
}
std::vector<std::tuple<std::string, std::string, float>> Consensus::getVotesQueue() {
    return votesQueue;
}

std::tuple<bool, std::string> Consensus::requestDelegate(const double balance) {

    if (balance >= minBalance) {
        const std::string id = genDelegateID();
        delegates.push_back(id);
        return {true, id};
    }
    std::cout << "Peer Already Confirmed Delegate || Balance Too Low!\n";
    return {false, ""};
}

void Consensus::updatedVotes(const std::vector<std::tuple<std::string, std::string, float>>& votes) {
    for (auto& vote : votes) {
        votesQueue.emplace_back(vote);
    }
}

void Consensus::setTimestamp(const unsigned long long ts) {
    timestamp = ts;
}
unsigned long long Consensus::getTimestamp() const {
    return timestamp;
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
    std::string currentDelegate = delegates.front();
    delegates.erase(delegates.begin());
    return currentDelegate;
}

void Consensus::updateDelegates() {
    if (const unsigned long long timeNow = util::TimeStamp(); timeNow - lastUpd >= votingPeriod) {

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
                std::get<2>(entry) += decayFactor;
            }
        }

        /* Process Vote Window <sort> */
        std::ranges::sort(votesQueue, [](const auto& a, const auto& b) {
            return std::get<2>(a) > std::get<2>(b);
        });

        /* Create Helper Vector for top voted delegates*/
        if (votesQueue.size() > maxDelegates) {
            /* votes in queue surpasses maximum delegates*/
            std::vector<std::string> topDelegates(maxDelegates);
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
            std::vector<std::string> topDelegates(votesQueue.size());
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

unsigned char* Consensus::serializeConsensus() {
    size_t tSize = sizeof(size_t) + sizeof(unsigned long long) + sizeof(unsigned long long) + sizeof(unsigned long);
    tSize += sizeof(unsigned short) + sizeof(unsigned short) + sizeof(float) + sizeof(float);

    unsigned char* Buffer = new unsigned char[tSize];

    size_t offset = 0;
    std::memcpy(Buffer, &tSize, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(Buffer + offset, &timestamp, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    std::memcpy(Buffer + offset, &lastUpd, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    std::memcpy(Buffer + offset, &votingPeriod, sizeof(unsigned long));
    offset += sizeof(unsigned long);

    std::memcpy(Buffer + offset, &windowPeriod, sizeof(unsigned short));
    offset += sizeof(unsigned short);

    std::memcpy(Buffer + offset, &maxDelegates, sizeof(unsigned short));
    offset += sizeof(unsigned short);

    std::memcpy(Buffer + offset, &decayFactor, sizeof(float));
    offset += sizeof(float);

    std::memcpy(Buffer + offset, &minBalance, sizeof(float));

    return Buffer;
}

std::tuple<unsigned long long, unsigned long long, unsigned long,
unsigned short, unsigned short, float, float> Consensus::deserializeConsensus(unsigned char* data) {

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
    std::memcpy(&tSize, data, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&timestamp, data + offset, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    std::memcpy(&lastUpd, data + offset, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    std::memcpy(&votingPeriod, data + offset, sizeof(unsigned long));
    offset += sizeof(unsigned long);

    std::memcpy(&windowPeriod, data + offset, sizeof(unsigned short));
    offset += sizeof(unsigned short);

    std::memcpy(&maxDelegates, data + offset, sizeof(unsigned short));
    offset += sizeof(unsigned short);

    std::memcpy(&decayFactor, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&minBalance, data + offset, sizeof(float));

    std::tuple<unsigned long long, unsigned long long, unsigned long, unsigned short, unsigned short, float,
    float>consensus(timestamp, lastUpd, votingPeriod, windowPeriod, maxDelegates, decayFactor, minBalance);

    return consensus;
}