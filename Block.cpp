/*-- Block.cpp------------------------------------------------------------
   This file implements node member functions.
-------------------------------------------------------------------------*/
#include "Block.h"
#include <cstddef>
#include <iostream>
using namespace std;


Block::Block(const std::vector<transactions>& d, std::vector<uint8_t> prevHash, float versionNum, unsigned int blockHeight, unsigned long long ts, Block* n)
    : data(d),
    head(ts, std::move(prevHash), versionNum, std::move(MerkleRoot(data))),
    blockHeight(blockHeight),
    blockSize(setSize()),
    currHash(setCurrHash())
{
    next = n;
}

Block::Block(const Block& copy)
    :data(copy.data),
    head(copy.head),
    blockHeight(copy.blockHeight),
    currHash(copy.currHash),
    blockSize(copy.blockSize),
    next(copy.next)
{
}

float Block::getVersion() {
    return head.versionNum;
}

/* Merkle root of transactions */

std::vector<uint8_t> Block::MerkleRoot(const std::vector<transactions>& tx) {
    /* Compute hashes for each transaction */
    std::vector<std::vector<uint8_t>> txHash;
    for (const auto& elem : tx) {
        unsigned char* data = nullptr;
        data = elem.serialize();
        txHash.push_back(utility.shaHash(data));
        delete[] data; // Free the allocated memory
        data = nullptr;
    }
    
    /* Ensure even number of hashes */
    if (txHash.size() % 2 != 0) {
        txHash.push_back(txHash.back());
    }

    /* Compute the Merkle root */
    while (txHash.size() > 1) {
        std::vector<std::vector<uint8_t>> newLevel;
        for (size_t i = 0; i < txHash.size(); i += 2) {
            std::vector<uint8_t> combined(txHash[i].begin(), txHash[i].end());
            combined.insert(combined.end(), txHash[i + 1].begin(), txHash[i + 1].end());
            unsigned char* combinedData = nullptr;
            combinedData = utility.toUnsignedChar(combined);
            newLevel.push_back(utility.shaHash(combinedData));
            delete[] combinedData; // Free the allocated memory
            combinedData = nullptr;
        }
        txHash = newLevel;
    }

    return txHash.front();
}

std::vector<uint8_t> Block::getMerkleRoot() {
    return head.merkleRoot;
}

unsigned long long Block::setTimestamp(){
    return utility.TimeStamp();
}

unsigned long long Block::getTimestamp() {
    return head.timestamp;
}

/* Gets current block size */
uint32_t Block::getBlockSize() {
    return blockSize;
}

unsigned int Block::getBlockHeight() {
    return blockHeight;
}

std::vector<uint8_t> Block::setCurrHash() const{

    /* Block data used for current hash */
    unsigned char* data = serialize();
    return utility.shaHash(data);
}

std::vector<uint8_t> Block::getCurrHash() {
    return currHash;
}


std::vector<uint8_t> Block::getPrevHash() {
    return head.prevHash;
}

std::vector<transactions> Block::getTxs() {
    return data;
}

std::vector<transactions> Block::getData() {
    return data;
}


uint32_t Block::setSize() const{
    uint32_t size = 0;
    size += data.size() + head.merkleRoot.size() + sizeof(unsigned long long) + head.prevHash.size() + sizeof(float) + sizeof(head);
    size += sizeof(uint32_t) + sizeof(unsigned int);
    return size;
}

uint32_t Block::getSize() {
    return blockSize;
}

