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
  All Member Vars w/ Exception next are Constant for added Integrity!
-------------------------------------------------------------------------*/
#include "util.h"
#include "transactions.h"
#include <cstddef>

class Block {
private:
	/* Block basic data */
	const std::vector<transactions> data;
	static util utility;

	/* Block Head, Struct assembled */
	struct Head
	{
		/* Unsigned long-long timestamp */
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

	/* Current height, hash & Size of Block */
	const unsigned int blockHeight;
	const size_t blockSize;
	const std::vector<uint8_t> currHash;

public:
	/* Constructor Ran on Block Creation */
	Block(const std::vector<transactions>& d, std::vector<uint8_t> prevHash, float versionNum, unsigned int blockHeight,
		unsigned long long ts = setTimestamp(), Block* n = nullptr);

	/* Copy Constructor */
	Block(const Block& copy);

	/* Next Pointer */
	Block* next;

	/* Getter & Setter for timestamp */
	unsigned long long getTimestamp() const;
	static unsigned long long setTimestamp();

	/* Version Control Getter & Setters */
	float getVersion() const;

	/* Sets and gets current block size & Height*/
	unsigned int getBlockHeight() const;

	/* Merkle root of transactions */
	std::vector<uint8_t> getMerkleRoot() const;
	std::vector<uint8_t> MerkleRoot(const std::vector<transactions>& tx) const;

	/* Serialize method */
	unsigned char* serialize() const;
	Block* deserialize(const unsigned char* buffer) const;

	/* Getters and Setters for Hashes */
	std::vector<uint8_t> setCurrHash() const;
	std::vector<uint8_t> getCurrHash() const;
	std::vector<uint8_t> getPrevHash() const;

	/* Get transactions */
	std::vector<transactions> getTxs() const;
	std::vector<transactions> getData() const;

	/* Size Calculations
	*	Precedes Current Hash & Next.
	*	Negates Utility for simplicity
	*/
	size_t setSize() const;
	size_t getSize() const;
};

#endif