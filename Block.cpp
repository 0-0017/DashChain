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
                std::cerr << "Failed to hash combined data" << std::endl;
                return {};
            }
        }
        txHash = newLevel;
    }

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
        return hash;
    }
    else {
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
    return data;
}

std::vector<transactions> Block::getData() const {
    return data;
}


size_t Block::setSize() const{
    size_t size = 0;
    size += data.size() + head.merkleRoot.size() + sizeof(unsigned long long) + head.prevHash.size() + sizeof(float) + sizeof(head);
    size += sizeof(size_t) + sizeof(unsigned int);
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
    // Initial header: blockHeight (unsigned int), versionNum (float),
    // timestamp (unsigned long-long), blockSize (uint32_t)
    size_t tSize = sizeof(unsigned int) + sizeof(float)
        + sizeof(unsigned long long) + sizeof(size_t);

    // Compute sizes for hash data:
    size_t phNum = head.prevHash.size();
    size_t chNum = currHash.size();
    size_t mrNum = head.merkleRoot.size();
    size_t phSize = phNum * sizeof(unsigned char);
    size_t chSize = chNum * sizeof(unsigned char);
    size_t mrSize = mrNum * sizeof(unsigned char);

    // Calculate serialized transactions
    size_t dSize = 0;
    size_t dNum = 0;
    std::vector<unsigned char*> stx;      // Serialized transactions
    std::vector<size_t> txSizes;            // Their sizes

    unsigned char* tdata = nullptr;
    for (const auto& tx : data) {
        tdata = tx.serialize();
        stx.push_back(tdata);

        // The first sizeof(size_t) bytes contain the serialized size
        size_t serSize = 0;
        std::memcpy(&serSize, tdata, sizeof(serSize));
        txSizes.push_back(serSize);
        dSize += serSize;
        dNum++;
        // tdata remains allocated
    }

    // Prepare txSizes field: size_t for tSizedNum + array for txSizes
    size_t txSizesNum = txSizes.size();
    size_t txSizedSize = txSizesNum * sizeof(size_t);

    // Add size for txSizes metadata and transaction data
    tSize = tSize + sizeof(size_t)      // For tSizedNum field
        + sizeof(size_t)      // For tSize field? (as originally intended)
        + txSizedSize + dSize;

    // Add size for 4 size_t fields (phNum, chNum, mrNum, dNum)
    tSize = tSize + 4 * sizeof(size_t);

    // Add size for 4 size_t fields (phSize, chSize, mrSize, dSize field)
    tSize = tSize + 4 * sizeof(size_t);

    // *** Add the sizes of the hash vectors ***
    tSize = tSize + phSize + chSize + mrSize;

    // Allocate memory for the entire buffer
    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    // Serialize fields in the proper order:
    std::memcpy(buffer + offset, &tSize, sizeof(tSize));
    offset += sizeof(tSize);

    std::memcpy(buffer + offset, &txSizesNum, sizeof(txSizesNum));
    offset += sizeof(txSizesNum);

    std::memcpy(buffer + offset, txSizes.data(), txSizedSize);
    offset += txSizedSize;

    std::memcpy(buffer + offset, &phNum, sizeof(phNum));
    offset += sizeof(phNum);

    std::memcpy(buffer + offset, &chNum, sizeof(chNum));
    offset += sizeof(chNum);

    std::memcpy(buffer + offset, &mrNum, sizeof(mrNum));
    offset += sizeof(mrNum);

    std::memcpy(buffer + offset, &dNum, sizeof(dNum));
    offset += sizeof(dNum);

    std::memcpy(buffer + offset, &phSize, sizeof(phSize));
    offset += sizeof(phSize);

    std::memcpy(buffer + offset, &chSize, sizeof(chSize));
    offset += sizeof(chSize);

    std::memcpy(buffer + offset, &mrSize, sizeof(mrSize));
    offset += sizeof(mrSize);

    std::memcpy(buffer + offset, &dSize, sizeof(dSize));
    offset += sizeof(dSize);

    std::memcpy(buffer + offset, &blockHeight, sizeof(blockHeight));
    offset += sizeof(blockHeight);

    std::memcpy(buffer + offset, &head.versionNum, sizeof(head.versionNum));
    offset += sizeof(head.versionNum);

    std::memcpy(buffer + offset, &head.timestamp, sizeof(head.timestamp));
    offset += sizeof(head.timestamp);

    std::memcpy(buffer + offset, &blockSize, sizeof(blockSize));
    offset += sizeof(blockSize);

    std::memcpy(buffer + offset, head.prevHash.data(), phSize);
    offset += phSize;

    std::memcpy(buffer + offset, currHash.data(), chSize);
    offset += chSize;

    std::memcpy(buffer + offset, head.merkleRoot.data(), mrSize);
    offset += mrSize;

    /* Serialize each transaction�s data */
    for (size_t i = 0; i < stx.size(); ++i) {
        std::memcpy(buffer + offset, stx[i], txSizes[i]);
        offset += txSizes[i];
        delete[] stx[i];
    }

    return buffer;
}

/* Deserialize method
* Current Has And Merkle Root Checks for Integrity
*/
Block* Block::deserialize(const unsigned char* buffer) const {
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
    size_t phNum = 0, chNum = 0, mrNum = 0, dNum = 0;
    std::memcpy(&phNum, buffer + offset, sizeof(phNum));
    offset += sizeof(phNum);
    std::memcpy(&chNum, buffer + offset, sizeof(chNum));
    offset += sizeof(chNum);
    std::memcpy(&mrNum, buffer + offset, sizeof(mrNum));
    offset += sizeof(mrNum);
    std::memcpy(&dNum, buffer + offset, sizeof(dNum));
    offset += sizeof(dNum);

    // Read total sizes for each hash and data vector
    size_t phSize = 0, chSize = 0, mrSize = 0, dSize = 0;
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
    size_t blockSize = 0;

    std::memcpy(&blockHeight, buffer + offset, sizeof(blockHeight));
    offset += sizeof(blockHeight);
    std::memcpy(&versionNum, buffer + offset, sizeof(versionNum));
    offset += sizeof(versionNum);
    std::memcpy(&timestamp, buffer + offset, sizeof(timestamp));
    offset += sizeof(timestamp);
    std::memcpy(&blockSize, buffer + offset, sizeof(blockSize));
    offset += sizeof(blockSize);

    // Read prevHash, currHash, merkleRoot
    std::vector<unsigned char> prevHash(phSize);
    std::memcpy(prevHash.data(), buffer + offset, phSize);
    offset += phSize;

    std::vector<unsigned char> currHash(chSize);
    std::memcpy(currHash.data(), buffer + offset, chSize);
    offset += chSize;

    std::vector<unsigned char> merkleRoot(mrSize);
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
    size_t newBlockSiz = block->getSize();
    std::vector<unsigned char> newCurrHs = block->getCurrHash();
    std::vector<unsigned char> newMerk = block->getMerkleRoot();

    if (newMerk == merkleRoot && newCurrHs == currHash) {
        if (newBlockSiz == blockSize) {
            return block;
        }
        else {
            std::cout << "Block Size Does Not Match\n";
            return nullptr;
        }
    }
    else {
        std::cout << "Block Size Does Not Match\n";
        return nullptr;
    }
}