/*-- Network.cpp------------------------------------------------------------
   This file implements Network & Server member functions.
---------------------------------------------------------------------------*/
#include "Network.h"

void Network::PingServer() {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerPing;

    // Caution with this...
    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

    msg << timeNow;
    Send(msg);
}

void  Network::maintainance() {
    /* Update Loop */
    uploop = std::thread(&Network::upLoop, this);
    uploop.detach();
}

void Network::upLoop() {
    bool bQuit = false;
    while (!bQuit)
    {
        if (IsConnected())
        {
            if (!Incoming().empty())
            {


                auto msg = Incoming().pop_front().msg;

                switch (msg.header.id)
                {
                case CustomMsgTypes::ServerAccept:
                {
                    /* Server has responded to a ping request */
                    std::cout << "Server Accepted Connection\n";
                }
                break;
                case CustomMsgTypes::ServerStart:
                {
                    unsigned char* body;
                    std::memcpy(&body, msg.body.data(), msg.body.size());
                    nodeID.push_back(deserializeStruct(body));
                }
                break;
                case CustomMsgTypes::ServerPing:
                {
                    /* Server has responded to a ping request */
                    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                    std::chrono::system_clock::time_point timeThen;
                    msg >> timeThen;
                    std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
                }
                break;

                case CustomMsgTypes::ServerMessage:
                {
                    /* Server has responded to a ping request */
                    uint32_t clientID;
                    msg >> clientID;
                    std::cout << "Hello from [" << clientID << "]\n";
                }
                break;
                case CustomMsgTypes::ClientUpdate:
                {
                    unsigned char* body;
                    std::memcpy(&body, msg.body.data(), msg.body.size());
                    transactions txts = txts.deserialize(body); //***
                    w1.inUTXO(txts);
                }
                break;
                }
            }
        }
        else
        {
            std::cout << "Server Down\n";
            bQuit = true;
        }
    }
}

void Network::MessageAll() {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::MessageAll;
    Send(msg);
}

void Network::ServerStart(unsigned char* sid) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerStart;
    msg << sid;
    Send(msg);
}

void Network::sendUTXO(const utxout& uout) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::TxRecieved;
    msg << w1.serialize_utxout(uout);
    Send(msg);
}

void Network::sendBlk(Block* block) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::BlkRecieved;
    msg << block->serialize();
    Send(msg);
}

void Network::connectWallet() {
    olc::net::message<CustomMsgTypes> msg;
    walletInfo swi;
    swi.clientID = m_connection->GetID();
    swi.pubKeyy = w1.getPubKey();
    swi.walladdr = w1.getWalletAddr();
    msg.header.id = CustomMsgTypes::WalletInfo;
    msg << serializeWalletInfo(swi);
    Send(msg);
}

