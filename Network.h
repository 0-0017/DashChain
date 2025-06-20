/*
	Decentralized P2P Many To Many Network
	Abstracted Asio Framework
*/


#ifndef NETWORK
#define NETWORK


#include<iostream>
#include <olc_net.h>
#include "util.h"
#include "BlockChain.h"
#include "Wallet.h"
#include "Coin.h"
#include "Consensus.h"

// Message Types
enum class CustomMsgTypes : uint32_t
{
	ChatMessage,
	ServerStart,
	Consensus,
	KnownNode,
	TxRecieved,
	InitialBlock,
	BlkRecieved,
	WalletInfo,
	DelegateID,
	Votes,
};

enum class PODSizeType { POD1, POD2, POD3 };

struct AnyPOD {
	PODSizeType type;
	std::array<unsigned char, 1024> data;
	size_t size;
};

struct servID { std::string host;  uint16_t portNum = 0; };
struct walletInfo { uint32_t clientID = 0; std::string walladdr; EVP_PKEY_ptr pubKeyy; };

/* Server Class */
class Peer : public olc::net::peer_interface<CustomMsgTypes> {
protected:
	//Variables
	Wallet w1;
	Coin X0017;
	static util u;
	uint16_t port;
	servID serverID;
	Consensus consensus;
	std::string delegateID;
	std::string currentDelegate;
	unsigned short sPeriod;
	BlockChain* chain;
	std::thread tMsg; // This Thread Manages Messages
	std::thread tBlk; // This Thread Manages Block Creation
	std::thread tCns; // This Thread Manages Consensus
	unsigned long long created; // Time Server Was Created
	std::vector<servID> nodeID; // List of Servers Structs
	std::vector<walletInfo> wallets;
	std::vector<transactions> mempool;

	AnyPOD SerializePOD(unsigned char* data) {
		size_t pSize = 0;
		std::memcpy(&pSize, data, sizeof(pSize));

		AnyPOD pod;
		pod.size = pSize;

		if (pSize <= 256) {
			std::memcpy(pod.data.data(), data, 256);
			pod.type = PODSizeType::POD1;
		}
		else if (pSize <= 512) {
			std::memcpy(pod.data.data(), data, 512);
			pod.type = PODSizeType::POD2;
		}
		else {
			std::memcpy(pod.data.data(), data, 1024);
			pod.type = PODSizeType::POD3;
		}

		return pod;
	}

	unsigned char* deserializePOD(const AnyPOD& pod) {
		// Allocate just enough memory to hold the data
		unsigned char* buffer = new unsigned char[pod.size];

		// Copy data out of the serialized container
		std::memcpy(buffer, pod.data.data(), pod.size);

		// Caller is responsible for deleting the buffer
		return buffer;
	}

