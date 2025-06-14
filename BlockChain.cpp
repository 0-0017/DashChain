/*-- BlockChain.cpp------------------------------------------------------------
   This file implements chain member functions.
------------------------------------------------------------------------------*/
#include "BlockChain.h"

BlockChain::BlockChain()
{
	//pointer to first */
	first = nullptr;
	currBlock = nullptr;
	height = 0;
	slot = 0;
	version = 1.0;
	timestamp = util::TimeStamp();
	util::logCall("BLOCKCHAIN", "BlockChain()", true);
}

BlockChain::~BlockChain() {
	Block* temp;
	temp = first;
	while (temp != nullptr)
	{
		Block* save = temp;
		temp = temp->next;
		delete save;
	}
	util::logCall("BLOCKCHAIN", "~BlockChain()", true);
}

void BlockChain::initial(Block* initial) {
	if (initial == nullptr) {
		//Genesis Block Creation */
		//Empty Transaction Data
		std::vector<transactions> txs;
		std::vector<std::string> tra;
		std::vector<EVP_PKEY_ptr> trpk;
		tra.emplace_back("NA");
		std::vector<double> tamm;
		tamm.push_back(1.0);
		std::vector<std::string> del;
		std::vector<std::string> delID;
		std::vector<std::tuple<std::string, std::string, float>> votes;
		transactions tx("NA", tra, tamm, 0.0, 7, 1.0, del, delID, votes);
		txs.push_back(tx);
		//Data To be Hashed */
		std::string dataToHash = "Genesis1:1; Thank you Jesus; Thank You God; A New creation => Your Creation";
		std::vector<unsigned char> genhash;
		if (util::shaHash(dataToHash, genhash)) {
			Block* Genesis = new Block(txs, genhash, version, height, util::TimeStamp());
			first = Genesis;
			currBlock = Genesis;
			Genesis->next = nullptr;
			util::logCall("BLOCKCHAIN", "initial()", true);
		}
		else {
			util::logCall("BLOCKCHAIN", "initial()", false, "Block could not be created");
			std::cerr << "BlockChain::initial(): Block could not be created" << std::endl;
		}
	}
	else {
		first = initial;
		currBlock = initial;
		initial->next = nullptr;
		util::logCall("BLOCKCHAIN", "initial()", true);
	}
}
void BlockChain::GenerateBlock(const std::vector<transactions>& d, Block* b) {
	if (b == nullptr) {
		/* Creates Block, set Block Info And Adds Genesis Block */
		Block* preBlk = currBlock;
		Block* newBlk = new Block(d,preBlk->getCurrHash(), getVersion(), (height + 1), util::TimeStamp());

		/* Verify Block & update chain */
		if (verifyBlock(newBlk)) {
			preBlk->next = newBlk;
			currBlock = newBlk;
			currBlock->next = nullptr;
			setHeight();
			updateChnSlot();
			util::logCall("BLOCKCHAIN", "GenerateBlock()", true);
		}
		else {
			util::logCall("BLOCKCHAIN", "GenerateBlock()", false, "Block Cannot Be Verified");
			std::cout << "Block Cannot Be Verified\n";
		}
	}
	else {
		Block* preBlk = currBlock;

		/* Verify Block & update chain */
		if (verifyBlock(b)) {
			preBlk->next = b;
			currBlock = b;
			currBlock->next = nullptr;
			setHeight();
			updateChnSlot();
		}
		util::logCall("BLOCKCHAIN", "GenerateBlock()", true);
	}
}

bool BlockChain::empty() {
	if (first == nullptr) {
		return true;
	}
	else {
		return false;
	}
}

float BlockChain::getVersion() {
	return version;
}

void BlockChain::setVersion(float vnum) {
	version = vnum;
}


unsigned long long BlockChain::getTimestamp() const {
	return timestamp;
}

void BlockChain::setChnTmstmp(unsigned long long ts) {
	timestamp = ts;
}

unsigned int BlockChain::getBlockHeight() {
	return currBlock->getBlockHeight();
}

void BlockChain::setHeight() {
	height++;
}

bool BlockChain::isNewTxid(const std::string txid) {
	Block* ptr = first;
	std::vector<transactions> pbTxs;
	while (ptr->next != nullptr) {
		pbTxs = ptr->getTxs();
		for (int i = 0; i < pbTxs.size(); i++) {
			if (pbTxs[i].getTxid() == txid) {
				util::logCall("BLOCKCHAIN", "isNewTxid()", true);
				return false; // tx is in blockchain
			}
		}
		ptr = ptr->next;
	}
	delete(ptr);
	util::logCall("BLOCKCHAIN", "isNewTxid()", true);
	return true;
}