/* Serialize method */
unsigned char* Block::serialize() const {
    /* Set Const Variables 
        1. blockHeight
        2. versionNum
        3. timestamp
        4. blockSize
    */

    size_t tSize = 0;
    tSize = sizeof(unsigned int) + sizeof(float) + sizeof(unsigned long long) + sizeof(uint32_t);

    /* Variable Vars        uint32_t
        1. prevHash         (phSize)
        2. currHash         (chSize)
        3. merkleRoot       (mrSize)
        4. data             (dSize)
    */

    uint32_t phSize = 0, chSize = 0, mrSize = 0, dSize = 0;
    uint8_t phNum = 0, chNum = 0, mrNum = 0, dNum = 0;

    /* Calculate phSize */
    phNum = head.prevHash.size();
    phSize = phNum * sizeof(uint8_t);

    /* Calculate chSize */
    chNum = currHash.size();
    chSize = chNum * sizeof(uint8_t);

    /* Calculate mr */
    mrNum = head.merkleRoot.size();
    mrSize = mrNum * sizeof(uint8_t);

    /* Calculate data */
    std::vector<unsigned char*> stx; // Temporary vec for serialized tx's
    unsigned char* tdata = nullptr;

    std::vector<size_t> txSizes; // vector of each transaction size

    for (const auto& tx : data) {
        // Serialize each Tx & Push to stx
        tdata = tx.serialize();
        stx.push_back(tdata);

        //Get Size
        size_t serSize = 0;
        std::memcpy(&serSize, tdata, sizeof(serSize));
        txSizes.push_back(serSize);
        dSize += serSize;

        //Clean up
        tdata = nullptr;
        serSize = 0;
        dNum++;
    }
    
    /* Create Buffer */
    size_t txSizesNum = txSizes.size();
    size_t txSizedSize = txSizesNum * sizeof(size_t);
    tSize = tSize + sizeof(size_t) + sizeof(size_t) + txSizedSize + dSize;
    tSize = tSize + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t);
    tSize = tSize + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

    /* Allocate memory for buffer */
    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    /* Serialize tSize itself */
    std::memcpy(buffer + offset, &tSize, sizeof(tSize));
    offset += sizeof(tSize);

    /* Serialize tSizedNum itself */
    std::memcpy(buffer + offset, &txSizesNum, sizeof(txSizesNum));
    offset += sizeof(txSizesNum);

    /* Serialize txSizes itself */
    std::memcpy(buffer + offset, txSizes.data(), txSizedSize);
    offset += txSizedSize;

    /* Serialize phNum itself */
    std::memcpy(buffer + offset, &phNum, sizeof(phNum));
    offset += sizeof(phNum);

    /* Serialize chNum itself */
    std::memcpy(buffer + offset, &chNum, sizeof(chNum));
    offset += sizeof(chNum);

    /* Serialize mrNum itself */
    std::memcpy(buffer + offset, &mrNum, sizeof(mrNum));
    offset += sizeof(mrNum);

    /* Serialize dNum itself */
    std::memcpy(buffer + offset, &dNum, sizeof(dNum));
    offset += sizeof(dNum);

    /* Serialize phSize itself */
    std::memcpy(buffer + offset, &phSize, sizeof(phSize));
    offset += sizeof(phSize);

    /* Serialize chSize itself */
    std::memcpy(buffer + offset, &chSize, sizeof(chSize));
    offset += sizeof(chSize);

    /* Serialize mrSize itself */
    std::memcpy(buffer + offset, &mrSize, sizeof(mrSize));
    offset += sizeof(mrSize);

    /* Serialize dSize itself */
    std::memcpy(buffer + offset, &dSize, sizeof(dSize));
    offset += sizeof(dSize);

    /* Serialize blockHeight itself */
    std::memcpy(buffer + offset, &blockHeight, sizeof(blockHeight));
    offset += sizeof(blockHeight);

    /* Serialize versionNum itself */
    std::memcpy(buffer + offset, &head.versionNum, sizeof(head.versionNum));
    offset += sizeof(head.versionNum);

    /* Serialize timestamp itself */
    std::memcpy(buffer + offset, &head.timestamp, sizeof(head.timestamp));
    offset += sizeof(head.timestamp);

    /* Serialize blockSize itself */
    std::memcpy(buffer + offset, &blockSize, sizeof(blockSize));
    offset += sizeof(blockSize);

    /* Serialize prevHash itself */
    std::memcpy(buffer + offset, head.prevHash.data(), phSize);
    offset += phSize;

    /* Serialize currHash itself */
    std::memcpy(buffer + offset, currHash.data(), chSize);
    offset += chSize;

    /* Serialize currHash itself */
    std::memcpy(buffer + offset, head.merkleRoot.data(), mrSize);
    offset += mrSize;

    /* Serialize data itself */
    std::memcpy(buffer + offset, data.data(), dSize);
    
    return buffer;
}

