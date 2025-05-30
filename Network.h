/*
	Decentralized P2P Many To Many Network
	Abstracted Asio Framework
*/


#ifndef NETWORK
#define NETWORK


#include<iostream>
#include <olc_net.h>
#include <curl/curl.h>
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
	Delegate,
	DelegateUpdate,
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
	std::thread tMsg;  // This Thread Manages Messages
	std::thread tTx; // This Thread Manages Transactions & everything else
	unsigned long long created; // Time Server Was Created
	std::vector<servID> nodeID; // List of Servers Structs
	std::vector<walletInfo> wallets;
	std::vector<transactions> mempool;

	virtual bool OnPeerConnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> peer) override
	{
		std::cout << "[NetworkManager] New peer connected, ID: " << peer->GetID() << "\n";
		// Accept the connection
		return true;
	}

	/* Called when a client appears to have disconnected */
	virtual void OnPeerDisconnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> peer) override
	{
		if (peer) {
			std::cout << "[NetworkManager] Peer disconnected, ID: " << peer->GetID() << "\n";
		}
		else {
			std::cout << "[NetworkManager] A peer disconnected (unknown ID)\n";
		}
	}

	// Helper function to extract a string from the message’s body.
	// (This assumes you serialized a std::string with the overloaded << operator.)
	static std::string ExtractString(olc::net::message<CustomMsgTypes>& msg) {
		std::string txt;
		// >> operator is defined to pull data from the message's body in LIFO order.
		msg >> txt;
		return txt;
	}

	/* Called when a message arrives */
	virtual void OnMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> peer, olc::net::message<CustomMsgTypes>& msg) override
	{
		switch (msg.header.id)
		{
			case CustomMsgTypes::ChatMessage:
			{
				std::string chatText = ExtractString(msg);
				std::cout << "[This Peer] Chat from peer " << (peer ? std::to_string(peer->GetID()) : "unknown")
						  << ": " << chatText << "\n";
			}
			case CustomMsgTypes::ServerStart:
			{
				servID newNode = deserializeStruct(msg.body.data());
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
			}
			break;
			case CustomMsgTypes::KnownNode:
			{
				servID newNode = deserializeStruct(msg.body.data());
				nodeID.push_back(newNode);
			}
				break;
			case CustomMsgTypes::TxRecieved:
			{
				/* deserialize utx-out and verify transaction */
				utxout uin;
				unsigned char* body;
				std::memcpy(&body, msg.body.data(), msg.body.size());
				uin = w1.deserialize_utxout(body);
				transactions tx = tx.deserialize(uin.utxo.get());


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
								}
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
			break;
			case CustomMsgTypes::InitialBlock:
			{
				unsigned char* body;
				std::memcpy(&body, msg.body.data(), msg.body.size());
				Block* nb = chain->getCurrBlock()->deserialize(body);
				chain->initial(nb);
				verifyMempool();
			}
				break;
			case CustomMsgTypes::BlkRecieved: //***
			{
				unsigned char* body;
				std::memcpy(&body, msg.body.data(), msg.body.size());
				Block* nb = chain->getCurrBlock()->deserialize(body);
				chain->getCurrBlock()->next = nb;
				nb->next = nullptr;
				verifyMempool();
				/////***** PUT VERIFY CHAIN IN UPLOOP!
			}
			break;
			case CustomMsgTypes::WalletInfo: ///*****
			{
				walletInfo wi;
				unsigned char* body;
				std::memcpy(&body, msg.body.data(), msg.body.size());
				wi = deserializeWalletInfo(body);
				wallets.push_back(wi);
			}
			break;
			case CustomMsgTypes::Delegate:
			{

			}
				break;
		}
	}
private:
	/* Variable */
	std::mutex mtx;  // Shared mutex


public:
	/* Constructor */
	Peer(uint16_t nPort) : olc::net::peer_interface<CustomMsgTypes>(nPort) {
		if (nPort != 0) {
			port = nPort;
			std::cout << "[NetworkManager] Listening on port " << nPort << "\n";
		}
	}

	~Peer() {
		delete chain;
		this->Stop();
	}

	/* Updates the Server for incoming connection */
	void upLoop(Peer& server);

	/* Updates the D-POS consensus for each voting period  */
	void updateD_POS();

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

	/* Update Wallets */
	void updateWallets();

	/* Get Wallets Balance */
	double getBalance() const;

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

	void broadcastTransaction(const utxout& u_out);

	void broadcastDelegates();

	void broadcastDelegateUpdate(const std::string& delID);

	/* struct de-serialization methods */
	unsigned char* serializeStruct(const servID& sid);
	servID deserializeStruct(const unsigned char* buffer);

	unsigned char* serializeWalletInfo(const walletInfo& info);
	walletInfo deserializeWalletInfo(const unsigned char* buffer);


};
#endif