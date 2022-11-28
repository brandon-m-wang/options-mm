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

#define MINIFY                                                                 \
    string t = option->ticker;                                                 \
    char c = option->callPut;                                                  \
    string e = option->expirationDate;                                         \
    double s = option->strike;

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

    void setOption(Option *option, double bid, double ask, double volume) {
        MINIFY;
        this->options[t][c][e][s]["bid"] = bid;
        this->options[t][c][e][s]["ask"] = ask;
        this->options[t][c][e][s]["volume"] = volume;
    }

    bool updateOption(Option *option, double volume) {
        MINIFY;
        return true;
    }

    bool removeOption(Option *option) {
        MINIFY;
        if (options.count(t) == 0 || options[t].count(c) == 0 ||
            options[t][c].count(e) == 0 || options[t][c][e].count(s) == 0) {
            return false;
        }
        options[t][c][e].erase(s);
        if (options[t][c][e].size() == 0) {
            options[t][c].erase(e);
        }
        if (options[t][c].size() == 0) {
            options[t].erase(c);
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

void init(MarketMaker *mm) {
    filesystem::path ifpath = filesystem::current_path() / "mm/STRIKE_PRICES";

    string line;

    fstream ifile(ifpath, ios::in);

    Option *option = new Option("AMZN", 'C', 1500.5, "20200130");

    mm->updateOption(option, 10);

    // if (ifile.is_open()) {
    //     while (getline(ifile, line)) {
    //     }
    //     ifile.close();
    // }
}

void process(vector<string> tick, MarketMaker *mm) {
    for (string attribute : tick) {
        cout << attribute << " ";
    }
    cout << endl;
}

int main() {

    MarketMaker *mm = new MarketMaker(100000);

    init(mm);

    // FIFO file path
    filesystem::path ofpath = filesystem::current_path() / "market/ORDERBOOK";
    const char *myfifo = ofpath.c_str();

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    mkfifo(myfifo, 0644);

    char buf[1028];
    vector<string> tick;
    // Open FIFO for Read only
    int fd = open(myfifo, O_RDONLY);
    while (1) {

        // Read from FIFO
        read(fd, buf, sizeof(buf));

        tick = tick_from_stream(buf);

        process(tick, mm);
    }
    close(fd);

    return 0;
}