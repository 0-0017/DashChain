/*-- Coin.cpp ------------------------------------------------------------
This file implements 0017 member functions.
-------------------------------------------------------------------------*/
#include "Coin.h"

Coin::Coin() {
    totalSupply = 0;
    block_time = 5;
    reward = 15;
    txFee = 0.5;
    circSupply = 0;
    blkSzLimit = 1000000;
    util::logCall("COIN", "Coin()", true);
}

const char* Coin::getName() {
    util::logCall("COIN", "getName()", true);
    return name;
}
const char* Coin::getSymbol() {
    util::logCall("COIN", "getSymbol()", true);
    return symbol;
}
double Coin::getTotalSupply() {
    util::logCall("COIN", "getTotalSupply()", true);
    return totalSupply;
}
void Coin::setTotalSupply(double ts) {
    util::logCall("COIN", "setTotalSupply()", true);
    totalSupply += ts;
}
double Coin::getCircSupply() {
    util::logCall("COIN", "getCircSupply()", true);
    return circSupply;
}
void Coin::setCircSupply(double cs) {
    util::logCall("COIN", "setCircSupply()", true);
    circSupply += cs;
}
float Coin::getReward() {
    util::logCall("COIN", "getReward()", true);
    return reward;
}
void Coin::setReward(float rw) {
    util::logCall("COIN", "setReward()", true);
    reward = rw;
}
unsigned short Coin::getBlockTime() {
    util::logCall("COIN", "getBlockTime()", true);
    return block_time;
}
void Coin::setBlockTime(unsigned short bt) {
    util::logCall("COIN", "setBlockTime()", true);
    block_time = bt;
}
double Coin::getTxFee() {
    util::logCall("COIN", "getTxFee()", true);
    return txFee;
}
void Coin::setTxFee(double txf) {
    util::logCall("COIN", "setTxFee()", true);
    txFee = txf;
}
uint32_t Coin::getBlkSzLimit() {
    util::logCall("COIN", "getBlkSzLimit()", true);
    return blkSzLimit;
}
void Coin::setBlkSzLimit(uint32_t bsl) {
    util::logCall("COIN", "setBlkSzLimit()", true);
    blkSzLimit = bsl;
}


//void Coin::updateTotalSupply(){}
//void Coin::updateCircSupply(){}
//transactions Coin::grantRewards(){}