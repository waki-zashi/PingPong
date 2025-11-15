#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class PongServer {
private:
    SOCKET serverSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;
    bool running = true;
    const int PORT = 8080;

public:
    PongServer() {
        // Initialize Winsock
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return;
        }
    }

    ~PongServer() {
        cleanup();
        WSACleanup();
    }

    void cleanup() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
        }
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
        }
    }

    void run() {
        std::cout << "Pong Server started. Waiting for connections...\n";

        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation error: " << WSAGetLastError() << std::endl;
            return;
        }

        // Configure server address
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);

        // Bind socket
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            return;
        }

        // Listen on port
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            return;
        }

        std::cout << "Server listening on port " << PORT << std::endl;
        std::cout << "Close this window to stop the server\n\n";

        while (running) {
            // State 1: Waiting for connection
            std::cout << "State 1: Waiting for client connection...\n";

            // Accept connection
            sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);

            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
                continue;
            }

            std::cout << "Client connected!\n";

            char buffer[256];
            int bytesReceived;

            // Receive data from client
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string message(buffer);

                // State 2.1: Processing request
                std::cout << "State 2.1: Processing request: " << message << "\n";

                if (message == "ping") {
                    // State 3: Sending response
                    std::cout << "State 3: Sending 'pong' response\n";

                    // Send response
                    std::string response = "pong";
                    send(clientSocket, response.c_str(), response.length(), 0);

                    std::cout << "Response 'pong' sent to client\n";
                } else {
                    // State 2.2: Error
                    std::cout << "State 2.2: Error - unknown request: " << message << "\n";

                    std::string error = "error: unknown command";
                    send(clientSocket, error.c_str(), error.length(), 0);
                }
            }

            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;

            std::cout << "Ready for new connection...\n\n";
        }
    }
};

int main() {
    PongServer server;
    server.run();
    return 0;
}