/* Serialize Server ID */
unsigned char* Network::serializeStruct(const servID& sid) {

    /* Calculate the total size needed: size of portNum + size of host string + size of host size */
    size_t tSize = 0;
    size_t hSize = sid.host.size() * sizeof(char);
    size_t pnSize = sizeof(size_t);
    tSize += sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + hSize + pnSize;

    /* Allocate memory for the serialized data */
    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    /* serialization */

    /* Serialize tSize itself */
    std::memcpy(buffer + offset, &tSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize hSize itself */
    std::memcpy(buffer + offset, &hSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize pnSize itself */
    std::memcpy(buffer + offset, &pnSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize host itself */
    std::memcpy(buffer + offset, &sid.host, hSize);
    offset += hSize;

    /* Serialize Port Number itself */
    std::memcpy(buffer + offset, &sid.portNum, pnSize);
    offset += pnSize;

    return buffer;
}

/* Serialize Server ID */
servID Network::deserializeStruct(const unsigned char* buffer) {
    servID sid;
    size_t offset = 0;

    /* Deserialize tSize (not used in this function, but read to advance the offset) */
    size_t tSize;
    std::memcpy(&tSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize hSize */
    size_t hSize;
    std::memcpy(&hSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize pnSize */
    size_t pnSize;
    std::memcpy(&pnSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize host */
    sid.host.resize(hSize / sizeof(char));
    std::memcpy(&sid.host[0], buffer + offset, hSize);
    offset += hSize;

    /* Deserialize Port Number */
    std::memcpy(&sid.portNum, buffer + offset, pnSize);
    offset += pnSize;

    return sid;
}


/* Serialization Methods */
// serialize
unsigned char* Network::serializeWalletInfo(const walletInfo& info) {

    /* Calculate the size required for serialization: */
    size_t tSize = 0;

    // Size of clientID (uint32_t), walladdr length (size_t), walladdr content, and public key length */
    size_t clientSize = sizeof(size_t);
    size_t addrSize = info.walladdr.size() * sizeof(char);
    size_t pubKeySize = (info.pubKeyy != nullptr) ? i2d_PUBKEY(info.pubKeyy, nullptr) : 0;
    tSize += sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + clientSize + addrSize + pubKeySize;

    /* Allocate buffer for serialization */
    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    /* serialize utxout */

    /* Serialize tSize itself */
    std::memcpy(buffer + offset, &tSize, sizeof(tSize));
    offset += sizeof(tSize);

    /* Serialize cID Size itself */
    std::memcpy(buffer + offset, &clientSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize addrSize Size itself */
    std::memcpy(buffer + offset, &addrSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize pubKeySize Size itself */
    std::memcpy(buffer + offset, &pubKeySize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize clientID itself */
    std::memcpy(buffer + offset, &info.clientID, clientSize);
    offset += clientSize;

    /* Serialize WallAddress itself */
    std::memcpy(buffer + offset, &info.walladdr, addrSize);
    offset += addrSize;

    /* Serialize pubKeyy (EVP_PKEY*) */
    if (info.pubKeyy != nullptr) {
        unsigned char* tempPtr = buffer + offset;
        i2d_PUBKEY(info.pubKeyy, &tempPtr);
        offset += pubKeySize;
    }

    return buffer;
}

/* Deserialize Wallet info */
walletInfo Network::deserializeWalletInfo(const unsigned char* buffer) {
    walletInfo info;
    size_t offset = 0;

    /* Deserialize tSize (not used in this function, but read to advance the offset) */
    size_t tSize;
    std::memcpy(&tSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize clientSize */
    size_t clientSize;
    std::memcpy(&clientSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize addrSize */
    size_t addrSize;
    std::memcpy(&addrSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize pubKeySize */
    size_t pubKeySize;
    std::memcpy(&pubKeySize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize clientID */
    std::memcpy(&info.clientID, buffer + offset, clientSize);
    offset += clientSize;

    /* Deserialize WallAddress */
    info.walladdr.resize(addrSize / sizeof(char));
    std::memcpy(&info.walladdr[0], buffer + offset, addrSize);
    offset += addrSize;

    /* Deserialize pubKeyy (EVP_PKEY*) */
    if (pubKeySize > 0) {
        const unsigned char* tempPtr = buffer + offset;
        info.pubKeyy = d2i_PUBKEY(nullptr, &tempPtr, pubKeySize);
        offset += pubKeySize;
    }
    else {
        info.pubKeyy = nullptr;
    }

    return info;
}


/* Server Setup */
void Server::serverOnStart(Server& server) {
    /* Setting initial Server Node */
    servID initialNode;
    initialNode.host = "99.105.19.8";
    initialNode.portNum = 50507;
    setNodeID(initialNode);
    setTimeCreated();
    setServerID();

    /* Server STartup Logic */
    for (int i = 0; i < nodeID.size(); i++) {
        if (serverID.host == nodeID[i].host && serverID.portNum == nodeID[i].portNum) {
            chain = new BlockChain();
            std::cout << "Chain Created!\n";

            /* Update Loop */
            uploop = std::thread(&Server::upLoop, this, std::ref(server));
            uploop.detach();
        }
        else
        {
            /* Define Server's Client Connection */
            n.Connect(nodeID[0].host, nodeID[0].portNum);
            n.ServerStart(serializeStruct(serverID));

            /* Update Loop */
            uploop = std::thread(&Server::upLoop, this, std::ref(server));
            uploop.detach();
        }
    }
}

/* Update loop keeps server busy listening for connections */
void Server::upLoop(Server& server) {
    /* update loop */
    while (true)
    {
        /* Lock mutex for the update operation */
        std::lock_guard<std::mutex> lock(mtx);
        clientUpLoop();
        updateSlot();
        updateWallets();
        server.Update(-1, true);  // Call server update in a loop

        //only initial updates slot
        //if(serverID.host == )
    }
}

/* Add a ServerID JSON object to the Node vector */
void Server::setNodeID(const servID& sid) {
    // Ensure the JSON object is valid if needed
    // You can also perform validation checks here
    nodeID.push_back(sid);
}

/* Time of Server Creation */
void Server::setTimeCreated() {
    created = u.TimeStamp();
}

/* Callback Helper function to handle the data returned from curl */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

/* Get public IP address of server */
void Server::setServerID() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    /* Initialize curl */
    curl = curl_easy_init();
    if (curl) {
        /* Set the URL for the IPify API to get the public IP */
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org?format=text");

        /* Set the callback function to handle the data received */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        /* Pass the string buffer to store the result */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        /* Perform the request */
        res = curl_easy_perform(curl);

        /* Check if the request was successful */
        if (res != CURLE_OK) {
            std::cerr << "Curl request failed: " << curl_easy_strerror(res) << std::endl;
        }

        /* Cleanup */
        curl_easy_cleanup(curl);
    }
    /* Set Server ID */
    serverID.host = readBuffer;
    serverID.portNum = port;
}

void Server::clientUpLoop() {
    if (n.IsConnected()) {
        /* Retrieve messages from the client's message queue */
        while (!n.Incoming().empty()) {
            /* Get the message from the client's queue */
            auto msg = n.Incoming().pop_front();

            /* Add it to the server's message queue */
            m_qMessagesIn.push_back(msg);
        }
    }
}

void Server::updateSlot() {
    unsigned long long timeNow = u.TimeStamp();

    if ((timeNow - chain->getTimestamp()) % sPeriod) {
        chain->updateChnSlot();
        /* Create a random number generator */
        if (serverID.host == nodeID[0].host && serverID.portNum == nodeID[0].portNum) {
            blkRqMethod();
            verifyMempool();
            std::cout << "blk & vmem\n";
            if (!nodeID.empty()) {
                for (int i = 1; i < nodeID.size(); i++) {
                    n.Connect(nodeID[i].host, nodeID[i].portNum);
                    n.sendBlk(chain->getCurrBlock());
                }
            }
        }
    }
}

void Server::verifyMempool() {
    for (int i = 0; i < mempool.size(); i++) {
        do {
            if (chain->isNewTxid(mempool[i].getTxid())) {
                continue;
            }
            else {
                mempool[i].setTxid(w1.calcTxid(u.TimeStamp()));
            }
        } while (!chain->isNewTxid(mempool[i].getTxid()));
    }
}

void Server::blkRqMethod() {
    std::vector<transactions> txs;
    for (int i = 0; i < mempool.size(); i++) {
        txs.push_back(mempool[i]);
    }

    transactions reward;
    reward.setTimeStamp(u.TimeStamp());
    reward.setTxid(u.toUnsignedChar(u.TimeStamp()));
    reward.setSendAddr(w1.getWalletAddr());
    reward.setSendPkey(w1.getPubKey());
    std::vector<std::string> ra;
    std::vector<EVP_PKEY*> rpk;
    std::vector<double> amm;
    ra.push_back(w1.getWalletAddr());
    rpk.push_back(w1.getPubKey());
    amm.push_back(X0017.getReward());
    reward.setRecieveAddr(ra);
    reward.setRecievePkeys(rpk);
    reward.setAmmount(amm);
    reward.setFee(0);
    txs.push_back(reward);

    chain->GenerateBlock(txs);
    updateCoins(reward);
}

void Server::updateWallets() {
    /* check wallets of clients & my wallet */
    std::vector<transactions> tx;
    for (int i = 0; i < wallets.size(); i++) {
        tx = chain->checkWallets(wallets[i].walladdr);
        for (int j = 0; j < tx.size(); j++) {
            if (!tx[j].getRecieveAddr().empty()) {
                olc::net::message<CustomMsgTypes> msg;
                msg.header.id = CustomMsgTypes::ClientUpdate;
                msg << tx[j].serialize();
                MessageClient(m_deqConnections[wallets[i].clientID], msg);
            }
            std::vector<std::string> es;
            tx[j].setRecieveAddr(es);
        }
    }

    tx = chain->checkWallets(w1.getWalletAddr());
    for (int i = 0; i < tx.size(); i++) {
        if (!tx[i].getRecieveAddr().empty()) {
            w1.inUTXO(tx[i]);
        }
    }
    std::cout << "Wallets Updated\n";
}


void Server::updateCoins(transactions rew) {
    double totus = 0;
    std::vector<double> amm = rew.getAmmount();

    for (int i = 0; i < amm.size(); i++) {
        totus += amm[i];
    }

    X0017.setTotalSupply(totus);
    X0017.setCircSupply(totus);
}

unsigned char* Server::serializeStruct(const servID& sid) {
    /* Calculate the total size needed: size of portNum + size of host string + size of host size */
    size_t outSize = sizeof(uint16_t) + sizeof(size_t) + sid.host.size();

    /* Allocate memory for the serialized data */
    unsigned char* buffer = new unsigned char[outSize];
    size_t offset = 0;

    /* Copy the port number */
    std::memcpy(buffer + offset, &sid.portNum, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    /* Serialize the size of the host string */
    size_t hostSize = sid.host.size();
    std::memcpy(buffer + offset, &hostSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Copy the host string data */
    std::memcpy(buffer + offset, sid.host.data(), hostSize);

    return buffer;
}

servID Server::deserializeStruct(const unsigned char* buffer) {
    servID sid;
    size_t offset = 0;

    /* Deserialize the port number */
    std::memcpy(&sid.portNum, buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    /* Deserialize the size of the host string */
    size_t hostSize;
    std::memcpy(&hostSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Resize the host string and copy the data */
    sid.host.resize(hostSize);
    std::memcpy(&sid.host[0], buffer + offset, hostSize); // Use &sid.host[0] for string data

    return sid;
}
