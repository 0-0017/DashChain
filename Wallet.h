#ifndef WALLET
#define WALLET

#include "util.h"
#include "transactions.h"
#include "CryptoTypes.h"

/*-- wallet.h ---------------------------------------------------------------

  This header file defines the Wallet class for this project.
  It provides functionality for:

  - Generating ECDSA key pairs (public/private keys)
  - Creating unique wallet addresses
  - Managing UTXOs (Unspent Transaction Outputs)
  - Signing and verifying transactions using ECDSA
  - Calculating transaction IDs (TxID)
  - Serializing and deserializing UTXOs for storage and transmission
  - Managing wallet balance and tracking transaction counts
  - Extracting and retrieving public keys
  - Secure handling of cryptographic operations with OpenSSL

  The Wallet class interfaces with the `util` class and `transactions` objects
  to handle blockchain-related logic and crypto operations.

-------------------------------------------------------------------------*/


/* Struct for hashed utxo & signed hash */
struct utxout { size_t txSize = 0; size_t shSize = 0; int pkeySize = 0; std::string utxo;
	std::vector<unsigned char> utxoSignedHash; EVP_PKEY_ptr pubkey;};

class Wallet
{
public:
	/* Constructor */
	Wallet();

	/* ECDSA Function, Creates Public and Private Keys for newly created wallets */
	EVP_PKEY_ptr generateECDSAKeyPair();

	/* Wallet Methods */
	bool ecDoSign(const std::vector<unsigned char> &hash, std::vector<unsigned char> &signature) const;
	bool ecDoVerify(const EVP_PKEY_ptr& pubKey, const std::vector<unsigned char> &hash, const std::vector<unsigned char> &signature);
	EVP_PKEY_ptr extract_public_key();
	utxout outUTXO(double feee, const std::vector<std::string>& rwa, const std::vector<double>& amm, const std::vector<std::string> &delegates,
		const std::vector<std::string> &delegateID, const std::vector<std::tuple<std::string, std::string, float>> &votesQueue);
	void inUTXO(const transactions& txin, size_t index);
	bool verifyTx(const utxout& out);
	void listTxs();
	void setBalance();
	double getBalance() const;
	std::string getWalletAddr() const;
	EVP_PKEY_ptr getPubKey() const;
	unsigned short getLockTime() const;
	void setLockTime(unsigned short lk);
	float getVersion() const;
	void setVersion(float vs);

	/* De-serialize utxo and its data */
	std::unique_ptr<unsigned char[]> serialize_utxout(const utxout& obj) const ;
	utxout deserialize_utxout(const std::unique_ptr<unsigned char[]>& buffer) const ;

private:
	/* Wallet address Function, Creates wallet address for newly created wallets */
	std::string genAddress() const;

	/* Private Wallet Variables */
	std::string address;
	static util utility;
	const EVP_PKEY_ptr keyPair = createEVP_PKEY();
	std::vector<transactions> UTXO;
	const char* curvename = "P-256";
	const EVP_PKEY_ptr pubKeyP = createEVP_PKEY();
	unsigned short txCount;
	double balance;
	unsigned short locktimeUTXO;
	float versionUTXO;


	/* UTXO Implementation */
};

#endif