	virtual bool OnPeerConnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> peer) override
	{
		std::cout << "[NetworkManager] New peer connected, ID: " << peer->GetID() << "\n";
		// Accept the connection
		util::logCall("NETWORK", "OnPeerConnect()", true);
		return true;
	}

	/* Called when a client appears to have disconnected */
	virtual void OnPeerDisconnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> peer) override
	{
		if (peer) {
			std::cout << "[NetworkManager] Peer disconnected, ID: " << peer->GetID() << "\n";
			util::logCall("NETWORK", "OnPeerDisconnect()", true);
		}
		else {
			std::cout << "[NetworkManager] A peer disconnected (unknown ID)\n";
			util::logCall("NETWORK", "OnPeerDisconnect()", true);
		}
	}

	/* Called when a message arrives */
	virtual void OnMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> peer, olc::net::message<CustomMsgTypes>& msg) override
	{
		util::logCall("NETWORK", "OnMessage()", true);
		switch (msg.header.id)
		{
			case CustomMsgTypes::ChatMessage:
			{
				AnyPOD rcvMsg;
				msg >> rcvMsg;
				unsigned char* data = deserializePOD(rcvMsg);
				std::string chatText = util::toString(data);
				std::cout << "[This Peer] Chat from peer " << (peer ? std::to_string(peer->GetID()) : "unknown")
						  << ": " << chatText << "\n";
				util::logCall("NETWORK", "OnMessage(ChatMessage)", true);
			}
			case CustomMsgTypes::Consensus:
			{
				AnyPOD rcvMsg;
				msg >> rcvMsg;
				std::tuple<unsigned long long, unsigned long long, unsigned long, unsigned short, unsigned short,
				float, float> cns = consensus.deserializeConsensus(deserializePOD(rcvMsg));
				unsigned long long timestamp = std::get<0>(cns);
				unsigned long long lastUPD = std::get<1>(cns);
				unsigned long votingPeriod = std::get<2>(cns);
				unsigned short windowPeriod = std::get<3>(cns);
				unsigned short maxDelegates = std::get<4>(cns);
				float decayFactor = std::get<5>(cns);
				float minBalance = std::get<6>(cns);
				consensus.setTimestamp(timestamp);
				consensus.setLastUpd(lastUPD);
				consensus.setVotingPeriod(votingPeriod);
				consensus.setWindowPeriod(windowPeriod);
				consensus.setMaxDelegates(maxDelegates);
				consensus.setDecayFactor(decayFactor);
				consensus.setMinBalance(minBalance);
				util::logCall("NETWORK", "OnMessage(Consensus)", true);
			}
				break;
			case CustomMsgTypes::ServerStart:
			{
				AnyPOD rcvMsg;
				msg >> rcvMsg;
				servID newNode = deserializeStruct(deserializePOD(rcvMsg));
				nodeID.push_back(newNode);

				/* Send Information For New Node */
				auto* curr_blk = chain->getFirstBlock();
				int i = 0;
				while (curr_blk->next != nullptr) {
					if (i == 0) {
						auto* block_ser = curr_blk->serialize();
						curr_blk = curr_blk->next;
						olc::net::message<CustomMsgTypes> newBlk;
						newBlk.header.id = CustomMsgTypes::InitialBlock;
						newBlk << block_ser;
						SendToPeer(peer, newBlk);
						i++;
					}
					else {
						auto* block_ser = curr_blk->serialize();
						curr_blk = curr_blk->next;
						olc::net::message<CustomMsgTypes> newBlk;
						newBlk.header.id = CustomMsgTypes::BlkRecieved;
						newBlk << block_ser;
						SendToPeer(peer, newBlk);
						i++;
					}
				}

				/* Send Node List */
				for (auto& it : nodeID) {
					olc::net::message<CustomMsgTypes> list;
					list.header.id = CustomMsgTypes::KnownNode;
					list << serializeStruct(it);
					SendToPeer(peer, list);
				}

				/* Send Consensus Info */
				olc::net::message<CustomMsgTypes> cons;
				cons.header.id = CustomMsgTypes::Consensus;
				cons << consensus.serializeConsensus();
				SendToPeer(peer, cons);

				/* Broadcast updated Node List */
				olc::net::message<CustomMsgTypes> node;
				node.header.id = CustomMsgTypes::KnownNode;
				node << msg.body.data();
				this->Broadcast(node);
				util::logCall("NETWORK", "OnMessage(ServerStart)", true);
			}
			break;
			case CustomMsgTypes::KnownNode:
			{
				AnyPOD rcvMsg;
				msg >> rcvMsg;
				servID newNode = deserializeStruct(deserializePOD(rcvMsg));
				bool present = false;
				for (auto& it : nodeID) {
					if (newNode.portNum == it.portNum && newNode.host == it.host) {
						present = true;
					}
				}

				if (!present) {
					nodeID.push_back(newNode);
					this->broadcastNode(serializeStruct(newNode));
				}
				util::logCall("NETWORK", "OnMessage(KnownNode)", true);
			}
				break;
			case CustomMsgTypes::TxRecieved:
			{
				/* deserialize utx-out and verify transaction */
				AnyPOD rcvMsg;
				msg >> rcvMsg;
				utxout uin;
				uin = w1.deserialize_utxout(deserializePOD(rcvMsg));
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
								for (int i = 0; i < tx.getRecieveAddr().size(); i++) {
									std::vector<std::string>ra;
									std::vector<double> am;
									ra.push_back(txra[i]);
									am.push_back(txam[i]);
									transactions confirmed(w1.getWalletAddr(), ra, am, (tx.getFee() / txra.size()),
										tx.getLockTime(),tx.getVersion(), delegates, delegateID, votesQueue);
									mempool.push_back(confirmed);
									verifyMempool();
									util::logCall("NETWORK", "OnMessage(TxRecieved)", true);
								}
							}
							else {
								/* If the transaction Cant be verifies */
								util::logCall("NETWORK", "OnMessage(TxRecieved)", false, "Addresses Mix Match!");
								std::cout << "Addresses Mix Match!\n";
							}
						}
						else {
							/* If the transaction Cant be verifies */
							util::logCall("NETWORK", "OnMessage(TxRecieved)", false, "Transaction Cannot Be Verified!");
							std::cout << "Transaction Cannot Be Verified!\n";
						}
					}
					else {
						/* If the transaction is on blockchain */
						util::logCall("NETWORK", "OnMessage(TxRecieved)", false, "Transaction Spent!");
						std::cout << "Transaction Spent!\n";
					}

				}
				else {
					/* If the transaction inputs or outputs are invalid */
					util::logCall("NETWORK", "OnMessage(TxRecieved)", false, "Invalid transaction inputs or outputs!");
					std::cout << "Invalid transaction inputs or outputs!\n";
				}

			}
			break;
			case CustomMsgTypes::InitialBlock:
			{
				AnyPOD rcvMsg;
				msg >> rcvMsg;
				Block* nb = chain->getCurrBlock()->deserialize(deserializePOD(rcvMsg));
				chain->initial(nb);
				chain->setChnTmstmp(nb->getTimestamp());
				verifyMempool();
				util::logCall("NETWORK", "OnMessage(InitialBlock)", true);
			}
				break;
			case CustomMsgTypes::BlkRecieved:
			{
				if (chain->verifyBlockchain()) {
					AnyPOD rcvMsg;
					msg >> rcvMsg;
					Block* nb = chain->getCurrBlock()->deserialize(deserializePOD(rcvMsg));
					chain->GenerateBlock(nb->getData(), nb);
					chain->setVersion(nb->getVersion());
					verifyMempool();
					util::logCall("NETWORK", "OnMessage(BlkRecieved)", true);
				}
			}
			break;
			case CustomMsgTypes::WalletInfo:
			{
				AnyPOD rcvMsg;
				msg >> rcvMsg;
				walletInfo wi;
				wi = deserializeWalletInfo(deserializePOD(rcvMsg));
				bool present = false;
				for (auto& wal: wallets) {
					if (wal.walladdr == wi.walladdr) {
						present = true;
					}
				}

				if (!present) {
					wallets.push_back(wi);
				}
				util::logCall("NETWORK", "OnMessage(WalletInfo)", true);
			}
			break;
			case CustomMsgTypes::DelegateID:
			{
				AnyPOD rcvMsg;
				msg >> rcvMsg;
				std::string id = util::toString(deserializePOD(rcvMsg));
				std::vector<std::string> IDs = consensus.getDelegateIDs();
				bool present = false;

				for (auto& it: IDs) {
					if (id == it) {
						present = true;
					}
				}

				if (!present) {
					consensus.addDelegateID(id);
					broadcastDelegateID(id);
				}
				util::logCall("NETWORK", "OnMessage(DelegateID)", true);
			}
				break;
			case CustomMsgTypes::Votes:
			{
				AnyPOD rcvMsg;
				msg >> rcvMsg;
				std::vector<std::tuple<std::string, std::string, float>> votes = Consensus::deserializeVector(deserializePOD(rcvMsg));

				/* Compare for Duplicates */
				bool present = false;
				std::vector<std::tuple<std::string, std::string, float>> vQueue = consensus.getVotesQueue();

				for (auto& it: votes) {
					for (auto& it2: vQueue) {
						if (std::get<0>(it) == std::get<0>(it2) && std::get<1>(it) == std::get<1>(it2) && std::get<2>(it) == std::get<2>(it2)) {
							present = true;
						}

						if (!present) {
							vote(votes);
						}
					}
				}
				util::logCall("NETWORK", "OnMessage(Votes)", true);
			}
				break;
		}
	}
