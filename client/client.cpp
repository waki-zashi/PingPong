#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

const char* SOCKET_PATH = "/tmp/pingpong.sock";
const int MAX_PINGS = 5;
const unsigned int DELAY_AFTER_PING  = 1;
const unsigned int DELAY_AFTER_PONG  = 1;

bool read_line(int fd, std::string& out) {
    out.clear();
    char ch;
    while (true) {
        ssize_t n = ::read(fd, &ch, 1);
        if (n <= 0) {
            if (n < 0) perror("read");
            return false;
        }
        if (ch == '\n') return true;
        out.push_back(ch);
    }
}

int main() {
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    std::cout << "[client] connected, fd=" << sock_fd << std::endl;

    for (int i = 0; i < MAX_PINGS; ++i) {

        std::string msg = "PING\n";
        if (write(sock_fd, msg.c_str(), msg.size()) != ssize_t(msg.size())) {
            perror("write");
            break;
        }
        std::cout << "[C] sent PING #" << (i + 1) << std::endl;

        if (DELAY_AFTER_PING > 0) {
            std::cout << "    sleeping " << DELAY_AFTER_PING << " sec after sending PING..." << std::endl;
            sleep(DELAY_AFTER_PING);
        }

        std::string resp;
        std::cout << "[C] waiting for PONG..." << std::endl;
        if (!read_line(sock_fd, resp)) {
            std::cout << "[client] server closed connection" << std::endl;
            break;
        }

        std::cout << "[C] received: '" << resp << "'" << std::endl;
        if (resp != "PONG") {
            std::cout << "[client] protocol error: expected PONG" << std::endl;
            break;
        }

        if (DELAY_AFTER_PONG > 0 && i + 1 < MAX_PINGS) {
            std::cout << "    sleeping " << DELAY_AFTER_PONG << " sec after receiving PONG..." << std::endl;
            sleep(DELAY_AFTER_PONG);
        }
    }

    std::cout << "[client] finished (sent " << MAX_PINGS << " PINGs)" << std::endl;
    close(sock_fd);
    return 0;
}
