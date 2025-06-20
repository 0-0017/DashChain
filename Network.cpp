/*-- Network.cpp------------------------------------------------------------
   This file implements Network & Server member functions.
---------------------------------------------------------------------------*/
#include "Network.h"

/* Server Setup */
void Peer::serverOnStart(Peer& server) {
    /* Setting initial Server Node */
    created = util::TimeStamp();
    sPeriod = 15;
    chain = nullptr;
    servID initialNode;
    initialNode.host = "34.44.200.9"; /* 99.105.19.8 */
    initialNode.portNum = 50507;
    setNodeID(initialNode);
    setTimeCreated();
    setServerID();

    /* Server Startup Logic */
    for (int i = 0; i < nodeID.size(); i++) {
        if (serverID.host == nodeID[i].host && serverID.portNum == nodeID[i].portNum) {
            chain = new BlockChain();
            chain->initial();
            std::cout << "Chain Created!\n";

            /* Prepare D-POS Consensus */
            consensus.setTimestamp(created);
            if (auto delegates = consensus.getDelegates(); delegates.empty()) {
                std::tuple<bool, std::string> ret = consensus.requestDelegate(w1.getBalance());
                if (get<0>(ret) == true) {
                    delegateID = get<1>(ret);
                    broadcastDelegateID(delegateID);
                }
                else {
                    std::cerr << "Failed to request delegate!\n";
                }
            }

            std::tuple<std::string, std::string, float> initialVote(w1.getWalletAddr(), delegateID, 0);
            std::vector<std::tuple<std::string, std::string, float>> iv_Vector;
            iv_Vector.emplace_back(initialVote);
            vote(iv_Vector);
            consensus.updateDelegates();
            consensus.setVotingPeriod(3600);
            util::logCall("NETWORK", "serverOnStart(Initial)", true);

            /* Server Threads */
            tMsg = std::thread(&Peer::msgLoop, this, std::ref(server));
            tBlk = std::thread(&Peer::blkLoop, this, std::ref(server));
            tCns = std::thread(&Peer::cnsLoop, this, std::ref(server));
            tMsg.detach();
            tBlk.detach();
            tCns.detach();

        }
        else
        {
            /* Connect To Initial Node */
            chain = new BlockChain();
            ConnectTo(nodeID[i].host, nodeID[i].portNum);

            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::ServerStart;
            msg << serializeStruct(serverID);
            SendToPeer(m_connections.front(), msg);
            util::logCall("NETWORK", "serverOnStart(PEER)", true);

            /* Server Threads */
            tMsg = std::thread(&Peer::msgLoop, this, std::ref(server));
            tBlk = std::thread(&Peer::blkLoop, this, std::ref(server));
            tCns = std::thread(&Peer::cnsLoop, this, std::ref(server));
            tMsg.detach();
            tBlk.detach();
            tCns.detach();
        }
    }
}

void Peer::msgLoop(Peer& server) {
    util::logCall("NETWORK", "msgLoop()", true);
    while (true)
    {
        /* Lock mutex for the update operation */
        std::lock_guard<std::mutex> lock(mtxA);
        server.Update(-1, true);  // Call server update in a loop
    }
}

void Peer::blkLoop(Peer& server) {
    util::logCall("NETWORK", "blkLoop()", true);
    while (true)
    {
        if (!chain->empty()) {
            /* Lock mutex for the update operation */
            std::lock_guard<std::mutex> lock(mtxB);
            unsigned long long timestamp = util::TimeStamp();

            if ((timestamp - chain->getCurrBlock()->getTimestamp()) >= 15) {
                currentDelegate = consensus.getCurrentDelegate();
                if (currentDelegate == delegateID) {
                    blkRqMethod();
                }
            }
        }
    }
}

void Peer::cnsLoop(Peer& server) {
    util::logCall("NETWORK", "cnsLoop()", true);
    while (true) {
        /* Lock mutex for the update operation */
        std::lock_guard<std::mutex> lock(mtxC);
        consensus.updateDelegates();
    }
}

