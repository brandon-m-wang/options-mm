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
    string ot = option.ticker;                                                 \
    char oc = option.callPut;                                                  \
    string oe = option.expirationDate;                                         \
    double os = option.strike;

#define MINIFY_STOCK                                                           \
    string st = stock.ticker;                                                  \
    double sp = stock.price;

#define MINIFY                                                                 \
    MINIFY_OPTION;                                                             \
    MINIFY_STOCK;

/* COMPUTATION */

double approx(double highValue, double highSize, double lowValue,
              double lowSize) {
    return (highValue * highSize + lowValue * lowSize) / (highSize + lowSize);
}

bool updatePriceSignal(string *prevTime, string time) {
    if (*prevTime != time) {
        *prevTime = time;
        return true;
    }
    return false;
}

int sinceMarketOpen(string time) {
    return stoi(time.substr(0, 2)) * 60 + stoi(time.substr(3, 2)) -
           (9 * 60 + 30);
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

Option optionFromOptionTick(vector<string> tick) {
    return Option(tick[OptionTick::Ticker], tick[OptionTick::CallPut].front(),
                  stod(tick[OptionTick::Strike]),
                  tick[OptionTick::ExpirationDate]);
}

Option optionFromTradedOptionTick(vector<string> tick) {
    return Option(tick[TradedOptionTick::Ticker],
                  tick[TradedOptionTick::CallPut].front(),
                  stod(tick[TradedOptionTick::Strike]),
                  tick[TradedOptionTick::ExpirationDate]);
}

Option optionFromFstream(string line) {

    vector<string> tick = tickFromBuffer(line.data());

    return optionFromTradedOptionTick(tick);
}

Stock stockFromStockTick(vector<string> tick) {
    return Stock(tick[StockTick::Ticker],
                 approx(stod(tick[StockTick::HighTradePrice]),
                        stod(tick[StockTick::HighTradeSize]),
                        stod(tick[StockTick::LowTradePrice]),
                        stod(tick[StockTick::LowTradeSize])));
}

Stock stockFromFstream(string line) {

    vector<string> tick = tickFromBuffer(line.data());

    return stockFromStockTick(tick);
}

/* PRICING */

double priceBid(Option option, Stock stock) {
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

double priceAsk(Option option, Stock stock) {
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
    filesystem::path tradedOptionsPath =
        filesystem::current_path() / "mm/TRADED_OPTIONS";
    filesystem::path stockPath =
        filesystem::current_path() / "data-sources/AMZN_STOCK_XFM.csv";

    MarketMaker(double equity, double startingVolume) {
        this->equity = equity;
        this->pnl = this->unrealized_gains = this->unrealized_losses =
            this->permanent_gains = this->permanent_losses = 0;

        fstream tradedOptionsFstream(tradedOptionsPath, ios::in);
        fstream stockFstream(stockPath, ios::in);

        string line;

        getline(stockFstream, line);
        Stock stock = stockFromFstream(line);

        if (tradedOptionsFstream.is_open()) {
            while (getline(tradedOptionsFstream, line)) {
                Option option = optionFromFstream(line);
                this->setOption(option, priceBid(option, stock),
                                priceAsk(option, stock), startingVolume);
            }
            tradedOptionsFstream.close();
        }
    }

    /* DEBUG */

    void printOptions() {
        for (auto &[option, price_volume] : this->options) {
            cout << option.strike << " " << option.callPut << endl;
            cout << price_volume["bid"] << " " << price_volume["ask"] << " "
                 << price_volume["volume"] << endl;
        }
    }

    /* PROPAGATORS */

    void updateOptionFromTick(vector<string> tick) {
        Option option = optionFromOptionTick(tick);
        if (stod(tick[OptionTick::LowAskPrice]) <=
            this->options[option]["bid"]) {
            this->addOptionVolume(option, stod(tick[OptionTick::LowAskSize]));
        }
        if (stod(tick[OptionTick::HighBidPrice]) >=
            this->options[option]["ask"]) {
            this->subOptionVolume(option, stod(tick[OptionTick::HighBidSize]));
        }
    }

    void updateOptionPrices(string time) {
        fstream stockFstream(stockPath, ios::in);

        string line;
        for (int i = 0; i < sinceMarketOpen(time); i++) {
            getline(stockFstream, line);
        }
        getline(stockFstream, line);
        Stock stock = stockFromFstream(line);

        // TODO: update all bid/ask prices.
    }

    /* SECONDARY MUTATORS */

    void addOptionVolume(Option option, double addition) {
        setOptionVolume(option, this->options[option]["volume"] + addition);
    }

    void subOptionVolume(Option option, double reduction) {
        setOptionVolume(option, this->options[option]["volume"] - reduction);
    }

    /* MUTATORS */

    void setOptionBid(Option option, double bid) {
        this->options[option]["bid"] = bid;
    }

    void setOptionAsk(Option option, double ask) {
        this->options[option]["ask"] = ask;
    }

    void setOptionVolume(Option option, double volume) {
        this->options[option]["volume"] = volume;
    }

    void setOption(Option option, double bid, double ask, double volume) {
        this->options[option]["bid"] = bid;
        this->options[option]["ask"] = ask;
        this->options[option]["volume"] = volume;
    }

    void removeOption(Option option) { this->options[option].clear(); }
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
    string prevTime;
    // Open FIFO for Read only
    int fd = open(orderbookPipe, O_RDONLY);
    while (1) {

        // Read from FIFO
        read(fd, buf, sizeof(buf));

        tick = tickFromBuffer(buf);

        if (updatePriceSignal(&prevTime, tick[OptionTick::TimeBarStart])) {
            mm->updateOptionPrices(tick[OptionTick::TimeBarStart]);
        }

        mm->updateOptionFromTick(tick);
    }
    close(fd);

    return 0;
}