/*-- Block.cpp------------------------------------------------------------
   This file implements node member functions.
-------------------------------------------------------------------------*/
#include "Block.h"
#include <cstddef>
#include <iostream>
using namespace std;


Block::Block(const std::vector<transactions>& d, std::vector<unsigned char> prevHash, float versionNum, unsigned int blockHeight, unsigned long long ts, Block* n)
    : data(d),
    head(ts, std::move(prevHash), versionNum, std::move(MerkleRoot(data))),
    blockHeight(blockHeight),
    blockSize(setSize()),
    currHash(setCurrHash())
{
    next = n;
    util::logCall("BLOCK", "Block()", true);
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

float Block::getVersion() const {
    return head.versionNum;
}

/* Merkle root of transactions */

std::vector<unsigned char> Block::MerkleRoot(const std::vector<transactions>& tx) const {
    /* Compute hashes for each transaction */
    std::vector<std::vector<unsigned char>> txHash;
    for (const auto& elem : tx) {
        std::string txHashStr(reinterpret_cast<char*>(elem.serialize()));
        if (std::vector<unsigned char> hash; util::shaHash(txHashStr, hash)) {
            txHash.emplace_back(std::move(hash));
        }
        else {
            util::logCall("BLOCK", "MerkleRoot()", false, "Failed to hash tx");
            std::cerr << "Failed to hash tx" << std::endl;
            return {};
        }
    }

    /* Ensure even number of hashes */
    if (txHash.size() % 2 != 0) {
        txHash.emplace_back(txHash.back());
    }

    /* Compute the Merkle root */
    while (txHash.size() > 1) {
        std::vector<std::vector<unsigned char>> newLevel;
        for (size_t i = 0; i < txHash.size(); i += 2) {
            // Convert hashes to a single string for hashing
            std::string combinedHashStr(txHash[i].begin(), txHash[i].end());
            combinedHashStr.append(txHash[i + 1].begin(), txHash[i + 1].end());

            if (std::vector<unsigned char> newHash; util::shaHash(combinedHashStr, newHash)) {
                newLevel.emplace_back(std::move(newHash));
            } else {
                util::logCall("BLOCK", "MerkleRoot()", false, "Failed to hash combined data");
                std::cerr << "Failed to hash combined data" << std::endl;
                return {};
            }
        }
        txHash = newLevel;
    }

    util::logCall("BLOCK", "MerkleRoot()", true);
    return txHash.front(); // Final Merkle Root
}

std::vector<unsigned char> Block::getMerkleRoot() const {
    return head.merkleRoot;
}

unsigned long long Block::setTimestamp(){
    return util::TimeStamp();
}

unsigned long long Block::getTimestamp() const {
    return head.timestamp;
}

unsigned int Block::getBlockHeight() const {
    return blockHeight;
}

std::vector<unsigned char> Block::setCurrHash() const{

    /* Block data used for current hash:clear
     * Data & Timestamp
     * Prev Hash & Version
     * Merkle, Height & Size
     */

    /* Data */
    size_t tSize = 0;
    /* Get Size of Each Transaction */
    for (const auto& tx : data) {
        size_t temp_size = 0;
        const unsigned char* tdata = tx.serialize();
        std::memcpy(&temp_size, tdata, sizeof(size_t));
        tSize += temp_size;
    }

    tSize += sizeof(unsigned long long) + sizeof(float) + sizeof(unsigned int) + sizeof(size_t);
    const std::vector<unsigned char> t_hash = getPrevHash();
    const std::vector<unsigned char> t_merkle = getMerkleRoot();
    tSize += (t_hash.size() * sizeof(unsigned char)) + (t_merkle.size() * sizeof(unsigned char));

    /* Add Each Transaction to Hash Data */
    unsigned char* h_data = new unsigned char[tSize];
    size_t offset = 0;
    for (const auto& tx : data) {
        size_t temp_size2 = 0;
        const unsigned char* tdata2 = tx.serialize();
        std::memcpy(&temp_size2, tdata2, sizeof(size_t));
        std::memcpy(h_data + offset, tdata2, temp_size2);
        offset += temp_size2;
    }

    /* Add All other data */
    size_t offset2 = offset;
    const unsigned long long t_ts = getTimestamp();
    std::memcpy(h_data + offset2, &t_ts, sizeof(unsigned long long));
    offset2 += sizeof(unsigned long long);

    std::memcpy(h_data + offset2, t_hash.data(), (t_hash.size() * sizeof(uint8_t)));
    offset2 += (t_hash.size() * sizeof(uint8_t));

    const float t_v = getVersion();
    std::memcpy(h_data + offset2, &t_v, sizeof(float));
    offset2 += sizeof(float);

    std::memcpy(h_data + offset2, t_merkle.data(), (t_merkle.size() * sizeof(uint8_t)));
    offset2 += (t_merkle.size() * sizeof(uint8_t));

    const unsigned int t_height = getBlockHeight();
    std::memcpy(h_data + offset2, &t_height, sizeof(unsigned int));
    offset2 += sizeof(unsigned int);

    const size_t t_bsize = getSize();
    std::memcpy(h_data + offset2, &t_bsize, sizeof(size_t));

    // tdata remains
    std::string msg(reinterpret_cast<char*>(h_data));
    std::vector<unsigned char> hash;
    if (util::shaHash(msg, hash)) {
        util::logCall("BLOCK", "setCurrHash()", true);
        return hash;
    }
    else {
        util::logCall("BLOCK", "setCurrHash()", false, "Failed to hash block");
        std::cerr << "Failed to hash block" << std::endl;
        return {};
    }
}

std::vector<unsigned char> Block::getCurrHash() const {
    return currHash;
}


std::vector<unsigned char> Block::getPrevHash() const {
    return head.prevHash;
}

std::vector<transactions> Block::getTxs() const {
    util::logCall("BLOCK", "getTxs()", true);
    return data;
}

std::vector<transactions> Block::getData() const {
    util::logCall("BLOCK", "getData()", true);
    return data;
}


size_t Block::setSize() const{
    size_t size = 0;
    size += data.size() + head.merkleRoot.size() + sizeof(unsigned long long) + head.prevHash.size() + sizeof(float) + sizeof(head);
    size += sizeof(size_t) + sizeof(unsigned int);
    util::logCall("BLOCK", "setSize()", true);
    return size;
}

size_t Block::getSize() const {
    return blockSize;
}

void Block::display() {
    std::cout << "===================================" << std::endl;
    std::cout << "|         Block Details           |" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "Timestamp       : " << util::toString(head.timestamp) << std::endl;
    std::cout << "Previous Hash   : " << util::toString(head.prevHash) << std::endl;
    std::cout << "Version         : " << util::toString(head.versionNum) << std::endl;
    std::cout << "Merkle Root     : " << util::toString(head.merkleRoot) << std::endl;
    std::cout << "Block Height    : " << util::toString(blockHeight) << std::endl;
    std::cout << "Block Size      : " << util::toString(blockSize) << " bytes" << std::endl;
    std::cout << "Current Hash    : " << util::toString(currHash) << std::endl;
    std::cout << "===================================" << std::endl;
}

/* Serialize method */
unsigned char* Block::serialize() const {

    /* Collect Total Sizes*/
    size_t tSize = 0; // total size
    /* Predetermined Sizes:  tSize, timestamp, version, blk height & blk size */
    tSize += sizeof(size_t) + sizeof(unsigned long long) + sizeof(float) + sizeof(unsigned int) + sizeof(size_t);

    size_t phSize = head.prevHash.size(); // previous hash size
    size_t mrSize = head.merkleRoot.size(); // merkle root size
    size_t chSize = currHash.size(); // current hash size

    /* collect total size of data vector & total vector */
    size_t dSize = 0;
    size_t doffset = 0;
    std::vector<unsigned char> tData;
    for (auto& tx: data) {
        unsigned char* temp = tx.serialize();
        size_t tempSize = 0;
        std::memcpy(&tempSize, temp, sizeof(size_t));
        dSize += tempSize;
        tData.resize(tData.size() + tempSize);
        std::memcpy(tData.data() + doffset, temp, tempSize);
        doffset += tempSize;
        temp = nullptr;
    }

    /* Calculate Total Size & create buffer */
    tSize += sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t);
    tSize += phSize + mrSize + chSize + dSize;
    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    /* Serialize Block */
    std::memcpy(buffer + offset, &tSize, sizeof(size_t)); //tSize
    offset += sizeof(size_t);

    std::memcpy(buffer + offset, &phSize, sizeof(size_t)); //phSize
    offset += sizeof(size_t);

    std::memcpy(buffer + offset, &mrSize, sizeof(size_t)); //mrSize
    offset += sizeof(size_t);

    std::memcpy(buffer + offset, &chSize, sizeof(size_t)); //chSize
    offset += sizeof(size_t);

    std::memcpy(buffer + offset, &dSize, sizeof(size_t)); //dSize
    offset += sizeof(size_t);

    std::memcpy(buffer + offset, &head.timestamp, sizeof(unsigned long long)); //timestamp
    offset += sizeof(unsigned long long);

    std::memcpy(buffer + offset, &head.versionNum, sizeof(float)); //version
    offset += sizeof(float);

    std::memcpy(buffer + offset, &blockHeight, sizeof(unsigned int)); //BlockHeight
    offset += sizeof(unsigned int);

    std::memcpy(buffer + offset, &blockSize, sizeof(size_t)); //blockSize
    offset += sizeof(size_t);

    std::memcpy(buffer + offset, head.prevHash.data(), phSize); //prevHash
    offset += phSize;

    std::memcpy(buffer + offset, head.merkleRoot.data(), mrSize); //merkleRoot
    offset += mrSize;

    std::memcpy(buffer + offset, currHash.data(), chSize); //currHash
    offset += chSize;

    std::memcpy(buffer + offset, tData.data(), dSize); //data

    util::logCall("BLOCK", "serialize()", true);
    return buffer;
}

