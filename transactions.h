#pragma once
#include"util.h"
#include "CryptoTypes.h"

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
public:
	/* Transaction data Getters and Setters */
	explicit transactions(std::string sa, std::vector<std::string> ra, EVP_PKEY_ptr spk, std::vector<EVP_PKEY_ptr> rpk, std::vector<double> amm,
		double fe, unsigned short lk, float v, unsigned long long timestamp = setTimeStamp(), std::string tid = setTxid());

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
		There Will be 6 uint32_t & 4 size_t Variables at the start of every Buffer
		This Will Account For The Sized Of:
			uint32_t (In Order)             size_t (In Order)
			1. sendAddr                     tSize
			2. txid
			3. ammount                      size_t numAmm;
			4. sendPkey
			5. recieveAddr                  size_t recAddAmm
			6. recievePkeys                 size_t prkAmm;

			NOTE: txid will be serialized as a string for efficiency purposes.
	*/

	/* Variables */
	// Sizes of: Locktime, Version, Fee & Timestamp (In That Order)
		size_t tSize = 0;
		tSize = sizeof(unsigned short) + sizeof(float) + sizeof(double) + sizeof(unsigned long long);

		/* Variable Vars */
		size_t numAmm = 0, recAddAmm = 0, prkAmm = 0;
		size_t sendSize = 0, txidSize = 0, ammSize = 0, spkSize = 0, recAddSize = 0, rpkSize = 0;

		sendSize = sendAddr.size() * sizeof(char); //Calculate size of sendAddr String

		/* Calculate txid Size */
		txidSize = txid.size() * sizeof(char);

		numAmm = ammount.size();
		ammSize = ammount.size() * sizeof(double); // Calculate size of ammount vector

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

		/* Create Buffer */
		tSize = tSize + sendSize + txidSize + ammSize + spkSize + recAddSize + rpkSize;
		tSize = tSize + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t);
		tSize = tSize +  sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t);

		return tSize;
	}

private:
	/* Variables */
	static util ut;
	EVP_PKEY_ptr pubKey;
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