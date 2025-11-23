
// client.cpp
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

const char* SOCKET_PATH = "/tmp/pingpong.sock";
const int   MAX_PINGS   = 5;   // same limit as on server

bool read_line(int fd, std::string &out) {
    out.clear();
    char ch;
    while (true) {
        ssize_t n = ::read(fd, &ch, 1);
        if (n == 0) {
            // server closed connection
            return false;
        }
        if (n < 0) {
            perror("read");
            return false;
        }
        if (ch == '\n') {
            return true;
        }
        out.push_back(ch);
    }
}

int main() {
    int sock_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (::connect(sock_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        perror("connect");
        ::close(sock_fd);
        return 1;
    }

    std::cout << "[client] connected, fd=" << sock_fd << std::endl;

    for (int i = 0; i < MAX_PINGS; ++i) {
        std::string msg = "PING\n";
        if (::write(sock_fd, msg.c_str(), msg.size()) == -1) {
            perror("write");
            break;
        }
        std::cout << "[C1] sent PING #" << (i + 1) << std::endl;

        std::string resp;
        std::cout << "[C2] waiting for response..." << std::endl;
        if (!read_line(sock_fd, resp)) {
            std::cout << "[client] server closed connection prematurely" << std::endl;
            ::close(sock_fd);
            return 1;
        }

        std::cout << "[C3] server response: '" << resp << "'" << std::endl;

        if (resp != "PONG") {
            std::cout << "[C4] unexpected response (expected PONG), terminating" << std::endl;
            ::close(sock_fd);
            return 1;
        }
    }

    std::cout << "[client] reached PING limit = " << MAX_PINGS
              << ", terminating correctly" << std::endl;

    ::close(sock_fd);
    return 0;
}

