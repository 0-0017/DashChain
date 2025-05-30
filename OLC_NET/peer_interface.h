#pragma once

#include "net_common.h"      // Your common includes and ASIO setup.
#include "net_tsqueue.h"     // Unchanged.
#include "net_message.h"    // Unchanged message definitions.
#include "net_connection.h" // Our updated connection class.


    namespace olc::net {

        // The peer_interface class is our unified peer-to-peer abstraction.
        template<typename T>
        class peer_interface {
        public:
            // Construct a peer; optionally start listening on a specific port.
            // (If listen_port is 0, the peer will operate in outbound-only mode.)
            peer_interface(uint16_t listen_port = 0)
                : m_listenPort(listen_port),
                  m_acceptor(m_ioContext)
            {
                if (m_listenPort != 0) {
                    StartListening(m_listenPort);
                }
            }

            virtual ~peer_interface() {
                Stop();
            }

            // Start listening on a given port for inbound peer connections.
            bool StartListening(uint16_t port) {
                try {
                    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
                    m_acceptor.open(endpoint.protocol());
                    m_acceptor.set_option(asio::socket_base::reuse_address(true));
                    m_acceptor.bind(endpoint);
                    m_acceptor.listen();
                    WaitForPeer();
                    m_threadContext = std::thread([this]() { m_ioContext.run(); });
                }
                catch (std::exception& e) {
                    std::cerr << "[PEER] Listen Exception: " << e.what() << "\n";
                    return false;
                }
                std::cout << "[PEER] Listening on port " << port << "\n";
                return true;
            }

            // Initiate an outbound connection to a remote peer.
            bool ConnectToPeer(const std::string& host, uint16_t port) {
                try {
                    asio::ip::tcp::resolver resolver(m_ioContext);
                    auto endpoints = resolver.resolve(host, std::to_string(port));
                    auto newPeer = std::make_shared<connection<T>>(m_ioContext, asio::ip::tcp::socket(m_ioContext), m_qMessagesIn);
                    newPeer->SetID(nIDCounter++);
                    newPeer->StartOutbound(endpoints);
                    m_connections.push_back(newPeer);
                }
                catch (std::exception& e) {
                    std::cerr << "[PEER] Connect Exception: " << e.what() << "\n";
                    return false;
                }
                return true;
            }

            // Disconnect all active peer connections and stop the IO context.
            void Stop() {
                m_ioContext.stop();
                if (m_threadContext.joinable())
                    m_threadContext.join();
                m_connections.clear();
            }

            // Send a message to a specific peer.
            void SendToPeer(std::shared_ptr<connection<T>> peerConn, const message<T>& msg) {
                if (peerConn && peerConn->IsConnected())
                    peerConn->Send(msg);
                else {
                    OnPeerDisconnect(peerConn);
                    RemovePeer(peerConn);
                }
            }

            // Broadcast a message to all connected peers.
            void Broadcast(const message<T>& msg, std::shared_ptr<connection<T>> pIgnorePeer = nullptr) {
                for (auto& peerConn : m_connections) {
                    if (peerConn && peerConn->IsConnected() && peerConn != pIgnorePeer)
                        peerConn->Send(msg);
                    else if (!peerConn || !peerConn->IsConnected()) {
                        OnPeerDisconnect(peerConn);
                        RemovePeer(peerConn);
                    }
                }
            }

            // Process incoming messages (up to nMaxMessages; block if bWait is true).
            void Update(size_t nMaxMessages = -1, bool bWait = false) {
                if (bWait)
                    m_qMessagesIn.wait();

                size_t nMessageCount = 0;
                while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty()) {
                    auto msg = m_qMessagesIn.pop_front();
                    OnMessage(msg.remote, msg.msg);
                    ++nMessageCount;
                }
            }

        protected:
            // Override these virtual callbacks to handle events as needed.
            virtual bool OnPeerConnect(std::shared_ptr<connection<T>> peer) {
                // Accept all inbound peer connections by default.
                return true;
            }

            virtual void OnPeerDisconnect(std::shared_ptr<connection<T>> peer) {
                std::cout << "[PEER] Disconnected: " << (peer ? peer->GetID() : 0) << "\n";
            }

            virtual void OnMessage(std::shared_ptr<connection<T>> peer, message<T>& msg) {
                // Default does nothing. Override to process received messages.
            }

        private:
            // Internal function to continuously accept new inbound connections.
            void WaitForPeer() {
                m_acceptor.async_accept(
                    [this](std::error_code ec, asio::ip::tcp::socket socket) {
                        if (!ec) {
                            std::cout << "[PEER] New inbound connection from: " << socket.remote_endpoint() << "\n";
                            auto newPeer = std::make_shared<connection<T>>(m_ioContext, std::move(socket), m_qMessagesIn);
                            newPeer->SetID(nIDCounter++);
                            if (OnPeerConnect(newPeer)) {
                                m_connections.push_back(newPeer);
                                newPeer->StartInbound();
                                std::cout << "[PEER] Connection approved with ID: " << newPeer->GetID() << "\n";
                            }
                            else {
                                std::cout << "[PEER] Connection denied.\n";
                            }
                        }
                        else {
                            std::cerr << "[PEER] Accept error: " << ec.message() << "\n";
                        }
                        WaitForPeer(); // Continue accepting the next connection.
                    });
            }

            // Remove a peer from the container.
            void RemovePeer(std::shared_ptr<connection<T>> peer) {
                m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), peer), m_connections.end());
            }

        protected:
            asio::io_context m_ioContext;
            asio::ip::tcp::acceptor m_acceptor;
            std::thread m_threadContext;
            std::deque<std::shared_ptr<connection<T>>> m_connections;
            tsqueue<owned_message<T>> m_qMessagesIn;

            uint16_t m_listenPort;
            uint32_t nIDCounter = 10000;
        };
    }

