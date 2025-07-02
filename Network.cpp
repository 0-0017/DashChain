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
    rcvCns = false;
    servID initialNode;
    initialNode.host = "99.105.19.8";
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
            tTrn = std::thread(&Peer::trnLoop, this, std::ref(server));
            tMsg.detach();
            tBlk.detach();
            tCns.detach();
            tTrn.detach();

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
            tTrn = std::thread(&Peer::trnLoop, this, std::ref(server));
            tMsg.detach();
            tBlk.detach();
            tCns.detach();
            tTrn.detach();
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
                    train_data();
                }
            }
        }
    }
}

void Peer::cnsLoop(Peer& server) {
    util::logCall("NETWORK", "cnsLoop()", true);
    while (rcvCns) {
        /* Lock mutex for the update operation */
        std::lock_guard<std::mutex> lock(mtxC);
        consensus.updateDelegates();
    }
}

void Peer::trnLoop(Peer& server) {
    unsigned int i = 0;
    unsigned int cbh = chain->getCurrBlock()->getBlockHeight();
    /* Lock mutex for the update operation */
    std::lock_guard<std::mutex> lock(mtxD);
    while (cbh > i && delegateID == currentDelegate) {
        train_data();
        i++;
    }
}

/* Add a ServerID JSON object to the Node vector */
void Peer::setNodeID(const servID& sid) {
    // Ensure the JSON object is valid if needed
    // You can also perform validation checks here
    nodeID.emplace_back(sid);
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
    for (auto& tx : mempool) {
        if (chain->isNewTxid(tx.getTxid())) {
            new_mempool.emplace_back(tx);
        }
        else {
            continue;
        }
    }
    mempool = std::move(new_mempool);
    mempool.shrink_to_fit();
}

