#include"BlockChain.h"
#include"Wallet.h"
#include"Network.h"


int main() {
	//Wallet wal;
	Server server(50507);
	server.Start();
	server.serverOnStart(server);



	// Test Client Conn
	std::cout << "Connecting Client...\n";
	Network n;
	n.Connect("127.0.0.1", 50507);
	n.maintainance();
	n.connectWallet();

	return 0;
}