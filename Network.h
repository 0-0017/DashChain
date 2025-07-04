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
#include "Trainer.h"

// Message Types
enum class CustomMsgTypes : uint32_t
{
	ChatMessage,
	ServerStart,
	Consensus,
	KnownNode,
	TxRecieved,
	BlkRecieved,
	StartComplete,
	PopulateChain,
	WalletInfo,
	DelegateID,
	Votes,
	Chain,
};

struct servID { std::string host;  uint16_t portNum = 0; };
struct walletInfo { uint32_t clientID = 0; std::string walladdr; EVP_PKEY_ptr pubKeyy; };

/* Server Class */
class Peer : public olc::net::peer_interface<CustomMsgTypes> {
protected:
	//Variables
	Wallet w1;
	Coin X0017;
	Trainer trainer;
	static util u;
	uint16_t port;
	bool rcvCns;
	servID serverID;
	Consensus consensus;
	std::string delegateID;
	std::string currentDelegate;
	unsigned short sPeriod;
	BlockChain* chain;
	std::thread tMsg; // This Thread Manages Messages
	std::thread tBlk; // This Thread Manages Block Creation
	std::thread tCns; // This Thread Manages Consensus
	std::thread tTrn; // This thread manages AI Training
	unsigned long long created; // Time Server Was Created
	std::vector<servID> nodeID; // List of Servers Structs
	std::vector<Block*> block_pool;
	std::vector<walletInfo> wallets;
	std::vector<transactions> mempool;

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
				std::cout << "Chat Message\n";
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				size_t offset = 0;
				size_t dataSize = 0;
				std::memcpy(&dataSize, rec.get() + sizeof(size_t), sizeof(size_t)); offset = (sizeof(size_t) * 2);
				std::unique_ptr<unsigned char[]> data(new unsigned char[dataSize]);
				std::memcpy(data.get(), rec.get() + offset, dataSize);
				std::string chatText = util::toString(data.get());
				std::cout << "[This Peer] Chat from peer " << (peer ? std::to_string(peer->GetID()) : "unknown")
						  << ": " << chatText << "\n";
				util::logCall("NETWORK", "OnMessage(ChatMessage)", true);
			}
			case CustomMsgTypes::Consensus:
			{
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				std::tuple<unsigned long long, unsigned long long, unsigned long, unsigned short, unsigned short,
				float, float> cns = consensus.deserializeConsensus(rec);
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
				rcvCns = true;
				util::logCall("NETWORK", "OnMessage(Consensus)", true);
			}
				break;
			case CustomMsgTypes::ServerStart:
			{
				std::cout << "ServerStart Message\n";
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				servID newNode = deserializeStruct(rec);
				nodeID.push_back(newNode);

				/* Send Current Chain State */
				olc::net::message<CustomMsgTypes> chn;
				chn.header.id = CustomMsgTypes::Chain;
				chn << chain->serializeInfo();
				SendToPeer(peer, chn);

				/* Send Information For New Node */
				auto* curr_blk = chain->getFirstBlock();
				int i = 0;
				if (curr_blk->next == nullptr) {
					std::unique_ptr<unsigned char[]> block_ser = curr_blk->serialize();
					olc::net::message<CustomMsgTypes> newBlk;
					newBlk.header.id = CustomMsgTypes::PopulateChain;
					newBlk << block_ser;
					SendToPeer(peer, newBlk);
				}
				else {
					do {
						std::unique_ptr<unsigned char[]> block_ser = curr_blk->serialize();
						curr_blk = curr_blk->next;
						olc::net::message<CustomMsgTypes> newBlk;
						newBlk.header.id = CustomMsgTypes::PopulateChain;
						newBlk << block_ser;
						SendToPeer(peer, newBlk);
						i++;
					} while (curr_blk->next != nullptr);

					/* Serialize Final Block */
					std::unique_ptr<unsigned char[]> block_ser = curr_blk->serialize();
					curr_blk = nullptr;
					olc::net::message<CustomMsgTypes> newBlk;
					newBlk.header.id = CustomMsgTypes::PopulateChain;
					newBlk << block_ser;
					SendToPeer(peer, newBlk);
				}
				/* Signal The End Of Block Transmission */
				olc::net::message<CustomMsgTypes> strComplete;
				strComplete.header.id = CustomMsgTypes::StartComplete;
				SendToPeer(peer, strComplete);

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
				node << rec;
				this->Broadcast(node);
				util::logCall("NETWORK", "OnMessage(ServerStart)", true);
			}
			break;
			case CustomMsgTypes::StartComplete:
			{
				std::cout << "StartComplete Message\n";

				/* Sort Blocks From Lowest To Highest by Block height */
				std::ranges::sort(block_pool, [](const Block* a, const Block* b)
				                  {return a->getBlockHeight() < b->getBlockHeight();});

				/* Verify & Populate Chain */
				size_t pos = 0;

				for (auto& it : block_pool) {
					if (pos == 0) {
						chain->initial(it);
						chain->setChnTmstmp(it->getTimestamp());
						verifyMempool();
						confirm();
						util::logCall("NETWORK", "OnMessage(StartComplete - Initial)", true);
						pos++;
					}
					else {
						if (chain->verifyBlockchain()) {
							chain->GenerateBlock(it->getData(), it);
							chain->setVersion(it->getVersion());
							verifyMempool();
							confirm();
							util::logCall("NETWORK", "OnMessage(StartComplete - NextBlk)", true);
						}
					}
				}
			}
			break;
			case CustomMsgTypes::PopulateChain:
			{
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				Block* nb = chain->getCurrBlock()->deserialize(rec);
				block_pool.emplace_back(nb);
			}
			break;
			case CustomMsgTypes::KnownNode:
			{
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				servID newNode = deserializeStruct(rec);
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
			}
				break;
			case CustomMsgTypes::TxRecieved:
			{
				/* deserialize utx-out and verify transaction */
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				utxout uin;
				uin = w1.deserialize_utxout(rec);
				mempool_emplace(uin);
				broadcastTransaction(uin);
			}
			break;
			case CustomMsgTypes::BlkRecieved:
			{
				if (chain->verifyBlockchain()) {
					std::unique_ptr<unsigned char[]> rec;
					msg >> rec;
					Block* nb = chain->getCurrBlock()->deserialize(rec);
					chain->GenerateBlock(nb->getData(), nb);
					chain->setVersion(nb->getVersion());
					verifyMempool();
					confirm();
				}
			}
			break;
			case CustomMsgTypes::WalletInfo:
			{
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				walletInfo wi;
				wi = deserializeWalletInfo(rec);
				bool present = false;
				for (auto& wal: wallets) {
					if (wal.walladdr == wi.walladdr) {
						present = true;
					}
				}

				if (!present) {
					wallets.push_back(wi);
				}
			}
			break;
			case CustomMsgTypes::DelegateID:
			{
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				size_t offset = 0;
				size_t dataSize = 0;
				std::memcpy(&dataSize, rec.get() + sizeof(size_t), sizeof(size_t)); offset = (sizeof(size_t) * 2);
				std::unique_ptr<unsigned char[]> data(new unsigned char[dataSize]);
				std::memcpy(data.get(), rec.get() + offset, dataSize);
				std::string id = util::toString(data.get());
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
			}
				break;
			case CustomMsgTypes::Votes:
			{
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				std::vector<std::tuple<std::string, std::string, float>> votes = Consensus::deserializeVector(rec);

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
			}
				break;
			case CustomMsgTypes::Chain:
			{
				std::cout << "Chain Message\n";
				std::unique_ptr<unsigned char[]> rec;
				msg >> rec;
				chain->deserializeInfo(rec);
			}
		}
	}