private:
	/* Variable */
	std::mutex mtxA;  // Shared mutex for messages
	std::mutex mtxB;  // Shared mutex for blocks
	std::mutex mtxC;  // Shared mutex consensus


public:
	/* Constructor */
	Peer(uint16_t nPort) : olc::net::peer_interface<CustomMsgTypes>(nPort) {
		if (nPort != 0) {
			port = nPort;
			std::cout << "[NetworkManager] Listening on port " << nPort << "\n";
		}
	}

	~Peer() {
		tMsg.join();
		tBlk.join();
		tCns.join();
		delete chain;
		this->Stop();
	}

	/* Updates the Server for incoming messages */
	void msgLoop(Peer& server);

	/* Updates the Server for Block Creation */
	void blkLoop(Peer& server);

	/* Updates the Server for consensus */
	void cnsLoop(Peer& server);

	/* Setter to add a ServerID object to the vector */
	void setNodeID(const servID& sid);

	/* setter for timestamp created */
	void setTimeCreated();

	/* set server IP and Port */
	void setServerID();

	/* Server Setup */
	void serverOnStart(Peer& server);

	/* Update Time slot max seconds! */
	void updateSlot();

	/* Update mempool (Called everytime tx is received) */
	void verifyMempool();

	/* add new block to chain */
	void blkRqMethod();

	/* Update Coin */
	void updateCoins(transactions rew);

	/*Alert Delegate to Generate block*/
	void voteDelegate(const std::vector<std::tuple<std::string, std::string, float>>&);

	std::string requestDelegate();

	/* Update Wallets */
	void updateWallets();

	/* Get Wallets Balance */
	double getBalance() const;

	/* Get Wallets Address */
	std::string getWalletAddress() const;

	/* Vote For Delegates */
	void vote(std::vector<std::tuple<std::string, std::string, float>> votes);

	/* Get List of UTXOs In Wallet */
	void listTx();

	/* sends transaction and returns success factor */
	bool sendTx(std::vector<std::string>& recipients, std::vector<double> amounts);

	/* Display transaction present in blockchain */
	void getKnownTx(std::string& txid);

	/* get current blockchain information */
	void currBlockInfo();

	void getBlock(unsigned int height);

	// Utility function to initiate an outbound connection to a remote peer.
	bool ConnectTo(const std::string& host, uint16_t port);

	// Utility function to broadcast a chat message to every connected peer.
	void BroadcastChat(const std::string& text);

	void broadcastNode(unsigned char* sid);

	void broadcastBlock(const Block* block);

	void broadcastVotes(std::vector<std::tuple<std::string, std::string, float>> votes);

	void broadcastTransaction(const utxout& u_out);

	void broadcastDelegateID(const std::string& id);

	/* struct de-serialization methods */
	unsigned char* serializeStruct(const servID& sid);
	servID deserializeStruct(const unsigned char* buffer);

	unsigned char* serializeWalletInfo(const walletInfo& info);
	walletInfo deserializeWalletInfo(const unsigned char* buffer);


};
#endif