transactions BlockChain::getTx(const std::string txid) {
	Block* ptr = first;
	std::vector<transactions> pbTxs;
	while (ptr->next != nullptr) {
		pbTxs = ptr->getTxs();
		for (int i = 0; i < pbTxs.size(); i++) {
			if (pbTxs[i].getTxid() == txid) {
				util::logCall("BLOCKCHAIN", "getTx()", true);
				return pbTxs[i]; // return tx
			}
		}
		ptr = ptr->next;
	}
	delete(ptr);

	/* Return Dummy Tx */
	std::vector<transactions> txs;
	std::vector<std::string> tra;
	std::vector<EVP_PKEY_ptr> trpk;
	std::vector<double> tamm;
	std::vector<std::string> del;
	std::vector<std::string> delID;
	std::vector<std::tuple<std::string, std::string, float>> votes;
	transactions dummy("", tra, tamm, 0.0, 0.0, 0.0, del, delID, votes);
	util::logCall("BLOCKCHAIN", "getTx()", true);
	return dummy;
}

/* Verifies the integrity of the entire blockchain //inco */
bool BlockChain::verifyBlockchain() {
	Block* ptr = first;
	while (ptr->next != nullptr) {
		if (ptr->getCurrHash() != ptr->next->getPrevHash()) {
			util::logCall("BLOCKCHAIN", "verifyBlockchain()", false, "Hash Mix Match");
			return false; // If any hash mismatch, blockchain is invalid
		}
		if (ptr != first && !verifyBlock(ptr)) {
			util::logCall("BLOCKCHAIN", "verifyBlockchain()", false, "Verification Failed");
			return false;
		}
		ptr = ptr->next;
	}
	delete(ptr);
	return true;
}

bool BlockChain::verifyBlock(Block* newBlock) {
	/* Check Timestamp */
	if (newBlock->getTimestamp() == 0) {
		util::logCall("BLOCKCHAIN", "verifyBlock()", false, "Block rejected: Block Invalid");
		std::cout << "Block rejected: Block Invalid" << std::endl;
		return false;
	}

	/* Verify Version */
	if (newBlock->getVersion() != getVersion()) {
		util::logCall("BLOCKCHAIN", "verifyBlock()", false, "Block rejected: Version mismatch");
		std::cout << "Block rejected: Version mismatch" << std::endl;
		return false;
	}

	/* Verify the previous hash matches */
	if (newBlock->getPrevHash() != currBlock->getCurrHash()) {
		util::logCall("BLOCKCHAIN", "verifyBlock()", false, "Block rejected: Previous hash mismatch");
		std::cout << "Block rejected: Previous hash mismatch" << std::endl;
		return false;
	}

	/* Verify block height matches the current height + 1 */
	if (newBlock->getBlockHeight() != (getBlockHeight() + 1)) {
		util::logCall("BLOCKCHAIN", "verifyBlock()", false, "Block rejected: Invalid block height");
		std::cout << "Block rejected: Invalid block height" << std::endl;
		return false;
	}

	/* verify transactions */
	std::vector<uint8_t> tx = newBlock->getMerkleRoot(); //currBlock->MerkleRoot(newBlock->data)
	if (tx != newBlock->getMerkleRoot()) {
		util::logCall("BLOCKCHAIN", "verifyBlock()", false, "Block rejected: Invalid Merkle!");
		std::cout << "Block rejected: Invalid Merkle!" << std::endl;
		return false;
	}

	return true; // Block is valid
}

Block* BlockChain::getCurrBlock() {
	return currBlock;
}

Block* BlockChain::getFirstBlock() {
	return first;
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
	util::logCall("BLOCKCHAIN", "checkWallets()", true);
	return txout;
}

void BlockChain::getBlock(unsigned int bheight) {
	if (bheight <= height) {
		Block* ptr = first;
		while (ptr->next != nullptr) {
			if (bheight == ptr->getBlockHeight()) {
				ptr->display();
			}
			ptr = ptr->next;
		}
		util::logCall("BLOCKCHAIN", "getBlock()", true);
		delete(ptr);
	}
	else {
		std::cout << "Block height not yet reached!\n";
	}
}

void BlockChain::display() {
	if (empty()) {
		std::cout << "Blockchain is empty" << std::endl;
	}
	else {
		std::cout << "===================================" << std::endl;
		std::cout << "|        Block Information        |" << std::endl;
		std::cout << "===================================" << std::endl;
		std::cout << "Blockchain Timestamp : " << util::toString(getTimestamp()) << std::endl;
		std::cout << "Block Timestamp      : " << util::toString(currBlock->getTimestamp()) << std::endl;
		std::cout << "Current Hash         : " << util::toString(currBlock->getCurrHash()) << std::endl;
		std::cout << "Previous Hash        : " << util::toString(currBlock->getPrevHash()) << std::endl;
		std::cout << "Block Height         : " << util::toString(getBlockHeight()) << std::endl;
		std::cout << "Merkle Root          : " << util::toString(currBlock->getMerkleRoot()) << std::endl;
		std::cout << "Slot Number          : " << util::toString(getChnSlot()) << std::endl;
		std::cout << "Version              : " << util::toString(getVersion()) << std::endl;
		std::cout << "===================================" << std::endl;
	}
}