/* Deserialize method
* Current Has And Merkle Root Checks for Integrity
*/
Block* Block::deserialize(const unsigned char* buffer) const {
    size_t offset = 0;

    /* Deserialize total Size */
    size_t tSize = 0;
    std::memcpy(&tSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize phSize */
    size_t phSize = 0;
    std::memcpy(&phSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize mrSize */
    size_t mrSize = 0;
    std::memcpy(&mrSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize chSize */
    size_t chSize = 0;
    std::memcpy(&chSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize dSize */
    size_t dSize = 0;
    std::memcpy(&dSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize timestamp */
    unsigned long long timestamp = 0;
    std::memcpy(&timestamp, buffer + offset, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    /* Deserialize version */
    float versionNum = 0;
    std::memcpy(&versionNum, buffer + offset, sizeof(float));
    offset += sizeof(float);

    /* Deserialize block Heigt */
    unsigned int blockHeight = 0;
    std::memcpy(&blockHeight, buffer + offset, sizeof(unsigned int));
    offset += sizeof(unsigned int);

    /* Deserialize block size */
    size_t blockSize = 0;
    std::memcpy(&blockSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize previous hash itself */
    std::vector<unsigned char> prevHash(phSize);
    std::memcpy(prevHash.data(), buffer + offset, phSize);
    offset += phSize;

    /* Deserialize merkle root itself */
    std::vector<unsigned char> merkleRoot(mrSize);
    std::memcpy(merkleRoot.data(), buffer + offset, mrSize);\
    offset += mrSize;

    /* Deserialize merkle root itself */
    std::vector<unsigned char> currHash(chSize);
    std::memcpy(currHash.data(), buffer + offset, chSize);
    offset += chSize;

    /* Deserialize Data */
    size_t doff = 0;
    std::vector<transactions> data;
    unsigned char* temp_data = new unsigned char[dSize];
    std::memcpy(temp_data, buffer + offset, dSize);

    while (doff < dSize) {
        size_t si = 0;
        std::memcpy(&si, temp_data + doff, sizeof(size_t));
        unsigned char* t_data = new unsigned char[si];
        std::memcpy(t_data, temp_data + doff, si);
        data.emplace_back(transactions::deserialize(t_data));
        t_data = nullptr;
        doff += si;
    }


    // Create and populate Block
    Block* block = new Block(data, std::move(prevHash), versionNum, blockHeight, timestamp);

    /* Current Hash & Merkle Root Equality Check */
    size_t newBlockSiz = block->getSize();
    std::vector<unsigned char> newCurrHs = block->getCurrHash();
    std::vector<unsigned char> newMerk = block->getMerkleRoot();

    if (newMerk == merkleRoot && newCurrHs == currHash) {
        if (newBlockSiz == blockSize) {
            util::logCall("BLOCK", "deserialize()", true);
            return block;
        }
        else {
            util::logCall("BLOCK", "deserialize()", false, "Block Size Does Not Match");
            std::cout << "Block Size Does Not Match\n";
            return nullptr;
        }
    }
    else {
        util::logCall("BLOCK", "deserialize()", false, "Block Size Does Not Match");
        std::cout << "Block Size Does Not Match\n";
        return nullptr;
    }
}