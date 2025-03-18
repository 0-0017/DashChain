#ifndef BLOCK
#define BLOCK
/*-- Block.h ---------------------------------------------------------------

  This header file defines the Blocl class for building The Blockchain.
  Basic operations are:
	 Constructor
  This class has two public data members as well:
	  data - data element for the node
	  next - pointer to the next node in the list or null if no items
			 follow

Note:
  AI Will be a central part in this class like all other classes
-------------------------------------------------------------------------*/
#include "util.h"
#include "transactions.h"
#include <cstddef>

class Block {
private:

	/* Block Head, Struct assembled */
	struct Head
	{
		/* Unsigned long long timestamp */
		unsigned long long timestamp;

		/* Vector unsigned int of binary data */
		std::vector<uint8_t> prevHash;

		/* Version Control */
		float versionNum;//*

		/* Merkle Root of transactions */
		std::vector<uint8_t> merkleRoot;
	};

	/* Instance of Head structure */
	Head head;

	/* Curret hash of Block */
	std::vector<uint8_t> currHash;

	/* Current size of block */
	uint32_t blockSize;

public:
	/* Constuctor Ran on Block Creation */
	Block(const std::vector<transactions>& d, Block* n = NULL); //*

	/* Getter for timestamp */
	unsigned long long getTimestamp();

	/* Version Control Getter & Setters */
	void setVersion(float v);
	float getVersion();

	/* Sets and gets current block size */
	void setBlockSize(uint32_t size);
	uint32_t getBlockSize();

	/* Merkle root of transactions */
	void setMerkleRoot(const std::vector<transactions>& d);
	std::vector<uint8_t> getMerkleRoot();
	std::vector<uint8_t> MerkleRoot(const std::vector<transactions>& tx);

	/* Serialize method */
	unsigned char* serialize() const;
	Block* deserialize(const unsigned char* buffer);

	/* Getters and Setters for Hashes */
	void setCurrHash();
	std::vector<uint8_t> getCurrHash();
	void setPrevHash(std::vector<uint8_t> ph);
	std::vector<uint8_t> getPrevHash();

	/* Get trasactions */
	std::vector<transactions> getTxs();

	/* Block basic data and next ptr */
	std::vector<transactions> data;
	Block* next;
	util utility;
	unsigned int blockHeight;
};

#endif
