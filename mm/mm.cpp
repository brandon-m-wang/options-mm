#include "../utils/utils.h"
#include <chrono>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

using namespace std;

class MarketMaker {
  public:
    options_map options;
    double equity;
    double pnl;
    double unrealized_gains;
    double unrealized_losses;
    double permanent_gains;
    double permanent_losses;
    int trades;

    MarketMaker(double equity) {
        this->equity = equity;
        this->pnl = this->unrealized_gains = this->unrealized_losses =
            this->permanent_gains = this->permanent_losses = 0;
    }

    void setOption(string ticker, string expiration, char type, double strike,
                   double *bid_asks) {
        options[ticker][expiration][strike] = bid_asks;
    }

    bool removeOption(string ticker, string expiration, double strike) {
        if (options.count(ticker) == 0 ||
            options[ticker].count(expiration) == 0 ||
            options[ticker][expiration].count(strike) == 0) {
            return false;
        }
        options[ticker][expiration].erase(strike);
        if (options[ticker][expiration].size() == 0) {
            options[ticker].erase(expiration);
        }
        if (options[ticker].size() == 0) {
            options[ticker].erase(expiration);
        }
        return true;
    }
};

vector<string> tick_from_stream(char *buf) {
    vector<string> tick;

    char delimiter[2] = ",";
    char *token = strtok(buf, delimiter);

    while (token != NULL) {
        tick.push_back(token);
        token = strtok(NULL, delimiter);
    }

    return tick;
}

void process(vector<string> tick) {
    for (string attribute : tick) {
        cout << attribute << " ";
    }
    cout << endl;
}

int main() {
    int fd;

    // FIFO file path
    filesystem::path ofpath = filesystem::current_path() / "market/ORDERBOOK";
    const char *myfifo = ofpath.c_str();

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    mkfifo(myfifo, 0644);

    char buf[1028];
    vector<string> tick;
    while (1) {

        // Open FIFO for Read only
        fd = open(myfifo, O_RDONLY);

        // Read from FIFO
        read(fd, buf, sizeof(buf));

        tick = tick_from_stream(buf);

        process(tick);
    }
    close(fd);

    return 0;
}