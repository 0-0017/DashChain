/*-- BlockChain.cpp------------------------------------------------------------
   This file implements chain member functions.
------------------------------------------------------------------------------*/
#include "BlockChain.h"

BlockChain::BlockChain() {
	//pointer to first */
	first = NULL;
	currBlock = NULL;
	timestamp = utility.TimeStamp();
	height = 0;


	//Genesis Block Creation */
	//Empty Transaction Data
	std::vector<transactions> txs;
	std::vector<std::string> tra;
	std::vector<EVP_PKEY_ptr> trpk;
	std::vector<double> tamm;
	transactions tx("", tra, nullptr, trpk, tamm, 0.0, 0, 0.0); //*
	txs.push_back(tx);
	//Data To be Hashed */
	std::string dataToHash =
		"Genesis1:1; Thank you Jesus; Thank You God; A New creation => Your Creation";
	unsigned char* data = utility.toUnsignedChar(dataToHash);
	size_t dataSize = dataToHash.size();
	std::vector<uint8_t> GenHash = utility.shaHash(data, dataSize);
	Block* Genesis = new Block(txs, GenHash, 1.0, 1, utility.TimeStamp());
	first = Genesis;
	currBlock = Genesis;
	Genesis->next = NULL;
}

BlockChain::~BlockChain() {
	Block* temp;
	temp = first;
	while (temp != NULL)
	{
		Block* save = temp;
		temp = temp->next;
		delete save;
	}
}


void BlockChain::GenerateBlock(const std::vector<transactions>& d, Block* b) {
	if (b == nullptr) {
		/* Creates Block, set Block Info And Adds Genesis Block */
		Block* preBlk = currBlock;
		Block* newBlk = new Block(d,preBlk->getCurrHash(), 1.0, height + 1, utility.TimeStamp());

		/* Verify Block & update chain */
		if (verifyBlock(newBlk)) {
			preBlk->next = newBlk;
			currBlock = newBlk;
			setHeight();
		}
		else {
			std::cout << "Block Cannot Be Verified\n";
		}
	}
	else {
		Block* preBlk = currBlock;

		/* Verify Block & update chain */
		if (verifyBlock(b)) {
			preBlk->next = b;
			currBlock = b;
			setHeight();
		}

	}
}

bool BlockChain::empty() {
	if (first == NULL) {
		return true;
	}
	else {
		return false;
	}
}

unsigned long long BlockChain::getTimestamp() {
	timestamp = utility.TimeStamp();
	return timestamp;
}

unsigned int BlockChain::getBlockHeight() {
	unsigned int bHeight = currBlock->getBlockHeight();
	return (bHeight + 1);
}

void BlockChain::setHeight() {
	height++;
}

bool BlockChain::isNewTxid(const unsigned char* txid) {
	Block* ptr = first;
	std::vector<transactions> pbTxs;
	while (ptr->next != NULL) {
		pbTxs = ptr->getTxs();
		for (int i = 0; i < pbTxs.size(); i++) {
			if (pbTxs[i].getTxid() == txid) {
				return false; // tx is in blockchain
			}
		}
		ptr = ptr->next;
	}
	delete(ptr);
	return true;
}

/* Verifies the integrity of the entire blockchain //inco */
bool BlockChain::verifyBlockchain() {
	Block* ptr = first;
	while (ptr->next != NULL) {
		if (ptr->getCurrHash() != ptr->next->getPrevHash()) {
			return false; // If any hash mismatch, blockchain is invalid
		}
		if (ptr != first && !verifyBlock(ptr)) {
			return false;
		}
		ptr = ptr->next;
	}
	delete(ptr);
	return true;
}

bool BlockChain::verifyBlock(Block* newBlock) {
	/* Check Timestamp */
	if (newBlock->getTimestamp() == NULL) {
		std::cout << "Block rejected: Block Invalid" << std::endl;
		return false;
	}

	/* Verify Version */
	if (newBlock->getVersion() != currBlock->getVersion()) {
		std::cout << "Block rejected: Version mismatch" << std::endl;
		return false;
	}

	/* Verify the previous hash matches */
	if (newBlock->getPrevHash() != currBlock->getCurrHash()) {
		std::cout << "Block rejected: Previous hash mismatch" << std::endl;
		return false;
	}

	/* Verify block height matches the current height + 1 */
	if (newBlock->getBlockHeight() != currBlock->getBlockHeight() + 1) {
		std::cout << "Block rejected: Invalid block height" << std::endl;
		return false;
	}

	/* verify transactions */
	std::vector<uint8_t> tx = newBlock->getMerkleRoot(); //currBlock->MerkleRoot(newBlock->data)
	if (tx != newBlock->getMerkleRoot()) {
		std::cout << "Block rejected: Invalid Merkle!" << std::endl;
		return false;
	}

	return true; // Block is valid
}

Block* BlockChain::getCurrBlock() {
	return currBlock;
}

unsigned long long BlockChain::getChnTmstmp() {
	return timestamp;
}

unsigned long long BlockChain::getChnSlot() {
	return slot;
}

void BlockChain::updateChnSlot() {
	slot++;
}

std::vector<transactions> BlockChain::checkWallets(std::string wa) {
	/* Only Check Current block for transactions */
	std::vector<transactions> pbTxs = currBlock->getData();
	std::vector<transactions> txout;
	for (int i = 0; i < pbTxs.size(); i++) {
		std::vector<std::string> ra = pbTxs[i].getRecieveAddr();
		for (int j = 0; j < ra.size(); j++) {
			if (ra[j] == wa) {
				txout.push_back(pbTxs[i]); // wa is in blockchain
			}
		}
	}
	return txout;
}

void BlockChain::display() {
	util ut;
	std::cout << "BlockChain Timestamp: " + ut.toString(getTimestamp()) << std::endl;
	std::cout << "Block TimeStamp: " + ut.toString(currBlock->getTimestamp()) << std::endl;
	std::cout << "Current Hash: " + ut.toString(currBlock->getCurrHash()) << std::endl;
	std::cout << "Previous Hash: " + ut.toString(currBlock->getPrevHash()) << std::endl;
	std::cout << "Block Height: " + ut.toString(getBlockHeight()) << std::endl;
	std::cout << "Merkle Root: " + ut.toString(currBlock->getMerkleRoot()) << std::endl;
}
