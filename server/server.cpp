#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

const char* SOCKET_PATH = "/tmp/pingpong.sock";
const int   MAX_PINGS   = 5;

bool read_line(int fd, std::string &out) {
    out.clear();
    char ch;
    while (true) {
        ssize_t n = ::read(fd, &ch, 1);
        if (n == 0) {
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
    ::unlink(SOCKET_PATH);

    int listen_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (::bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        perror("bind");
        ::close(listen_fd);
        return 1;
    }

    if (::listen(listen_fd, 1) == -1) {
        perror("listen");
        ::close(listen_fd);
        return 1;
    }

    std::cout << "[server] waiting for client..." << std::endl;
    int conn_fd = ::accept(listen_fd, nullptr, nullptr);
    if (conn_fd == -1) {
        perror("accept");
        ::close(listen_fd);
        return 1;
    }

    std::cout << "[server] client connected, fd=" << conn_fd << std::endl;

    int ping_count = 0;
    std::string req;

    while (ping_count < MAX_PINGS) {
        std::cout << "[S1] waiting for request, processed pings: " << ping_count << std::endl;

        if (!read_line(conn_fd, req)) {
            std::cout << "[server] client disconnected (EOF)" << std::endl;
            break;
        }

        std::cout << "[S2.1] received: '" << req << "'" << std::endl;

        if (req == "PING") {
            std::string resp = "PONG\n";
            if (::write(conn_fd, resp.c_str(), resp.size()) == -1) {
                perror("write");
                break;
            }
            ++ping_count;
            std::cout << "[S3] sent PONG #" << ping_count << std::endl;
        } else {
            std::string resp = "ERROR: expected PING\n";
            std::cout << "[S2.2] protocol error, sending ERROR" << std::endl;
            if (::write(conn_fd, resp.c_str(), resp.size()) == -1) {
                perror("write");
                break;
            }
        }
    }

    std::cout << "[server] finished work, processed pings: " << ping_count
              << " (limit: " << MAX_PINGS << ")" << std::endl;

    ::close(conn_fd);
    ::close(listen_fd);
    ::unlink(SOCKET_PATH);
    return 0;
}
