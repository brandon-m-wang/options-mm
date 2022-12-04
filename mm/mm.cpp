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
    if (*prevTime == time) {
        return false;
    }
    *prevTime = time;
    return true;
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
        return max(sp - os - 5, 0.0);
    } else if (oc == 'P') {
        return max(os - sp - 5, 0.0);
    } else {
        throw new std::invalid_argument(
            "Option CallPut must be either P or C.");
    }
}

double priceAsk(Option option, Stock stock) {
    MINIFY;
    if (oc == 'C') {
        return max(sp - os + 5, 0.0);
    } else if (oc == 'P') {
        return max(os - sp + 5, 0.0);
    } else {
        throw new std::invalid_argument(
            "Option CallPut must be either P or C.");
    }
}

/* COX-ROSS-RUBENSTEIN OPTIONS PRICING MODEL */

double price(int b, int y, double deltaT, double s, double k, double sigma,
             double r) {
    /*
     * b - switch (call: 1, put: -1)
     * y - years to expiration
     * deltaT - discrete timestep
     * s - stock price
     * k - strike price
     * sigma - annualized volatility
     * r - risk-free interest rate
     * u - upward move factor
     * d - downward move factor
     * p - probability of upward move
     * n - number of discrete timesteps
     * lambda - recombinant binomial lattice
     * e - exercise value at timestep
     */

    double u = exp(sigma * sqrt(deltaT));
    double d = exp(-sigma * sqrt(deltaT));
    double p = (exp(r * deltaT) - d) / (u - d);
    int n = y / deltaT;

    double lambda[n];

    for (int i = 0; i < n; i++) {
        lambda[i] = b * ((s * pow(u, 2 * i - n)) - k);
        lambda[i] = max(lambda[i], 0.0);
    }

    for (int j = n - 1; j >= 0; j--) {
        for (int i = 0; i < j; i++) {
            lambda[i] = p * lambda[i + 1] + (1 - p) * lambda[i];
            double e = b * ((s * pow(u, 2 * i - j)) - k);
            lambda[i] = max(lambda[i], e);
        }
    }

    return lambda[0];
}

double priceOption(Option option, Stock stock) {
    int b = option.callPut == 'C' ? 1 : -1;
    int y = stoi(getenv("YEARS_TO_EXPIRATION"));
    double deltaT = stod(getenv("DELTA_T"));
    double s = stock.price;
    double k = option.strike;
    double sigma = stod(getenv("ANNUALIZED_VOLATILITY"));
    double r = stod(getenv("RISK_FREE_RATE"));

    return price(b, y, deltaT, s, k, sigma, r);
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
    fstream tradedOptionsFstream;
    fstream stockFstream;
    const filesystem::path tradedOptionsPath =
        filesystem::current_path() / getenv("TRADED_OPTIONS_PATH");
    const filesystem::path stockPath =
        filesystem::current_path() / getenv("STOCK_PATH");

    MarketMaker(double equity, double startingVolume) {
        this->equity = equity;
        this->pnl = this->unrealized_gains = this->unrealized_losses =
            this->permanent_gains = this->permanent_losses = 0;

        this->tradedOptionsFstream = fstream(tradedOptionsPath, ios::in);
        this->stockFstream = fstream(stockPath, ios::in);

        string line;

        getline(stockFstream, line);
        Stock stock = stockFromFstream(line);

        if (tradedOptionsFstream.is_open()) {
            while (getline(tradedOptionsFstream, line)) {
                Option option = optionFromFstream(line);
                this->setOption(option, priceOption(option, stock),
                                priceOption(option, stock), startingVolume);
            }
            tradedOptionsFstream.close();
        }

        this->tradedOptionsFstream.clear();
        this->tradedOptionsFstream.seekg(0);
        this->stockFstream.clear();
        this->stockFstream.seekg(0);

        this->printOptions();
    }

    /* DEBUG */

    void printOptions() {
        for (auto &[option, priceVolume] : this->options) {
            cout << option.strike << " " << option.callPut << endl;
            cout << priceVolume["bid"] << " " << priceVolume["ask"] << " "
                 << priceVolume["volume"] << endl;
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
        string line;
        getline(stockFstream, line);
        Stock stock = stockFromFstream(line);

        for (auto &[option, priceVolume] : this->options) {
            this->setOptionBid(option, priceOption(option, stock));
            this->setOptionAsk(option, priceOption(option, stock));
        }
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
        filesystem::current_path() / getenv("ORDERBOOK_PATH");
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