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
    return Stock(tick[StockTick::Ticker], stod(tick[StockTick::Price]));
}

Stock stockFromFstream(string line) {

    vector<string> tick = tickFromBuffer(line.data());

    return stockFromStockTick(tick);
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

double spread(double price) { return price * 0.01; }

/* MM CLASS */

class MarketMaker {
  public:
    options_map options;
    double pnl;
    double upnl;
    int trades;
    fstream tradedOptionsFstream;
    fstream stockFstream;
    const filesystem::path tradedOptionsPath =
        filesystem::current_path() / getenv("TRADED_OPTIONS_PATH");
    const filesystem::path stockPath =
        filesystem::current_path() / getenv("STOCK_PATH");

    MarketMaker() {
        this->pnl = this->upnl = 0;

        this->tradedOptionsFstream = fstream(tradedOptionsPath, ios::in);
        this->stockFstream = fstream(stockPath, ios::in);

        string line;

        getline(stockFstream, line);
        Stock stock = stockFromFstream(line);

        while (getline(tradedOptionsFstream, line)) {
            Option option = optionFromFstream(line);
            double price = priceOption(option, stock);
            this->initOption(option, price - spread(price),
                             price + spread(price));
        }

        tradedOptionsFstream.close();

        this->tradedOptionsFstream.clear();
        this->tradedOptionsFstream.seekg(0);
        this->stockFstream.clear();
        this->stockFstream.seekg(0);
    }

    /* TICK PROCESSING */

    void updatePnl(double lowAskPrice, double highBidPrice) {
        this->upnl = 0;
        double value = 0;
        double diff = 0;
        for (auto &[option, metadata] : this->options) {
            if (metadata["position"] == 0) {
                this->pnl += metadata["short"] - metadata["long"];
                metadata["short"] = metadata["long"] = 0;
            } else {
                diff += abs(metadata["short"] - metadata["long"]);
                if (metadata["position"] > 0) {
                    value += metadata["short"] - metadata["long"] +
                             (highBidPrice * metadata["position"]);
                } else {
                    value += metadata["short"] - metadata["long"] +
                             (lowAskPrice * metadata["position"]);
                }
            }
        }
        this->upnl = value / (abs(value) + diff);
    }

    void updatePosition(vector<string> tick) {
        Option option = optionFromOptionTick(tick);
        double lowAskPrice = stod(tick[OptionTick::LowAskPrice]);
        double lowAskSize = stod(tick[OptionTick::LowAskSize]);
        double highBidPrice = stod(tick[OptionTick::HighBidPrice]);
        double highBidSize = stod(tick[OptionTick::HighBidSize]);

        if (lowAskPrice <= this->options[option]["bid"]) {
            this->addLongPosition(option, lowAskSize);
            trades += lowAskSize;
        }
        if (highBidPrice >= this->options[option]["ask"]) {
            this->addShortPosition(option, highBidSize);
            trades += highBidSize;
        }
        this->updatePnl(lowAskPrice, highBidPrice);
    }

    void updatePrices(string time) {
        string line;

        getline(stockFstream, line);
        Stock stock = stockFromFstream(line);

        for (auto &[option, metadata] : this->options) {
            double price = priceOption(option, stock);
            this->setOptionBid(option, price - spread(price));
            this->setOptionAsk(option, price + spread(price));
        }
    }

    /* SECONDARY MUTATORS */

    void addLongPosition(Option option, double addition) {
        this->options[option]["long"] +=
            this->options[option]["bid"] * addition;
        setOptionPosition(option, this->options[option]["position"] + addition);
    }

    void addShortPosition(Option option, double reduction) {
        this->options[option]["short"] +=
            this->options[option]["ask"] * reduction;
        setOptionPosition(option,
                          this->options[option]["position"] - reduction);
    }

    /* MUTATORS */

    void setOptionBid(Option option, double bid) {
        this->options[option]["bid"] = bid;
    }

    void setOptionAsk(Option option, double ask) {
        this->options[option]["ask"] = ask;
    }

    void setOptionPosition(Option option, double deltaP) {
        this->options[option]["position"] = deltaP;
    }

    void initOption(Option option, double bid, double ask) {
        this->options[option]["bid"] = bid;
        this->options[option]["ask"] = ask;
        this->options[option]["position"] = 0;
        this->options[option]["long"] = 0;
        this->options[option]["short"] = 0;
    }
};

/* COMPUTATION */

bool updatePriceSignal(string *prevTime, string time) {
    if (*prevTime == time) {
        return false;
    }
    *prevTime = time;
    return true;
}

/* GUI */

void printOptionChain(MarketMaker *mm) {
    system("clear");
    int col = 0;
    int cols = 3;
    cout << fixed << setprecision(2);
    for (int i = 0; i < cols; i++) {
        cout << setw(9) << left;
        cout << "STRIKE";
        cout << setw(3) << left;
        cout << "T";
        cout << setw(10) << left;
        cout << "EXPIRY";
        cout << setw(9) << left;
        cout << "BID";
        cout << setw(9) << left;
        cout << "ASK";
        cout << setw(9) << left;
        cout << "POS";
        cout << setw(11) << left;
        cout << "LONG";
        cout << setw(11) << left;
        cout << "SHORT";
    }
    cout << endl << endl;
    for (auto &[option, metadata] : mm->options) {
        cout << setw(9) << left;
        cout << option.strike;
        cout << setw(3) << left;
        cout << option.callPut;
        cout << setw(10) << left;
        cout << option.expirationDate;
        cout << setw(9) << left;
        cout << metadata["bid"];
        cout << setw(9) << left;
        cout << metadata["ask"];
        cout << setw(9) << left;
        cout << metadata["position"];
        cout << setw(11) << left;
        cout << metadata["long"];
        cout << setw(11) << left;
        cout << metadata["short"];
        if ((col = ++col % cols) == 0) {
            cout << endl;
        }
    }
    cout << endl << endl;
    cout << "TRADES: ";
    cout << setw(12) << left;
    cout << mm->trades;
    cout << "PNL: ";
    cout << setw(14) << left;
    cout << mm->pnl;
    cout << "UNREALIZED PNL: ";
    cout << mm->upnl * 100 << "%" << endl;
}

/* RUN */

int main() {

    MarketMaker *mm = new MarketMaker();

    filesystem::path orderbookPath =
        filesystem::current_path() / getenv("ORDERBOOK_PATH");
    const char *orderbookPipe = orderbookPath.c_str();

    mkfifo(orderbookPipe, 0644);

    char buf[1028];
    vector<string> tick;
    string prevTime;

    int fd = open(orderbookPipe, O_RDONLY);

    while (read(fd, buf, sizeof(buf))) {

        tick = tickFromBuffer(buf);

        if (updatePriceSignal(&prevTime, tick[OptionTick::TimeBarStart])) {
            mm->updatePrices(tick[OptionTick::TimeBarStart]);
            printOptionChain(mm);
        }

        mm->updatePosition(tick);
    }

    close(fd);

    return 0;
}