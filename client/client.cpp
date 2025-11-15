#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class PingClient {
private:
    SOCKET clientSocket = INVALID_SOCKET;
    const std::string SERVER_IP = "127.0.0.1";
    const int PORT = 8080;

public:
    PingClient() {
        // Initialize Winsock
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return;
        }
    }

    ~PingClient() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
        }
        WSACleanup();
    }

    bool connectToServer() {
        // Create socket
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation error: " << WSAGetLastError() << std::endl;
            return false;
        }

        // Configure server address
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
        serverAddr.sin_port = htons(PORT);

        // Connect to server
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
            return false;
        }

        return true;
    }

    void sendPing() {
        // State 1: Creating request
        std::cout << "State 1: Creating 'ping' request\n";

        if (!connectToServer()) {
            std::cout << "Error: Could not connect to server\n";
            return;
        }

        // Send ping
        std::string message = "ping";
        int sendResult = send(clientSocket, message.c_str(), message.length(), 0);
        if (sendResult == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
            return;
        }

        std::cout << "Request 'ping' sent to server\n";

        // State 2: Waiting for response
        std::cout << "State 2: Waiting for server response...\n";

        char buffer[256];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string response(buffer);

            // State 3: Reading response
            std::cout << "State 3: Received response: " << response << "\n";

            // State 4: Processing response
            std::cout << "State 4: Processing response\n";

            if (response == "pong") {
                std::cout << "Success: Received correct 'pong' response\n";
            } else {
                std::cout << "Error: " << response << "\n";
            }
        } else {
            std::cout << "Error: No response received from server\n";
        }

        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
};

int main() {
    PingClient client;

    std::cout << "Ping Client started.\n";
    std::cout << "Program will automatically send ping every 3 seconds.\n";
    std::cout << "Press Ctrl+C to exit\n\n";

    int counter = 0;
    while (counter < 10) {  // Limit to 10 cycles for demonstration
        std::cout << "\n--- Cycle " << ++counter << " ---\n";
        client.sendPing();
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    std::cout << "Client finished work.\n";
    return 0;
}