/* Add a ServerID JSON object to the Node vector */
void Peer::setNodeID(const servID& sid) {
    // Ensure the JSON object is valid if needed
    // You can also perform validation checks here
    nodeID.push_back(sid);
    util::logCall("NETWORK", "setNodeID()", true);
}

/* Time of Server Creation */
void Peer::setTimeCreated() {
    created = util::TimeStamp();
    util::logCall("NETWORK", "setTimeCreated()", true);
}

/* Get public IP address of server */
void Peer::setServerID() {
    try
    {
        asio::io_context io_context;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::socket socket(io_context);

        // Connect to api.ipify.org:80
        auto endpoints = resolver.resolve("api.ipify.org", "80");
        asio::connect(socket, endpoints);

        // Send GET request
        std::string request =
            "GET / HTTP/1.1\r\n"
            "Host: api.ipify.org\r\n"
            "Connection: close\r\n\r\n";
        asio::write(socket, asio::buffer(request));

        // Read status line
        asio::streambuf response_buf;
        asio::read_until(socket, response_buf, "\r\n");
        std::istream response_stream(&response_buf);
        std::string http_version;
        unsigned int status_code;
        std::string status_message;
        response_stream >> http_version >> status_code;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/" || status_code != 200) {
            util::logCall("NETWORK", "setServerID()", false, "Error status");
            throw std::runtime_error("Bad response from server");
        }

        // Skip headers
        asio::read_until(socket, response_buf, "\r\n\r\n");
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {}

        // Read the response body (IP)
        std::ostringstream ip_data;
        ip_data << &response_buf;
        asio::error_code ec;
        while (asio::read(socket, response_buf, ec)) {
            ip_data << &response_buf;
        }
        if (ec != asio::error::eof) {
            util::logCall("NETWORK", "setServerID()", false, "Error Getting IP");
            throw asio::system_error(ec);
        }

        // Assign to class member
        serverID.host = ip_data.str();
        serverID.portNum = port;
        util::logCall("NETWORK", "setServerID()", true);
    }
    catch (const std::exception& e) {
        util::logCall("NETWORK", "setServerID()", false, "Error Getting IP");
        std::cerr << "Error getting public IP: " << e.what() << std::endl;
        serverID.host = "0.0.0.0"; // fallback or sentinel value
        serverID.portNum = port;
    }
}

void Peer::updateSlot() {
    chain->updateChnSlot();
}

void Peer::verifyMempool() {
    std::vector<transactions> new_mempool;
    for (int i = 0; i < mempool.size(); i++) {
        if (chain->isNewTxid(mempool[i].getTxid())) {
            new_mempool.emplace_back(mempool[i]);
        }
        else {
            continue;
        }
    }
    mempool = std::move(new_mempool);
    mempool.shrink_to_fit();
    util::logCall("NETWORK", "verifyMempool()", true);
}

void Peer::blkRqMethod() {
    std::vector<transactions> txs;

    /* Reward For Miner */
    std::vector<std::string> ra;
    std::vector<EVP_PKEY_ptr> rpk;
    std::vector<double> amm;
    std::vector<std::string> delegates = consensus.getDelegates();
    std::vector<std::string> delegateID = consensus.getDelegateIDs();
    std::vector<std::tuple<std::string, std::string, float>> votesQueue = consensus.getVotesQueue();
    ra.push_back(w1.getWalletAddr());
    amm.push_back(X0017.getReward());

    transactions reward(w1.getWalletAddr(), ra, amm, 0.0, w1.getLockTime(), w1.getVersion(), delegates,
        delegateID, votesQueue);
    txs.emplace_back(reward);

    /* Add confirmed transactions To Block */
    for (int i = 0; i < mempool.size(); i++) {
        txs.push_back(mempool[i]);
    }

    /* Generate Block And Add To Network */
    chain->GenerateBlock(txs);
    updateCoins(reward); //************
    verifyMempool();
    broadcastBlock(chain->getCurrBlock());
    util::logCall("NETWORK", "blkRqMethod()", true);
}

