#pragma once
#include"util.h"

/*-- Transactions.h ---------------------------------------------------------------
  This header file defines the Transaction Logic that will be used to send and recieve
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
Locktime
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
private:
	/* Variables */
	unsigned long long timestamp = 0;
	unsigned char* txid;
	std::string sendAddr;
	std::vector<std::string> recieveAddr;
	EVP_PKEY* sendPkey = nullptr;
	std::vector<EVP_PKEY*> recievePkeys;
	std::vector<double> ammount;
	double fee = 0;
	unsigned short locktime = 0;
	float version = 0;

public:
	/* Transaction data Getters and Setters */
	transactions();
	/* Destructor to free EVP_PKEY pointers */
	~transactions();
	unsigned long long getTimeStamp() const;
	unsigned char* getTxid() const;
	std::string getSendAddr() const;
	std::vector<std::string> getRecieveAddr() const;
	std::vector<double> getAmmount() const;
	EVP_PKEY* getSendPkey() const;
	std::vector<EVP_PKEY*> getRecievePkeys() const;
	double getFee() const;
	unsigned short getLockTime() const;
	float getVersion() const;

	void setTimeStamp(unsigned long long ts);
	void setTxid(unsigned char* tx);
	void setSendAddr(std::string sa);
	void setRecieveAddr(std::vector<std::string> addy);
	void setAmmount(std::vector<double> amm);
	void setSendPkey(EVP_PKEY* sk);
	void setRecievePkeys(std::vector<EVP_PKEY*> rpk);
	void setFee(double fe);
	void setLockTime(unsigned short lt);
	void setVersion(float v);

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

		return tSize;
	}

};