private:
	/* Variable */
	std::mutex mtxA; // Shared mutex for messages
	std::mutex mtxB; // Shared mutex for blocks
	std::mutex mtxC; // Shared mutex consensus
	std::mutex mtxD; // Shared mutex AI


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
		tTrn.join();
		delete chain;
		this->Stop();
	}

	/* Updates the Server for incoming messages */
	void msgLoop(Peer& server);

	/* Updates the Server for Block Creation */
	void blkLoop(Peer& server);

	/* Updates the Server for consensus */
	void cnsLoop(Peer& server);

	/* Updates the Server for AI Training */
	void trnLoop(Peer& server);

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

	/* Add Transactions To The Mempool */
	void mempool_emplace(const utxout& uin);

	/* Update mempool (Called everytime tx is received) */
	void verifyMempool();

	/* add new block to chain */
	void blkRqMethod();

	/* Update Coin */
	void updateCoins(transactions rew);

	/*Alert Delegate to Generate block*/
	void voteDelegate(const std::vector<std::tuple<std::string, std::string, float>>&);

	/* Request to be a delegate */
	std::string requestDelegate();

	/* confirm latest Block in accordance with confirmation period */
	void confirm();

	/* Get Wallets Balance */
	double getBalance() const;

	/* Change Wallet Address to existing Address */
	void set_address(const std::string& wa);

	/* Get Instance Delegate ID */
	std::string get_this_delID() const;

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

	/* Utility function to initiate an outbound connection to a remote peer. */
	bool ConnectTo(const std::string& host, uint16_t port);

	/* Train Data Via AI API */
	void train_data();

	/* Utility function to broadcast a chat message to every connected peer. */
	void BroadcastChat(const std::string& text);

	void broadcastNode(std::unique_ptr<unsigned char[]> sid);

	void broadcastBlock(const Block* block);

	void broadcastVotes(std::vector<std::tuple<std::string, std::string, float>> votes);

	void broadcastTransaction(const utxout& u_out);

	void broadcastDelegateID(const std::string& id);

	/* struct de-serialization methods */
	std::unique_ptr<unsigned char[]> serializeStruct(const servID& sid);
	servID deserializeStruct(const std::unique_ptr<unsigned char[]>& buffer);

	std::unique_ptr<unsigned char[]> serializeWalletInfo(const walletInfo& info);
	walletInfo deserializeWalletInfo(const std::unique_ptr<unsigned char[]>& buffer);


};
#endif