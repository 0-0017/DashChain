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
		const std::vector<unsigned char> prevHash;

		/* Version Control */
		const float versionNum;//*

		/* Merkle Root of transactions */
		const std::vector<unsigned char> merkleRoot;

		// Constructor to initialize all const members
		Head(unsigned long long ts, std::vector<unsigned char> prev, float ver, std::vector<unsigned char> merkle)
			: timestamp(ts), prevHash(std::move(prev)), versionNum(ver), merkleRoot(std::move(merkle)) {
		}
	};

	/* Instance of Head structure */
	const Head head;

	/* Current height, hash & Size of Block */
	const unsigned int blockHeight;
	const size_t blockSize;
	const std::vector<unsigned char> currHash;

public:
	/* Constructor Ran on Block Creation */
	Block(const std::vector<transactions>& d, std::vector<unsigned char> prevHash, float versionNum, unsigned int blockHeight,
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

	/* display current block  */
	void display();

	/* Merkle root of transactions */
	std::vector<unsigned char> getMerkleRoot() const;
	std::vector<unsigned char> MerkleRoot(const std::vector<transactions>& tx) const;

	/* Serialize method */
	std::unique_ptr<unsigned char[]> serialize() const;
	Block* deserialize(std::unique_ptr<unsigned char[]> buffer) const;

	/* Getters and Setters for Hashes */
	std::vector<unsigned char> setCurrHash() const;
	std::vector<unsigned char> getCurrHash() const;
	std::vector<unsigned char> getPrevHash() const;

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