/* Deserialize method 
* Current Has And Merkle Root Checks for Integrity
*/
Block* Block::deserialize(const unsigned char* buffer) {
    size_t offset = 0;

    // Read total size (tSize)
    size_t tSize = 0;
    std::memcpy(&tSize, buffer + offset, sizeof(tSize));
    offset += sizeof(tSize);

    // Read number of transaction sizes (txSizesNum)
    size_t txSizesNum = 0;
    std::memcpy(&txSizesNum, buffer + offset, sizeof(txSizesNum));
    offset += sizeof(txSizesNum);

    // Read transaction sizes
    std::vector<size_t> txSizes(txSizesNum);
    std::memcpy(txSizes.data(), buffer + offset, txSizesNum * sizeof(size_t));
    offset += txSizesNum * sizeof(size_t);

    // Read number of elements for each hash and data vector
    uint8_t phNum = 0, chNum = 0, mrNum = 0, dNum = 0;
    std::memcpy(&phNum, buffer + offset, sizeof(phNum));
    offset += sizeof(phNum);
    std::memcpy(&chNum, buffer + offset, sizeof(chNum));
    offset += sizeof(chNum);
    std::memcpy(&mrNum, buffer + offset, sizeof(mrNum));
    offset += sizeof(mrNum);
    std::memcpy(&dNum, buffer + offset, sizeof(dNum));
    offset += sizeof(dNum);

    // Read total sizes for each hash and data vector
    uint32_t phSize = 0, chSize = 0, mrSize = 0, dSize = 0;
    std::memcpy(&phSize, buffer + offset, sizeof(phSize));
    offset += sizeof(phSize);
    std::memcpy(&chSize, buffer + offset, sizeof(chSize));
    offset += sizeof(chSize);
    std::memcpy(&mrSize, buffer + offset, sizeof(mrSize));
    offset += sizeof(mrSize);
    std::memcpy(&dSize, buffer + offset, sizeof(dSize));
    offset += sizeof(dSize);

    // Read block height, version number, timestamp, and block size
    unsigned int blockHeight = 0;
    float versionNum = 0.0f;
    unsigned long long timestamp = 0;
    uint32_t blockSize = 0;

    std::memcpy(&blockHeight, buffer + offset, sizeof(blockHeight));
    offset += sizeof(blockHeight);
    std::memcpy(&versionNum, buffer + offset, sizeof(versionNum));
    offset += sizeof(versionNum);
    std::memcpy(&timestamp, buffer + offset, sizeof(timestamp));
    offset += sizeof(timestamp);
    std::memcpy(&blockSize, buffer + offset, sizeof(blockSize));
    offset += sizeof(blockSize);

    // Read prevHash, currHash, merkleRoot
    std::vector<uint8_t> prevHash(phSize);
    std::memcpy(prevHash.data(), buffer + offset, phSize);
    offset += phSize;

    std::vector<uint8_t> currHash(chSize);
    std::memcpy(currHash.data(), buffer + offset, chSize);
    offset += chSize;

    std::vector<uint8_t> merkleRoot(mrSize);
    std::memcpy(merkleRoot.data(), buffer + offset, mrSize);
    offset += mrSize;

    // Read transactions (data)
    std::vector<transactions> tempdata;
    for (size_t i = 0; i < dNum; ++i) {
        transactions t = t.deserialize(buffer + offset);
        tempdata.push_back(t);
        offset += txSizes[i];
    }

    // Create and populate Block
    Block* block = new Block(tempdata, std::move(prevHash), versionNum, blockHeight, timestamp);

    /* Current Hash & Merkle Root Equality Check */
    uint32_t newBlockSiz = block->getSize();
    std::vector<uint8_t> newCurrHs = block->getCurrHash();
    std::vector<uint8_t> newMerk = block->getMerkleRoot();

    if (newMerk == merkleRoot && newCurrHs == currHash) {
        if (newBlockSiz == blockSize) {
            return block;
        }
        else {
            std::cout << "Block Size Does Not Match\n";
        }
    }
    else {
        std::cout << "Block Size Does Not Match\n";
    }
}