void Peer::updateWallets() {
    /* check wallets of clients & my wallet */
    std::vector<transactions> tx;
    tx = chain->checkWallets(w1.getWalletAddr());
    for (int k = 0; k < tx.size(); k++) {
        if (!tx[k].getRecieveAddr().empty()) {
            w1.inUTXO(tx[k]);
        }
    }
    std::cout << "Wallets Updated\n";
    util::logCall("NETWORK", "updateWallets()", true);
}

double Peer::getBalance() const {
    return w1.getBalance();
}

std::string Peer::getWalletAddress() const {
    return w1.getWalletAddr();
}

void Peer::listTx() {
    w1.listTxs();
}

bool Peer::sendTx(std::vector<std::string>& recipients, std::vector<double> amounts) {
    std::vector<std::string> delegates;
    std::vector<std::string> delegateID;
    std::vector<std::tuple<std::string, std::string, float>> votesQueue;
    utxout u1 = w1.outUTXO(X0017.getTxFee(), recipients, amounts, delegates, delegateID, votesQueue);
    broadcastTransaction(u1);

    util::logCall("NETWORK", "sendTx()", true);
    return true;
}

void Peer::getKnownTx(std::string& txid) {
    chain->getTx(txid).display();
}

void Peer::currBlockInfo() {
    chain->display();
}

void Peer::getBlock(unsigned int height) {
    chain->getBlock(height);
}

void Peer::updateCoins(transactions rew) {
    double totus = 0;
    std::vector<double> amm = rew.getAmmount();

    for (int i = 0; i < amm.size(); i++) {
        totus += amm[i];
    }

    X0017.setTotalSupply(totus);
    X0017.setCircSupply(totus);
    util::logCall("NETWORK", "updateCoins()", true);
}

/* Vote For Delegates */
void Peer::vote(std::vector<std::tuple<std::string, std::string, float>> votes) {
    consensus.updatedVotes(votes);
    broadcastVotes(votes);
    util::logCall("NETWORK", "vote()", true);
}

std::string Peer::requestDelegate(){
    if (std::tuple<bool, std::string> ret = consensus.requestDelegate(w1.getBalance()); std::get<0>(ret) == true) {
        delegateID = std::get<1>(ret);
        broadcastDelegateID(delegateID);
        util::logCall("NETWORK", "requestDelegate()", true);
        return std::get<1>(ret);
    }
    else {
        util::logCall("NETWORK", "requestDelegate()", false, "Balance Too Low");
        std::cout << "Balance Too Low\n";
        return "";
    }
}

// Initiates an outbound connection to the peer at the specified host and port.
bool Peer::ConnectTo(const std::string& host, uint16_t port) {
    std::cout << "[NetworkManager] Connecting to " << host << ":" << port << "\n";
    util::logCall("NETWORK", "ConnectTo()", true);
    return this->ConnectToPeer(host, port);
}

// Broadcasts a chat message to all connected peers.
void Peer::BroadcastChat(const std::string& text) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ChatMessage;

    // Use the overloaded operator<< to serialize the text into the message.
    size_t offset = 0;
    size_t datasize = text.size() * sizeof(char);
    size_t tSize = sizeof(size_t) + sizeof(size_t) + datasize;
    unsigned char* buffer = new unsigned char[tSize];
    std::memcpy(buffer + offset, &tSize, sizeof(size_t)); offset += sizeof(size_t);
    std::memcpy(buffer + offset, &datasize, sizeof(size_t)); offset += sizeof(size_t);
    std::memcpy(buffer + offset, text.data(), datasize);
    msg << buffer;

    // Broadcast the message using the base class function.
    util::logCall("NETWORK", "BroadcastChat()", true);
    this->Broadcast(msg);
}
void Peer::broadcastNode(unsigned char* sid) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::KnownNode;
    msg << sid;

    // Broadcast the message using the base class function.
    util::logCall("NETWORK", "broadcastNode()", true);
    this->Broadcast(msg);
}

void Peer::broadcastBlock(const Block* block) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::BlkRecieved;
    msg << block->serialize();

    // Broadcast the message using the base class function.
    util::logCall("NETWORK", "broadcastBlock()", true);
    this->Broadcast(msg);
}

