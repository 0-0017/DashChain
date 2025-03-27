/*
	Decentralized Bootstraped Node List
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

// Message Types
enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
	ServerStart,
	TxRecieved,
	BlkRecieved,
	WalletInfo,
	ClientUpdate
};

struct servID { std::string host;  uint16_t portNum; };
struct walletInfo { uint32_t clientID; std::string walladdr; EVP_PKEY* pubKeyy; };

/* Inherits From Net_Client's Interface */
class Network : public olc::net::client_interface<CustomMsgTypes> {
public:

	/* pings server with current time */
	void PingServer();

	/* Message all clients in conn pool */
	void MessageAll();

	/* Carries out normal server bootup operations */
	void ServerStart(unsigned char* sid);

	/* Sends Signed & verifies UTXOs to owner */
	void sendUTXO(const utxout& uout);

	/* Send Block to all nodes */
	void sendBlk(Block* block);

	void connectWallet();

	/* Updates the Server for incoming connection */
	void maintainance();
	void upLoop();

	/* struct de-serializaton methods */
	unsigned char* serializeStruct(const servID& sid);
	servID deserializeStruct(const unsigned char* buffer);

	/**/
	unsigned char* serializeWalletInfo(const walletInfo& info);
	walletInfo deserializeWalletInfo(const unsigned char* buffer);


public:
	Network() {
		//
	}

	~Network() {
		if (uploop.joinable()) {
			uploop.join();
		}
	}


protected:
	Wallet w1;
	std::vector<servID> nodeID;
	std::thread uploop;  // manage client update loop
	util u;

};


/* Server Class */
class Server : public olc::net::server_interface<CustomMsgTypes> {
protected:
	//Variables
	Network n;
	Wallet w1;
	Coin X0017;
	uint16_t port;
	servID serverID;
	util u; // Utility Class
	unsigned short sPeriod = 15;
	BlockChain* chain = nullptr;
	std::thread uploop;  // manage server update loop
	unsigned long long created = 0; // Time Server Was Created
	std::vector<servID> nodeID; // List of Servers Structs
	std::vector<walletInfo> wallets;
	std::vector<transactions> mempool;

	virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	/* Called when a client appears to have disconnected */
	virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	/* Called when a message arrives */
	virtual void OnMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client, olc::net::message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
		case CustomMsgTypes::ServerPing:
		{
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			/* Simply bounce message back to client */
			client->Send(msg);
		}
		break;

		case CustomMsgTypes::MessageAll:
		{
			std::cout << "[" << client->GetID() << "]: Message All\n";

			/* Construct a new messageand send it to all clients */
			olc::net::message<CustomMsgTypes> mssg;
			mssg.header.id = CustomMsgTypes::ServerMessage;
			mssg << client->GetID();
			MessageAllClients(mssg, client);

		}
		break;
		case CustomMsgTypes::ServerStart: //*********
		{
			std::cout << "[" << client->GetID() << "]: Server Started\n";
			/* Construct a new message and send it to new server */
			olc::net::message<CustomMsgTypes> mssg;
			mssg.header.id = CustomMsgTypes::ServerStart;
			unsigned char* body;
			std::memcpy(&body, msg.body.data(), msg.body.size());
			nodeID.push_back(deserializeStruct(body));

			/* Send Nodes List */
			for (int i = 0; i < nodeID.size(); i++) {
				mssg << serializeStruct(nodeID[i]);
				client->Send(mssg); //* on recieve //******** NOT EFFICIENT
				MessageAllClients(mssg, client); //*onRecieve //******** NOT EFFICIENT
			}
		}
		break;
		case CustomMsgTypes::TxRecieved:
		{
			/* deserialize utxout and verify transaction */
			utxout uin;
			transactions tx;
			unsigned char* body;
			std::memcpy(&body, msg.body.data(), msg.body.size());
			uin = w1.deserialize_utxout(body);
			tx = tx.deserialize(uin.utxo);


			/* Verify if the transaction is valid */
			if (tx.inputsValid() && tx.outputsValid()) {
				/* check for double spend */
				if (chain->isNewTxid(tx.getTxid())) {
					if (w1.verifyTx(uin)) {
						if (tx.getRecieveAddr().size() == tx.getAmmount().size() && tx.getRecieveAddr().size() == tx.getRecievePkeys().size()) {
							std::vector<std::string> txra = tx.getRecieveAddr();
							std::vector<EVP_PKEY*> txrpk = tx.getRecievePkeys();
							std::vector<double> txam = tx.getAmmount();
							for (int i = 0; i < tx.getRecieveAddr().size(); i++) {
								transactions confirmed;
								std::vector<std::string>ra;
								std::vector<EVP_PKEY*> rpk;
								std::vector<double> am;
								ra.push_back(txra[i]);
								rpk.push_back(txrpk[i]);
								am.push_back(txam[i]);
								confirmed.setTimeStamp(u.TimeStamp());
								confirmed.setTxid(w1.calcTxid(u.TimeStamp()));
								confirmed.setSendAddr(w1.getWalletAddr());
								confirmed.setRecieveAddr(ra);
								confirmed.setSendPkey(tx.getSendPkey());
								confirmed.setRecievePkeys(rpk);
								confirmed.setAmmount(am);
								confirmed.setFee((tx.getFee() / txra.size()));
								confirmed.setLockTime(tx.getLockTime());
								confirmed.setVersion(tx.getVersion());
								mempool.push_back(confirmed);
								verifyMempool();
							}
						}
						else {
							/* If the transaction Cant be verifies */
							std::cout << "Adresses Mix Match!\n";
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
		case CustomMsgTypes::BlkRecieved: //***
		{
			unsigned char* body;
			std::memcpy(&body, msg.body.data(), msg.body.size());
			Block* nb = chain->getCurrBlock()->deserialize(body);
			chain->GenerateBlock(nb->getTxs(), nb); //*
			verifyMempool();
			/////***** PUT VERIFY CHAIN IN UPLOOP!
		}
		break;
		case CustomMsgTypes::WalletInfo: ///*****
		{
			walletInfo wi;
			unsigned char* body;
			std::memcpy(&body, msg.body.data(), msg.body.size());
			wi = n.deserializeWalletInfo(body);
			wallets.push_back(wi);
		}
		break;
		}
	}
private:
	/* Variable */
	std::mutex mtx;  // Shared mutex


public:
	/* Constructor */
	Server(uint16_t nPort) : olc::net::server_interface<CustomMsgTypes>(nPort) {
		port = nPort;
	}

	~Server() {
		delete chain;
		if (uploop.joinable()) {
			uploop.join();
		}
	}

	/* Updates the Server for incoming connection */
	void upLoop(Server& server);

	/* Setter to add a ServerID object to the vector */
	void setNodeID(const servID& sid);

	/* setter for timestamp created */
	void setTimeCreated();

	/* set server IP and Port */
	void setServerID();

	/* Server Setup */
	void serverOnStart(Server& server);

	/* Retrieces clients messages from all clients */
	void clientUpLoop();

	/* Update Time slot max seconds! */
	void updateSlot();

	/* Update mempool (Called everytime tx is recieved) */
	void verifyMempool();

	/* add new block to chain */
	void blkRqMethod();

	/* Update Coin */
	void updateCoins(transactions rew);

	/* Update Wallets */
	void updateWallets();

	/* struct de-serializaton methods */
	unsigned char* serializeStruct(const servID& sid);
	servID deserializeStruct(const unsigned char* buffer);


};
#endif
