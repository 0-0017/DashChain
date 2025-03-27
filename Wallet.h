#ifndef WALLET
#define WALLET

#include "util.h"
#include "transactions.h"

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
struct utxout { size_t txSize; size_t shSize; unsigned char* utxo; unsigned char* utxoSignedHash; };

class Wallet
{
private:
	/* ECDSA Function, Creates Public and Private Keys for newly created wallets */
	EVP_PKEY* generateECDSAKeyPair();
	int get_key_values(EVP_PKEY* pkey);

	/* Wallet address Function, Creates wallet address for newly created wallets */
	std::string genAddress(const unsigned char pubKey[80]);

	/* Private Wallet Variables */
	std::string address;
	util utility;
	EVP_PKEY* keyPair = nullptr;
	std::vector<transactions> UTXO;
	const char* curvename = "P-256";
	EVP_PKEY* pubKeyP = nullptr;
	unsigned char* pubKey = nullptr;
	unsigned char* privKey = nullptr;
	unsigned short txCount = 0;
	double balance = 0;
	unsigned short locktimeUTXO = 0;
	float versionUTXO = 0;

	/* UTXO Implementation */


public:
	/* Constructor */
	Wallet();

	/* Sign & verify functions */
	unsigned char* ecDoSign(EVP_PKEY* keypair, const std::vector<uint8_t>& mesdgst);
	bool ecDoVerify(EVP_PKEY* pkey, const std::vector<uint8_t>& mesdgst, std::vector<unsigned char> signature);
	void extract_public_key();
	unsigned char* calcTxid(unsigned long long ts);
	utxout outUTXO(double feee, std::vector<std::string> rwa, std::vector<EVP_PKEY*> rks,
		std::vector<double>amm);
	void inUTXO(transactions txin);
	bool verifyTx(const utxout& out);
	void setBalance();
	double getBalance();
	std::string getWalletAddr();
	EVP_PKEY* getPubKey();

	/* De-serialize utxo and its data */
	unsigned char* serialize_utxout(const utxout& obj);
	utxout deserialize_utxout(const unsigned char* buffer);

	size_t getSignSize(std::vector<uint8_t> msg) {

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
		if (EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, keyPair) <= 0) {
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

		return siglen;
		EVP_MD_CTX_free(ctx);
	}
};

#endif

