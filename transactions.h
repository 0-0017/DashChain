#pragma once
#include"util.h"
#include "CryptoTypes.h"

/*-- Transactions.h ---------------------------------------------------------------
  This header file defines the Transaction Logic that will be used to send and receive
  transactions in this system

 elements:
Transaction ID (TXID)
recipient's address
Data (if any)
Chain ID
Inputs:
	Previous Transaction Hash
	Index
	Signature Script (Unlocking Script)
Outputs:
	Value
	Locking Script (ScriptPubKey)
Localtime
Version

  Basic operations are:
	 Constructor
	 GenerateBlock:   Insert an item
	 display:  Output the list

Note:
  AI Will be a Major Future Focus in this class like all other classes
-------------------------------------------------------------------------*/

class transactions
{
public:
	/* Transaction data Getters and Setters */
	explicit transactions(std::string sa, std::vector<std::string> ra, EVP_PKEY_ptr spk, std::vector<EVP_PKEY_ptr> rpk, std::vector<double> amm,
		double fe, unsigned short lk, float v, std::vector<std::string> delegate, std::vector<std::string> delegatesID, std::vector<std::tuple<std::string, std::string, float>> votes,
		unsigned long long timestamp = setTimeStamp(), std::string tid = setTxid());

	/* Copy Constructor */
	transactions(const transactions& copy);

	/* Destructor to free EVP_PKEY pointers */
	~transactions();
	unsigned long long getTimeStamp() const;
	const std::string getTxid() const;
	std::string getSendAddr() const;
	std::vector<std::string> getRecieveAddr() const;
	std::vector<double> getAmmount() const;
	EVP_PKEY_ptr getSendPkey() const;
	std::vector<EVP_PKEY_ptr> getRecievePkeys() const;
	double getFee() const;
	unsigned short getLockTime() const;
	float getVersion() const;
	std::vector<std::string> getDelegates() const;
	std::vector<std::string> getDelegatesID() const;
	std::vector<std::tuple<std::string, std::string, float>> getVotes() const;
	static unsigned long long setTimeStamp();
	static std::string setTxid();

	/* verification */
	bool inputsValid() const;
	bool outputsValid() const;
	double totalAmm() const;

	unsigned char* serialize() const;
	static transactions deserialize(const unsigned char* data);

	/* Get size for serialization */
	size_t getSize() const {
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
	    tSize = sizeof(unsigned short) + sizeof(float) + sizeof(double) + sizeof(unsigned long long);

	    /* Variable Vars */
	    size_t numAmm = 0, recAddAmm = 0, prkAmm = 0, delAmm = 0, delIDAmm = 0, vQueAmm = 0;
	    size_t sendSize = 0, txidSize = 0, ammSize = 0, spkSize = 0, recAddSize = 0, rpkSize = 0, delSize = 0, delIDSize = 0, vQueSize = 0;

	    sendSize = sendAddr.size() * sizeof(char); //Calculate size of sendAddr String

	    /* Calculate txid Size */
	    txidSize = txid.size() * sizeof(char);

	    numAmm = ammount.size();
	    ammSize = ammount.size() * sizeof(double); // Calculate size of amount vector

	    /* Calculate sendPkey Size */
	    int len = i2d_PUBKEY(sendPkey.get(), nullptr);
	    spkSize = len;

	    /* Calculate recieveAddr Size */
	    recAddAmm = recieveAddr.size();
	    for (const auto& addr : recieveAddr) {
	        recAddSize += addr.size() + 1; // Size for length + actual string content
	    }

	    /* Calculate recievePkeys size */
	    for (const auto& pkey : recievePkeys) {
	        int lenn = i2d_PUBKEY(pkey.get(), nullptr);
	        rpkSize += lenn;
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
	    tSize = tSize + sendSize + txidSize + ammSize + spkSize + recAddSize + rpkSize + delSize + delIDSize + vQueSize;
	    tSize = tSize + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t);
	    tSize = tSize + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t);

		return tSize;
	}

private:
	/* Variables */
	static util ut;
	EVP_PKEY_ptr pubKey;
	const std::vector<std::string> delegates;
	const std::vector<std::string> delegateID;
	const std::vector<std::tuple<std::string, std::string, float>> votesQueue;
	const unsigned long long timestamp;
	const std::string txid;
	const std::string sendAddr;
	const std::vector<std::string> recieveAddr;
	const EVP_PKEY_ptr sendPkey;
	const std::vector<EVP_PKEY_ptr> recievePkeys;
	const std::vector<double> ammount;
	const double fee;
	const unsigned short locktime;
	const float version;
};