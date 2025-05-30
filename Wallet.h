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
struct utxout { size_t txSize = 0; size_t shSize = 0; std::shared_ptr<unsigned char> utxo; std::shared_ptr<unsigned char> utxoSignedHash;};

class Wallet
{
public:
	/* Constructor */
	Wallet();

	/* ECDSA Function, Creates Public and Private Keys for newly created wallets */
	EVP_PKEY_ptr generateECDSAKeyPair();

	/* Wallet Methods */
	unsigned char* ecDoSign(const EVP_PKEY_ptr& keypair, const std::vector<uint8_t>& mesdgst);
	bool ecDoVerify(const EVP_PKEY_ptr& pkey, const std::vector<uint8_t>& mesdgst, const std::vector<unsigned char>& signature) const ;
	EVP_PKEY_ptr extract_public_key();
	utxout outUTXO(double feee, const std::vector<std::string>& rwa, const std::vector<double>& amm, const std::vector<std::string> &delegates,
		const std::vector<std::string> &delegateID, const std::vector<std::tuple<std::string, std::string, float>> &votesQueue);
	void inUTXO(const transactions& txin);
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
	unsigned char* serialize_utxout(const utxout& obj) const ;
	utxout deserialize_utxout(const unsigned char* buffer) const ;

	size_t getSignSize(const std::vector<uint8_t>& msg) const {
		/* Convert the message digest vector to a byte array */
		const unsigned char* md = msg.data();
		size_t mdlen = msg.size();

		/* Create a digest sign context */
		EVP_MD_CTX* ctx = EVP_MD_CTX_new();
		if (ctx == nullptr) {
			std::cerr << "ctx creation failed For Sign Size\n";
			return 0;
		}

		/* Initialize the digest sign operation */
		if (EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, keyPair.get()) <= 0) {
			std::cerr << "sign_init Failed For Sign Size\n";
			EVP_MD_CTX_free(ctx);
			return 0;
		}

		/* Update the digest sign operation with the message digest */
		if (EVP_DigestSignUpdate(ctx, md, mdlen) <= 0) {
			std::cerr << "sign_update Failed For Sign Size\n";
			EVP_MD_CTX_free(ctx);
			return 0;
		}

		/* Finalize the digest sign operation and get the signature length */
		size_t siglen;
		if (EVP_DigestSignFinal(ctx, nullptr, &siglen) <= 0) {
			std::cerr << "sign_final Failed For Sign Size\n";
			EVP_MD_CTX_free(ctx);
			return 0;
		}

		EVP_MD_CTX_free(ctx);
		return siglen;
	}

private:
	/* Wallet address Function, Creates wallet address for newly created wallets */
	std::string genAddress();

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