void Peer::mempool_emplace(const utxout& uin) {
    transactions tx = transactions::deserialize(util::toUnsignedChar(uin.utxo));
    /* Verify if the transaction is valid */
    if (tx.inputsValid() && tx.outputsValid()) {
        /* check for double spend */
        if (chain->isNewTxid(tx.getTxid())) {
            if (w1.verifyTx(uin)) {
                if (tx.getRecieveAddr().size() == tx.getAmmount().size()) {
                    std::vector<std::string> txra = tx.getRecieveAddr();
                    std::vector<double> txam = tx.getAmmount();
                    std::vector<std::string> delegates = tx.getDelegates();
                    std::vector<std::string> delegateID = tx.getDelegatesID();
                    std::vector<std::tuple<std::string, std::string, float>> votesQueue = tx.getVotes();
                    mempool.emplace_back(tx);
                    verifyMempool();
                }
                else {
                    /* If the transaction Cant be verifies */
                    std::cout << "Addresses Mix Match!\n";
                }
            }
            else {
                /* If the transaction Cant be verifies */
                std::cout << "Transaction Cannot Be Verified!\n";
            }
        }
        else {
            /* If the transaction is on blockchain */
            std::cout << "Transaction Spent!\n";
        }

    }
    else {
        /* If the transaction inputs or outputs are invalid */
        std::cout << "Invalid transaction inputs or outputs!\n";
    }
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
    for (auto& tx : mempool) {
        txs.emplace_back(tx);
    }

    /* Generate Block And Add To Network */
    chain->GenerateBlock(txs);
    updateCoins(reward); //************
    verifyMempool();
    confirm();
    broadcastBlock(chain->getCurrBlock());
    util::logCall("NETWORK", "blkRqMethod()", true);
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
    mempool_emplace(u1);

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

std::string Peer::get_this_delID() const {
    return delegateID;
}

void Peer::set_address(const std::string& wa) {
    w1.setWalletAddr(wa);
}

void Peer::updateCoins(transactions rew) {
    double totus = 0;
    std::vector<double> amm = rew.getAmmount();

    for (double am : amm) {
        totus += am;
    }

    X0017.setTotalSupply(totus);
    X0017.setCircSupply(totus);
}

void Peer::confirm() {
    /* Get latest Confirmed Block */
    Block* confirmed = chain->confirmation();

    if (confirmed != nullptr) {
        if (!confirmed->getTxs().empty()) {
            std::vector<transactions> txs;
            txs = confirmed->getTxs();

            for (auto& tx : txs) {
                const size_t recSize = tx.getRecieveAddr().size();
                std::vector<std::string> rec;
                rec = tx.getRecieveAddr();

                if (recSize > 0) {
                    /* Update Wallet */
                    size_t pos = 0;
                    for (auto& ra : rec) {
                        if (w1.getWalletAddr() == ra) {
                            w1.inUTXO(tx, pos);
                        }
                        pos++;
                    }
                    rec.clear();
                }

                /* Update Consensus */
                size_t delSize = tx.getDelegates().size();
                size_t delIDSize = tx.getDelegatesID().size();
                size_t votesSize = tx.getVotes().size();
                std::vector<std::string> delegates;
                std::vector<std::string> delegateID;
                std::vector<std::tuple<std::string, std::string, float>> votesQueue;

                if (delSize > 0) {
                    delegates = tx.getDelegates();
                    consensus.setDelegates(delegates);
                    delegates.clear();
                }

                if (delIDSize > 0) {
                    delegateID = tx.getDelegatesID();
                    consensus.setDelegateIDs(delegateID);
                    delegateID.clear();
                }

                if (votesSize > 0) {
                    votesQueue = tx.getVotes();
                    consensus.updatedVotes(votesQueue);
                    votesQueue.clear();
                }
            }
        }
    }
}

/* Vote For Delegates */
void Peer::vote(std::vector<std::tuple<std::string, std::string, float>> votes) {
    consensus.updatedVotes(votes);
    broadcastVotes(votes);
}

std::string Peer::requestDelegate(){
    if (std::tuple<bool, std::string> ret = consensus.requestDelegate(w1.getBalance()); std::get<0>(ret) == true) {
        delegateID = std::get<1>(ret);
        broadcastDelegateID(delegateID);
        return std::get<1>(ret);
    }
    else {
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

void Peer::train_data() {
    /*Prepare data for load */
    std::vector<double> data;
    const double totalSupply = X0017.getTotalSupply();
    const double circSupply = X0017.getCircSupply();
    const double balance = w1.getBalance();
    const double periodVotes = static_cast<double>(consensus.getVotesQueue().size());
    const double height = chain->getBlockHeight();
    const double txVolume = static_cast<double>(chain->getCurrBlock()->getData().size());

    /* Load Data */
    data.push_back(totalSupply);
    data.push_back(circSupply);
    data.push_back(balance);
    data.push_back(periodVotes);
    data.push_back(height);
    data.push_back(txVolume);
    trainer.load_data(data);

    /* Get Predictions. NOTE: can be empty */
    std::vector<double> predictions = trainer.train();

    /* Apply Predictions */
    if (!predictions.empty()) {
        consensus.setMaxDelegates(static_cast<unsigned long>(predictions[0]));
        consensus.setWindowPeriod(static_cast<unsigned long>(predictions[1]));
        consensus.setVotingPeriod(static_cast<unsigned long>(predictions[2]));
        consensus.setDecayFactor(static_cast<float>(predictions[3]));
        consensus.setMinBalance(static_cast<float>(predictions[4]));
        sPeriod = static_cast<unsigned short>(predictions[5]);
    }
}

// Broadcasts a chat message to all connected peers.
void Peer::BroadcastChat(const std::string& text) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ChatMessage;

    // Use the overloaded operator<< to serialize the text into the message.
    size_t offset = 0;
    size_t datasize = text.size() * sizeof(char);
    size_t tSize = sizeof(size_t) + sizeof(size_t) + datasize;
    std::unique_ptr<unsigned char[]> buffer(new unsigned char[tSize]);
    std::memcpy(buffer.get() + offset, &tSize, sizeof(size_t)); offset += sizeof(size_t);
    std::memcpy(buffer.get() + offset, &datasize, sizeof(size_t)); offset += sizeof(size_t);
    std::memcpy(buffer.get() + offset, text.data(), datasize);
    msg << buffer;

    // Broadcast the message using the base class function.
    util::logCall("NETWORK", "BroadcastChat()", true);
    this->Broadcast(msg);
}
void Peer::broadcastNode(std::unique_ptr<unsigned char[]> sid) {
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
    std::unique_ptr<unsigned char[]> buffer(new unsigned char[tSize]);
    std::memcpy(buffer.get() + offset, &tSize, sizeof(size_t)); offset += sizeof(size_t);
    std::memcpy(buffer.get() + offset, &datasize, sizeof(size_t)); offset += sizeof(size_t);
    std::memcpy(buffer.get() + offset, id.data(), datasize);
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

std::unique_ptr<unsigned char[]> Peer::serializeStruct(const servID& sid) {
    size_t hostSize = sid.host.size() + 1;  // include null terminator
    size_t outSize = sizeof(size_t) + sizeof(size_t) + sizeof(uint16_t) + hostSize;

    std::unique_ptr<unsigned char[]> buffer(new unsigned char[outSize]);
    size_t offset = 0;

    // Copy hostSize
    std::memcpy(buffer.get() + offset, &outSize, sizeof(size_t));
    offset += sizeof(size_t);

    // Copy hostSize
    std::memcpy(buffer.get() + offset, &hostSize, sizeof(size_t));
    offset += sizeof(size_t);

    // Copy port number
    std::memcpy(buffer.get() + offset, &sid.portNum, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // Copy host string
    std::memcpy(buffer.get() + offset, sid.host.c_str(), hostSize);

    return buffer;
}

servID Peer::deserializeStruct(const std::unique_ptr<unsigned char[]>& buffer) {
    servID sid;
    size_t tSize = 0;
    size_t hostSize = 0;
    size_t offset = 0;

    std::memcpy(&tSize, buffer.get() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&hostSize, buffer.get() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&sid.portNum, buffer.get() + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    sid.host = std::string(reinterpret_cast<const char*>(buffer.get() + offset), hostSize - 1);  // exclude null terminator

    return sid;
}

/* Serialization Methods */
// serialize
std::unique_ptr<unsigned char[]> Peer::serializeWalletInfo(const walletInfo& info) {

    /* Calculate the size required for serialization: */
    size_t tSize = 0;

    // Size of clientID (uint32_t), walladdr length (size_t), walladdr content, and public key length */
    size_t addrSize = info.walladdr.size() + 1;
    size_t pubKeySize = (info.pubKeyy != nullptr) ? i2d_PUBKEY(info.pubKeyy.get(), nullptr) : 0;
    tSize += sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + sizeof(uint32_t) + addrSize + pubKeySize;

    /* Allocate buffer for serialization */
    std::unique_ptr<unsigned char[]> buffer(new unsigned char[tSize]);
    size_t offset = 0;

    /* serialize utx-out */

    /* Serialize tSize itself */
    std::memcpy(buffer.get() + offset, &tSize, sizeof(tSize));
    offset += sizeof(tSize);

    /* Serialize addrSize Size itself */
    std::memcpy(buffer.get() + offset, &addrSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize pubKeySize Size itself */
    std::memcpy(buffer.get() + offset, &pubKeySize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize clientID itself */
    std::memcpy(buffer.get() + offset, &info.clientID, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    /* Serialize WallAddress itself */
    std::memcpy(buffer.get() + offset, info.walladdr.c_str(), addrSize);
    offset += addrSize;

    /* Serialize pubKeyy (EVP_PKEY*) */
    if (info.pubKeyy != nullptr) {
        unsigned char* tempPtr = buffer.get() + offset;
        i2d_PUBKEY(info.pubKeyy.get(), &tempPtr);
        offset += pubKeySize;
    }

    return buffer;
}

/* Deserialize Wallet info */
walletInfo Peer::deserializeWalletInfo(const std::unique_ptr<unsigned char[]>& buffer) {
    walletInfo info;
    size_t offset = 0;

    /* Deserialize tSize (not used in this function, but read to advance the offset) */
    size_t tSize;
    std::memcpy(&tSize, buffer.get() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize addrSize */
    size_t addrSize;
    std::memcpy(&addrSize, buffer.get() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize pubKeySize */
    size_t pubKeySize;
    std::memcpy(&pubKeySize, buffer.get() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize clientID */
    std::memcpy(&info.clientID, buffer.get() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    /* Deserialize WallAddress */
    info.walladdr.resize(addrSize);
    info.walladdr.assign(reinterpret_cast<const char*>(buffer.get() + offset), addrSize - 1);
    offset += addrSize;

    /* Deserialize pubKeyy (EVP_PKEY*) */
    if (pubKeySize > 0) {
        const unsigned char* tempPtr = buffer.get() + offset;
        EVP_PKEY* temp = d2i_PUBKEY(nullptr, &tempPtr, pubKeySize);
        info.pubKeyy.reset(temp, EVP_PKEY_Deleter());
        offset += pubKeySize;
    }
    else {
        info.pubKeyy = nullptr;
    }

    return info;
}