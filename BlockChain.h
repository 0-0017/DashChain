#ifndef BLOCKCHAIN
#define BLOCKCHAIN
#include "Block.h"
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

	/* Creates a Block into the chain*/
	void GenerateBlock(const std::vector<transactions>& d, Block* b = nullptr);

	/* Checks to see if Blockchain is Empty */
	bool empty();

	/* Returns timestamp of Current Blockchain */
	unsigned long long getTimestamp();

	/* Display Blockchain to stdout */
	unsigned int getBlockHeight();

	/* Set Height of the Blockchain */
	void setHeight();

	/* Verifies the balance of a wallet address */
	double verifyBalance(std::string wa);

	/* Verifies the txid is not already present in the chain */
	bool isNewTxid(unsigned char* txid);

	/* Verifies the integrity of all blocks in the chain */
	bool verifyBlockchain();

	/* Verifies if a given block is valid */
	bool verifyBlock(Block* newBlock);

	/* Getter for Current block */
	Block* getCurrBlock();

	/* Getter for timestamp */
	unsigned long long getChnTmstmp();

	/* Getter for Slot */
	unsigned long long getChnSlot();

	/* Uppdate Method for Slot */
	void updateChnSlot();

	/* check Method for if wallets in new block */
	std::vector<transactions> checkWallets(std::string wa);

	/* Display Blockchain to stdout */
	void display(); //*


private:
	Block* first;
	Block* currBlock;
	unsigned long long timestamp;
	unsigned long long slot;
	util utility;
	unsigned int height;
};

#endif

