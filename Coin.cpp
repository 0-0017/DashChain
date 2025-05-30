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
}

const char* Coin::getName() {
    return name;
}
const char* Coin::getSymbol() {
    return symbol;
}
double Coin::getTotalSupply() {
    return totalSupply;
}
void Coin::setTotalSupply(double ts) {
    totalSupply += ts;
}
double Coin::getCircSupply() {
    return circSupply;
}
void Coin::setCircSupply(double cs) {
    circSupply += cs;
}
float Coin::getReward() {
    return reward;
}
void Coin::setReward(float rw) {
    reward = rw;
}
unsigned short Coin::getBlockTime() {
    return block_time;
}
void Coin::setBlockTime(unsigned short bt) {
    block_time = bt;
}
double Coin::getTxFee() {
    return txFee;
}
void Coin::setTxFee(double txf) {
    txFee = txf;
}
uint32_t Coin::getBlkSzLimit() {
    return blkSzLimit;
}
void Coin::setBlkSzLimit(uint32_t bsl) {
    blkSzLimit = bsl;
}


//void Coin::updateTotalSupply(){}
//void Coin::updateCircSupply(){}
//transactions Coin::grantRewards(){}