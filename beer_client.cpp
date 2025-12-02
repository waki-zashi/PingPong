#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <chrono>
#include <thread>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <utility>

const char* SOCKET_PATH = "/tmp/pingpong.sock";
const int SIZE = 5;
const int CUPS = 5;

void delay() { std::this_thread::sleep_for(std::chrono::seconds(1)); }

struct Game {
    std::vector<std::pair<int, int>> cups;
    std::set<std::pair<int, int>> hit_by_opponent;
    std::set<std::pair<int, int>> my_shots;

    void place_cups() {
        cups.clear(); hit_by_opponent.clear(); my_shots.clear();
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> d(0, SIZE - 1);
        while (cups.size() < CUPS) {
            int x = d(rng), y = d(rng);
            std::pair<int, int> p{ x, y };
            if (std::find(cups.begin(), cups.end(), p) == cups.end())
                cups.push_back(p);
        }
        std::cout << "[CLIENT] My cups placed at:";
        for (auto [x, y] : cups) std::cout << " (" << x << "," << y << ")";
        std::cout << "\n\n";
    }

    std::string receive_shot(int x, int y) {
        std::pair<int, int> p{ x, y };
        if (hit_by_opponent.count(p)) return "ALREADY";
        hit_by_opponent.insert(p);

        auto it = std::find(cups.begin(), cups.end(), p);
        if (it == cups.end()) return "MISS";

        for (auto c : cups)
            if (hit_by_opponent.count(c) == 0)
                return "HIT";
        return "SUNK";
    }

    std::pair<int, int> make_shot() {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> d(0, SIZE - 1);
        int x, y;
        do {
            x = d(rng); y = d(rng);
        } while (my_shots.count({ x,y }));
        my_shots.insert({ x,y });
        return { x, y };
    }
};

bool read_line(int fd, std::string& s) {
    s.clear();
    char c;
    while (::read(fd, &c, 1) == 1) {
        if (c == '\n') return true;
        s += c;
    }
    return false;
}

int main() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }

    std::cout << "[CLIENT] Connected — placing cups...\n\n";

    Game game;
    game.place_cups();

    std::string msg;
    bool my_turn = true;

    while (true) {
        if (my_turn) {
            auto [x, y] = game.make_shot();
            std::cout << "Client shoots at (" << x << "," << y << ") — ";
            msg = "SHOT " + std::to_string(x) + "," + std::to_string(y) + "\n";
            write(sock, msg.c_str(), msg.size());

            if (!read_line(sock, msg)) break;
            std::cout << msg << "\n";

            if (msg == "SUNK") {
                delay();
                std::cout << "\nCLIENT WINS!\n";
                break;
            }
            delay();
            my_turn = false;

        }
        else {
            if (!read_line(sock, msg)) break;
            int x, y;
            if (sscanf(msg.c_str(), "SHOT %d,%d", &x, &y) != 2) continue;

            std::cout << "Server shoots at (" << x << "," << y << ") — ";
            std::string result = game.receive_shot(x, y);
            std::cout << result << "\n";

            write(sock, (result + "\n").c_str(), result.size() + 1);

            if (result == "SUNK") {
                delay();
                std::cout << "\nCLIENT DEFEATED! Client got drunk!\n";
                break;
            }
            delay();
            my_turn = true;
        }
    }

    close(sock);
    return 0;
}
