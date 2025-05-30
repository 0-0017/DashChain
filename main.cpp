#include"main.h"

// Enum for command mapping
enum Command {
    HELP,
    BALANCE,
    TX_HISTORY,
    SEND_TX,
    GET_TX,
    BLOCKCHAIN_INFO,
    GET_BLOCK,
    EXIT,
    UNKNOWN
};

// Command lookup table
std::unordered_map<std::string, Command> commandMap = {
    {"help", HELP},
    {"balance", BALANCE},
    {"tx_history", TX_HISTORY},
    {"send_tx", SEND_TX},
    {"get_tx", GET_TX},
    {"blockchain_info", BLOCKCHAIN_INFO},
    {"get_block", GET_BLOCK},
    {"exit", EXIT}
};

void displayHelp() {
    std::cout << "\nAvailable Commands:\n";
    std::cout << "  balance          - Get wallet balance\n";
    std::cout << "  tx_history       - Get transaction history\n";
    std::cout << "  send_tx          - Send transaction\n";
    std::cout << "  get_tx [txid]    - Get specific transaction details\n";
    std::cout << "  blockchain_info  - Get blockchain details\n";
    std::cout << "  get_block [num]  - Get specific block details\n";
    std::cout << "  connected_peers  - Get connected peers\n";
    std::cout << "  exit             - Terminate the program\n";
}

void getBalance(const Peer &p) {
    std::cout << "\nWallet Balance: %" << p.getBalance() << " Dash\n";
}

void getTransactionHistory(Peer &p) {
    std::cout << "\nTransaction History:\n";
    p.listTx();
}

void sendTransaction(Peer &p) {
    int numSenders;
    std::cout << "\nEnter number of senders: ";
    std::cin >> numSenders;

    std::vector<std::string> recipients;
    std::vector<double> amounts;

    for (int i = 0; i < numSenders; ++i) {
        std::string address;
        double amount;
        std::cout << "Sender " << (i + 1) << " - Enter wallet address: ";
        std::cin >> address;
        std::cout << "Enter amount to send: ";
        std::cin >> amount;

        recipients.push_back(address);
        amounts.push_back(amount);
    }

    if (p.sendTx(recipients, amounts)) {
        std::cout << "\nTransaction created & Processing" << "\n";
    }
    else {
        std::cout << "\nTransaction Failed Please Retry" << "\n";
    }
}

void getTransactionInfo(Peer &p) {
    std::string txid;
    std::cout << "\nEnter TXID: ";
    std::cin >> txid;

    p.getKnownTx(txid);
}

void getBlockchainInfo(Peer &p) {
    std::cout << "\nBlockchain Info:\n";
    p.currBlockInfo();
}

void getSpecificBlock(Peer &p) {
    int blockNum;
    std::cout << "\nEnter block number: ";
    std::cin >> blockNum;

    p.getBlock(blockNum);
}

void processCommand(std::string& command, Peer& p) {
    switch (commandMap.contains(command) ? commandMap[command] : UNKNOWN) {
        case HELP:
            displayHelp();
            break;
        case BALANCE:
            getBalance(p);
            break;
        case TX_HISTORY:
            getTransactionHistory(p);
            break;
        case SEND_TX:
            sendTransaction(p);
            break;
        case GET_TX:
            getTransactionInfo(p);
            break;
        case BLOCKCHAIN_INFO:
            getBlockchainInfo(p);
            break;
        case GET_BLOCK:
            getSpecificBlock(p);
            break;
        case EXIT:
            std::cout << "Terminating DashChain...\n";
            exit(0);
            break;
        default:
            std::cout << "Unknown command. Type 'help' for available commands.\n";
            break;
    }
}

int main() {
    /* Server Startup */
    Peer server(50507);
    server.serverOnStart(server);

    /* Welcome, Prompt */
    std::cout << "Welcome to DashChain!\n";
    std::cout << "DashChain is a Delegated Proof of Stake (DPOS) blockchain system.\n";
    std::cout << "Type 'help' to see available commands.\n";

    /* Process Commands */
    std::string command;
    while (true) {
        std::cout << "\n> ";
        std::cin >> command;
        processCommand(command, server);
    }
    return 0;
}