void Peer::broadcastTransaction(const utxout& u_out) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::TxRecieved;
    msg << w1.serialize_utxout(u_out);

    // Broadcast the message using the base class function.
    util::logCall("NETWORK", "broadcastTransaction()", true);
    this->Broadcast(msg);
}

void Peer::broadcastDelegateID(const std::string& id) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::DelegateID;

    size_t offset = 0;
    size_t datasize = id.size() * sizeof(char);
    size_t tSize = sizeof(size_t) + sizeof(size_t) + datasize;
    unsigned char* buffer = new unsigned char[tSize];
    std::memcpy(buffer + offset, &tSize, sizeof(size_t)); offset += sizeof(size_t);
    std::memcpy(buffer + offset, &datasize, sizeof(size_t)); offset += sizeof(size_t);
    std::memcpy(buffer + offset, id.data(), datasize);
    msg << buffer;

    // Broadcast the message using the base class function.
    util::logCall("NETWORK", "broadcastDelegateID()", true);
    this->Broadcast(msg);
}

void Peer::broadcastVotes(std::vector<std::tuple<std::string, std::string, float>> votes) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::Votes;
    msg << Consensus::serializeVector(votes);

    // Broadcast the message using the base class function.
    util::logCall("NETWORK", "broadcastVotes()", true);
    this->Broadcast(msg);
}

unsigned char* Peer::serializeStruct(const servID& sid) {
    size_t hostSize = sid.host.size() + 1;  // include null terminator
    size_t outSize = sizeof(size_t) + sizeof(size_t) + sizeof(uint16_t) + hostSize;

    unsigned char* buffer = new unsigned char[outSize];
    size_t offset = 0;

    // Copy hostSize
    std::memcpy(buffer + offset, &outSize, sizeof(size_t));
    offset += sizeof(size_t);

    // Copy hostSize
    std::memcpy(buffer + offset, &hostSize, sizeof(size_t));
    offset += sizeof(size_t);

    // Copy port number
    std::memcpy(buffer + offset, &sid.portNum, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // Copy host string
    std::memcpy(buffer + offset, sid.host.c_str(), hostSize);

    util::logCall("NETWORK", "serializeStruct()", true);
    return buffer;
}

servID Peer::deserializeStruct(const unsigned char* buffer) {
    servID sid;
    size_t tSize;
    size_t hostSize = 0;
    size_t offset = 0;

    std::memcpy(&tSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&hostSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&sid.portNum, buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    sid.host = std::string(reinterpret_cast<const char*>(buffer + offset), hostSize - 1);  // exclude null terminator

    util::logCall("NETWORK", "deserializeStruct()", true);
    return sid;
}

/* Serialization Methods */
// serialize
unsigned char* Peer::serializeWalletInfo(const walletInfo& info) {

    /* Calculate the size required for serialization: */
    size_t tSize = 0;

    // Size of clientID (uint32_t), walladdr length (size_t), walladdr content, and public key length */
    size_t clientSize = sizeof(size_t);
    size_t addrSize = info.walladdr.size() * sizeof(char);
    size_t pubKeySize = (info.pubKeyy != nullptr) ? i2d_PUBKEY(info.pubKeyy.get(), nullptr) : 0;
    tSize += sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + clientSize + addrSize + pubKeySize;

    /* Allocate buffer for serialization */
    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    /* serialize utx-out */

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
        i2d_PUBKEY(info.pubKeyy.get(), &tempPtr);
        offset += pubKeySize;
    }

    util::logCall("NETWORK", "serializeWalletInfo()", true);
    return buffer;
}

/* Deserialize Wallet info */
walletInfo Peer::deserializeWalletInfo(const unsigned char* buffer) {
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
        EVP_PKEY* temp = d2i_PUBKEY(nullptr, &tempPtr, pubKeySize);
        info.pubKeyy.reset(temp, EVP_PKEY_Deleter());
        offset += pubKeySize;
    }
    else {
        info.pubKeyy = nullptr;
    }

    util::logCall("NETWORK", "deserializeWalletInfo()", true);
    return info;
}