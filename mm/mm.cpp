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

    void printOptions() {
        for (auto const &[ticker, callPutToExpiration] : this->options) {
            cout << ticker << endl;
            for (auto const &[callPut, expirationToStrike] :
                 callPutToExpiration) {
                cout << "\t" << callPut << endl;
                for (auto const &[expiration, strikeToPriceVolume] :
                     expirationToStrike) {
                    cout << "\t\t" << expiration << endl;
                    for (auto const &[strike, price_volume] :
                         strikeToPriceVolume) {
                        cout << "\t\t\t" << strike << endl;
                        for (auto const &[key, value] : price_volume) {
                            cout << "\t\t\t\t" << key << ": " << value << endl;
                        }
                    }
                }
            }
        }
    }

    void setOption(string ticker, char callPut, string expiration,
                   double strike, double bid, double ask, double volume) {
        this->options[ticker][callPut][expiration][strike]["bid"] = bid;
        this->options[ticker][callPut][expiration][strike]["ask"] = ask;
        this->options[ticker][callPut][expiration][strike]["volume"] = volume;
    }

    bool removeOption(string ticker, char callPut, string expiration,
                      double strike) {
        if (options.count(ticker) == 0 || options[ticker].count(callPut) == 0 ||
            options[ticker][callPut].count(expiration) == 0 ||
            options[ticker][callPut][expiration].count(strike) == 0) {
            return false;
        }
        options[ticker][callPut][expiration].erase(strike);
        if (options[ticker][callPut][expiration].size() == 0) {
            options[ticker][callPut].erase(expiration);
        }
        if (options[ticker][callPut].size() == 0) {
            options[ticker].erase(callPut);
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

void process(vector<string> tick, MarketMaker *mm) {
    for (string attribute : tick) {
        cout << attribute << " ";
    }

    mm->setOption(tick[Options::Ticker], tick[Options::CallPut].front(),
                  tick[Options::ExpirationDate], stod(tick[Options::Strike]),
                  stod(tick[Options::LowBidPrice]),
                  stod(tick[Options::LowAskPrice]), 10);

    cout << endl;

    mm->printOptions();

    cout << endl;
}

int main() {

    MarketMaker *mm = new MarketMaker(100000);

    int fd;

    // FIFO file path
    filesystem::path ofpath = filesystem::current_path() / "market/ORDERBOOK";
    const char *myfifo = ofpath.c_str();

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    mkfifo(myfifo, 0644);

    char buf[1028];
    vector<string> tick;
    // Open FIFO for Read only
    fd = open(myfifo, O_RDONLY);
    while (1) {

        // Read from FIFO
        read(fd, buf, sizeof(buf));

        tick = tick_from_stream(buf);

        process(tick, mm);
    }
    close(fd);

    return 0;
}