#include"BlockChain.h"
#include"Wallet.h"
#include"Network.h"


int main() {
    //Wallet wal;
    Peer server(50507);
    server.serverOnStart(server);



    // Test Client Conn
    while (1) {
        server.upLoop(server);
    }

    return 0;
}