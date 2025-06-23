#ifndef BLOCKCHAIN
#define BLOCKCHAIN
#include "Block.h"
#include "CryptoTypes.h"
#include <iostream>
/*-- BlockChain.h ---------------------------------------------------------------
  This header file defines the Blockchain Logic that will be the Base for This
  Project
  Basic operations are:
	 Constructor
	 GenerateBlock:   Insert an item
	 display:  Output the list

Note:
  AI Will be a Major Future Focus in this class like all other classes
-------------------------------------------------------------------------*/
class BlockChain
{
public:
	/* Blockchain Constructor */
	BlockChain();

	/* Blockchain Destructor */
	~BlockChain();

	/* Adds Initial Block To BlockChain */
	void initial(Block* initial = nullptr);

	/* Creates a Block into the chain*/
	void GenerateBlock(const std::vector<transactions>& d, Block* b = nullptr);

	/* Checks to see if Blockchain is Empty */
	bool empty();

	/* Returns timestamp of Current Blockchain */
	unsigned long long getTimestamp() const;

	/* Display Blockchain to stdout */
	unsigned int getBlockHeight();

	/* Set Height of the Blockchain */
	void setHeight();

	/* Verifies the balance of a wallet address */
	double verifyBalance(std::string wa);

	/* Verifies the tx-id is not already present in the chain */
	bool isNewTxid(const std::string txid);

	/* Gets transaction if present in the chain */
	transactions getTx(const std::string txid);

	/* Verifies the integrity of all blocks in the chain */
	bool verifyBlockchain();

	/* Verifies if a given block is valid */
	bool verifyBlock(Block* newBlock);

	/* Getter for Current block */
	Block* getCurrBlock();

	/* Getter for First block */
	Block* getFirstBlock();

	/* Getter for timestamp */
	unsigned long long getChnTmstmp();

	/* Getter for timestamp */
	void setChnTmstmp(unsigned long long ts);

	/* Setter for Slot */
	unsigned long long getChnSlot();

	/* Getter for version */
	float getVersion();

	/* Getter for confirmation period */
	unsigned short getConf();

	/* Setter for confirmation period */
	void setConf(unsigned short confirm);

	/* Setter for version */
	void setVersion(float vnum);

	/* Update Method for Slot */
	void updateChnSlot();

	/* Check Method for if wallets in new block */
	std::vector<transactions> checkWallets(std::string wa);

	/* Display block in Blockchain at any height */
	void getBlock(unsigned int bheight);

	/* Returns latest confirmed block! */
	Block* confirmation();

	/* Display Blockchain to stdout */
	void display();

	/* Serialize Current Blockchain State */
	unsigned char* serializeInfo();

	/* Deserialize Current Blockchain State */
	void deserializeInfo(const unsigned char* info);


private:
	Block* first;
	Block* currBlock;
	unsigned long long timestamp;
	unsigned long long slot;
	unsigned short conf;
	static util utility;
	unsigned int height;
	float version;
};

#endif