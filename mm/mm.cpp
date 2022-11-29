#include "../utils/utils.h"
#include <chrono>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <math.h>
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

/* MACROS */

#define MINIFY_OPTION                                                          \
    string ot = option->ticker;                                                \
    char oc = option->callPut;                                                 \
    string oe = option->expirationDate;                                        \
    double os = option->strike;

#define MINIFY_STOCK                                                           \
    string st = stock->ticker;                                                 \
    double sp = stock->price;

#define MINIFY                                                                 \
    MINIFY_OPTION;                                                             \
    MINIFY_STOCK;

/* MATH */

double approx(double highValue, double highSize, double lowValue,
              double lowSize) {
    return (highValue * highSize + lowValue * lowSize) / (highSize + lowSize);
}

/* DATA STREAM */

vector<string> tickFromBuffer(char *buf) {
    vector<string> tick;

    char delimiter[2] = ",";
    char *token = strtok(buf, delimiter);

    while (token != NULL) {
        tick.push_back(token);
        token = strtok(NULL, delimiter);
    }

    return tick;
}

Option *optionFromFstream(string line) {

    vector<string> tick = tickFromBuffer(line.data());

    return new Option(tick[TradedOptionsTick::Ticker],
                      tick[TradedOptionsTick::CallPut].front(),
                      stod(tick[TradedOptionsTick::Strike]),
                      tick[TradedOptionsTick::ExpirationDate]);
}

Stock *stockFromFstream(string line) {

    vector<string> tick = tickFromBuffer(line.data());

    return new Stock(tick[StockTick::Ticker],
                     approx(stod(tick[StockTick::HighTradePrice]),
                            stod(tick[StockTick::HighTradeSize]),
                            stod(tick[StockTick::LowTradePrice]),
                            stod(tick[StockTick::LowTradeSize])));
}

/* PRICING */

double priceBid(Option *option, Stock *stock) {
    MINIFY;
    if (oc == 'C') {
        return max(sp - os - 1, 0.0);
    } else if (oc == 'P') {
        return max(os - sp - 1, 0.0);
    } else {
        throw new std::invalid_argument(
            "Option CallPut must be either P or C.");
    }
}

double priceAsk(Option *option, Stock *stock) {
    MINIFY;
    if (oc == 'C') {
        return max(sp - os + 1, 0.0);
    } else if (oc == 'P') {
        return max(os - sp + 1, 0.0);
    } else {
        throw new std::invalid_argument(
            "Option CallPut must be either P or C.");
    }
}

/* MM CLASS */

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

    MarketMaker(double equity, double startingVolume) {
        this->equity = equity;
        this->pnl = this->unrealized_gains = this->unrealized_losses =
            this->permanent_gains = this->permanent_losses = 0;

        filesystem::path tradedOptionsPath =
            filesystem::current_path() / "mm/TRADED_OPTIONS";
        filesystem::path stockPath =
            filesystem::current_path() / "data-sources/AMZN_STOCK_XFM.csv";

        fstream tradedOptionsFstream(tradedOptionsPath, ios::in);
        fstream stockFstream(stockPath, ios::in);

        string line;

        getline(stockFstream, line);
        Stock *stock = stockFromFstream(line);

        if (tradedOptionsFstream.is_open()) {
            while (getline(tradedOptionsFstream, line)) {
                Option *option = optionFromFstream(line);
                this->setOption(option, priceBid(option, stock),
                                priceAsk(option, stock), startingVolume);
            }
            tradedOptionsFstream.close();
        }
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

    /* PROPAGATORS */

    void updateOptionFromTick(vector<string> tick) {}

    /* MUTATORS */

    void setOption(Option *option, double bid, double ask, double volume) {
        MINIFY_OPTION;
        this->options[ot][oc][oe][os]["bid"] = bid;
        this->options[ot][oc][oe][os]["ask"] = ask;
        this->options[ot][oc][oe][os]["volume"] = volume;
    }

    void updateOption(Option *option, double volume) { MINIFY_OPTION; }

    void removeOption(Option *option) {
        MINIFY_OPTION;
        if (options.count(ot) == 0 || options[ot].count(oc) == 0 ||
            options[ot][oc].count(oe) == 0 ||
            options[ot][oc][oe].count(os) == 0) {
            return;
        }
        options[ot][oc][oe].erase(os);
        if (options[ot][oc][oe].size() == 0) {
            options[ot][oc].erase(oe);
        }
        if (options[ot][oc].size() == 0) {
            options[ot].erase(oc);
        }
    }
};

/* RUN */

int main() {

    MarketMaker *mm = new MarketMaker(100000, 50);

    // FIFO file path
    filesystem::path orderbookPath =
        filesystem::current_path() / "market/ORDERBOOK";
    const char *orderbookPipe = orderbookPath.c_str();

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    mkfifo(orderbookPipe, 0644);

    char buf[1028];
    vector<string> tick;
    // Open FIFO for Read only
    int fd = open(orderbookPipe, O_RDONLY);
    while (1) {

        // Read from FIFO
        read(fd, buf, sizeof(buf));

        tick = tickFromBuffer(buf);
        cout << "tick!" << endl;
    }
    close(fd);

    return 0;
}