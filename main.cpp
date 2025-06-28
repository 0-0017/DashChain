#include"main.h"

// Enum for command mapping
enum Command {
    HELP,
    BALANCE,
    TX_HISTORY,
    SEND_TX,
    GET_TX,
    BLOCKCHAIN_INFO,
    REQUEST_DELEGATE,
    GET_BLOCK,
    VOTE,
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
    {"request_delegate", REQUEST_DELEGATE},
    {"get_block", GET_BLOCK},
    {"vote", VOTE},
    {"exit", EXIT}
};

void displayHelp() {
    std::cout << "\nAvailable Commands:\n";
    std::cout << "  balance          - Get wallet balance\n";
    std::cout << "  tx_history       - Get transaction history\n";
    std::cout << "  send_tx          - Send transaction\n";
    std::cout << "  vote             - Vote for delegates\n";
    std::cout << "  get_tx [txid]    - Get specific transaction details\n";
    std::cout << "  blockchain_info  - Get blockchain details\n";
    std::cout << "  request_delegate - Get Delegate ID for voting\n";
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

void vote(Peer &p) {
    const double amount = p.getBalance();
    std::string sender = p.getWalletAddress();
    std::string ID = p.get_this_delID();
    float totalVotes = 0;
    std::vector<std::tuple<std::string, std::string, float>> votes;
    int numDelegates;

    if (ID == "error" || ID.empty()) {
        std::cout << "\nVote Failed: Invalid ID\n";
    }
    else{
        do {
            std::cout << "Enter the number of delegates to vote for: ";
            std::cin >> numDelegates;

            for (int i = 0; i < numDelegates; ++i) {
                std::string delegateID;
                float voteCount;

                std::cout << "Enter delegate ID: ";
                std::cin >> delegateID;

                std::cout << "Enter number of votes: ";
                std::cin >> voteCount;
                totalVotes += voteCount;

                if (totalVotes > amount) {
                    std::cout << " Balance Surpassed Please Revote \n";
                    votes.clear();
                    break;
                } else {
                    votes.emplace_back(sender, delegateID, voteCount);
                }
            }
        }while (totalVotes > amount);
    }

    p.vote(votes);

    // Display recorded votes
    std::cout << "\nRecorded Votes:\n";
    for (const auto& vote : votes) {
        std::cout << std::get<0>(vote) << " voted for " << std::get<1>(vote)
                  << " with " << std::get<2>(vote) << " votes.\n";
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

void requestDelegate(Peer &p) {
    const std::string delegateID = p.getWalletAddress();
    std::cout << "WARNING: SECURE DELEGATE ID...\n";
    std::cout << "\nDelegate ID: " << delegateID << "\n";
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
        case REQUEST_DELEGATE:
            requestDelegate(p);
            break;
        case GET_BLOCK:
            getSpecificBlock(p);
            break;
        case VOTE:
            vote(p);
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
