#ifndef BLOCK
#define BLOCK
/*-- Block.h ---------------------------------------------------------------

  This header file defines the Block class for building The Blockchain.
  Basic operations are:
	 Constructor
  This class has two public data members as well:
	  data - data element for the node
	  next - pointer to the next node in the list or null if no items
			 follow

Note:
  AI Will be a Major Future Focus in this class like all other classes
  All Member Vars w/ Expetion next are Constant for added Integrity!
-------------------------------------------------------------------------*/
#include "util.h"
#include "transactions.h"
#include <cstddef>

class Block {
private:
	/* Block basic data */
	const std::vector<transactions> data;
	static util utility;
	const unsigned int blockHeight;

	/* Block Head, Struct assembled */
	struct Head
	{
		/* Unsigned long long timestamp */
		const unsigned long long timestamp;

		/* Vector unsigned int of binary data */
		const std::vector<uint8_t> prevHash;

		/* Version Control */
		const float versionNum;//*

		/* Merkle Root of transactions */
		const std::vector<uint8_t> merkleRoot;

		// Constructor to initialize all const members
		Head(unsigned long long ts, std::vector<uint8_t> prev, float ver, std::vector<uint8_t> merkle)
			: timestamp(ts), prevHash(std::move(prev)), versionNum(ver), merkleRoot(std::move(merkle)) {
		}
	};

	/* Instance of Head structure */
	const Head head;

	/* Curret hash of Block */
	const std::vector<uint8_t> currHash;

	/* Current size of block */
	const uint32_t blockSize;

public:
	/* Constuctor Ran on Block Creation */
	Block(const std::vector<transactions>& d, std::vector<uint8_t> prevHash, float versionNum, unsigned int blockHeight, 
		unsigned long long ts = setTimestamp(), Block* n = nullptr);

	/* Copy Constructor */
	Block(const Block& copy);

	/* Next Pointer */
	Block* next;

	/* Getter & Setter for timestamp */
	unsigned long long getTimestamp();
	static unsigned long long setTimestamp();

	/* Version Control Getter & Setters */
	float getVersion();

	/* Sets and gets current block size & Height*/
	uint32_t getBlockSize();
	unsigned int getBlockHeight();

	/* Merkle root of transactions */
	std::vector<uint8_t> getMerkleRoot();
	std::vector<uint8_t> MerkleRoot(const std::vector<transactions>& tx);

	/* Serialize method */
	unsigned char* serialize() const;
	Block* deserialize(const unsigned char* buffer);

	/* Getters and Setters for Hashes */
	std::vector<uint8_t> setCurrHash() const;
	std::vector<uint8_t> getCurrHash();
	std::vector<uint8_t> getPrevHash();

	/* Get trasactions */
	std::vector<transactions> getTxs();
	std::vector<transactions> getData();

	/* Size Caculations
	*	Precedes Current Hash & Next.
	*	Negates Utility for simplicity
	*/
	uint32_t setSize() const;
	uint32_t getSize();
};

#endif