//
// Created by king dash on 5/23/25.
//

#ifndef CONSENSUS_H
#define CONSENSUS_H

#include"util.h"

class Consensus {
public:
    Consensus();
    ~Consensus() {
        std::cout << "Destroying Consensus\n";
    }

    std::vector<std::string> getDelegates();
    void setDelegates(const std::vector<std::string> &dels);
    std::vector<std::string> getDelegateIDs();
    void setDelegateIDs(const std::vector<std::string> &delIDs);
    void addDelegateID(const std::string& delegate_id);
    std::vector<std::tuple<std::string, std::string, float>> getVotesQueue();

    /* Request To become a delegate In the network */
    std::tuple<bool, std::string> requestDelegate(double balance);
    std::string genDelegateID();

    /* Get The Current Delegate Assigned to Block Creation */
    std::string getCurrentDelegate();
    bool delIDExist(const std::string &ID) const;

    /* Updated Votes From Network to Consensus */
    void updatedVotes(const std::vector<std::tuple<std::string, std::string, float>>& votes);

    /* Updated Delegate window based on votes in the network */
    void updateDelegates();
    unsigned long long getTimestamp() const;
    void setTimestamp(unsigned long long ts);
    void setLastUpd(unsigned long long lu);
    unsigned long long getLastUpd() const;
    unsigned long getVotingPeriod() const;
    void setVotingPeriod(unsigned long vp);
    unsigned long getWindowPeriod() const;
    void setWindowPeriod(unsigned long wp);
    unsigned long getMaxDelegates() const;
    void setMaxDelegates(unsigned long md);
    float getDecayFactor() const;
    void setDecayFactor(float df);
    float getMinBalance() const;
    void setMinBalance(float mb);

    /* Serialization For Consensus information passed through network */
    std::unique_ptr<unsigned char[]> serializeConsensus();
    std::tuple<unsigned long long, unsigned long long, unsigned long,
    unsigned short, unsigned short, float, float> deserializeConsensus(const std::unique_ptr<unsigned char[]>& data);
    static std::unique_ptr<unsigned char[]> serializeVector(const std::vector<std::tuple<std::string, std::string, float>>& vec);
    static std::vector<std::tuple<std::string, std::string, float>> deserializeVector(const std::unique_ptr<unsigned char[]>& data);

private:
    /*
     * Delegates queue
     * String 1: del ID
     * U Short 1: window
     *
     * Votes Queue
     * String 1: walled address
     * String 2: delegate ID
     * Float 1: votes
     *
     * Delegates: queue of delegates to process blocks
     * delegateID: list of known delegates
     */

    std::vector<std::string> delegates;
    std::vector<std::string> delegateID;
    std::vector<std::tuple<std::string, std::string, float>> votesQueue;
    unsigned long long timestamp;
    unsigned long long lastUpd;
    unsigned long votingPeriod;
    unsigned short windowPeriod;
    unsigned short maxDelegates;
    std::mutex delegatesMutex;
    float decayFactor;
    float minBalance;
    static util u;
};



#endif //CONSENSUS_H
