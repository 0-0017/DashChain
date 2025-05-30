//
// Created by king dash on 5/30/25.
//

#ifndef MAIN_H
#define MAIN_H

#include"BlockChain.h"
#include"Wallet.h"
#include"Network.h"

// Function declarations
void displayHelp();
void getBalance();
void getTransactionHistory();
void sendTransaction();
void getTransactionInfo();
void getBlockchainInfo();
void getSpecificBlock();
void processCommand(std::string& command, Peer& p);

#endif //MAIN_H
