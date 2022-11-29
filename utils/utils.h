#include <string>
#include <unordered_map>

namespace OptionTick {
enum {
    Ticker,
    TimeBarStart,
    CallPut,
    Strike,
    ExpirationDate,
    HighBidPrice,
    HighBidSize,
    LowBidPrice,
    LowBidSize,
    HighAskPrice,
    HighAskSize,
    LowAskPrice,
    LowAskSize,
    HighTradePrice,
    HighTradeSize,
    LowTradePrice,
    LowTradeSize,
    Volume
};
}

namespace StockTick {
enum {
    Ticker,
    TimeBarStart,
    HighTradePrice,
    HighTradeSize,
    LowTradePrice,
    LowTradeSize,
    Volume
};
}

namespace TradedOptionsTick {
enum { Ticker, CallPut, Strike, ExpirationDate };
}

using namespace std;

class Option {
  public:
    string ticker;
    char callPut;
    double strike;
    string expirationDate;

    Option(string ticker, char callPut, double strike, string expirationDate) {
        this->ticker = ticker;
        this->callPut = callPut;
        this->strike = strike;
        this->expirationDate = expirationDate;
    }
};

class Stock {
  public:
    string ticker;
    double price;

    Stock(string ticker, double price) {
        this->ticker = ticker;
        this->price = price;
    }
};

class OptionTick {
  public:
    string ticker;
    string timeBarStart;
    char callPut;
    double strike;
    string expirationDate;
    double highBidPrice;
    double highBidSize;
    double lowBidPrice;
    double lowBidSize;
    double highAskPrice;
    double highAskSize;
    double lowAskPrice;
    double lowAskSize;
    double highTradePrice;
    double highTradeSize;
    double lowTradePrice;
    double lowTradeSize;
    double volume;

    OptionTick(string ticker, string timeBarStart, char callPut, double strike,
               string expirationDate, double highBidPrice, double highBidSize,
               double lowBidPrice, double lowBidSize, double highAskPrice,
               double highAskSize, double lowAskPrice, double lowAskSize,
               double highTradePrice, double highTradeSize,
               double lowTradePrice, double lowTradeSize, double volume) {
        this->ticker = ticker;
        this->timeBarStart = timeBarStart;
        this->callPut = callPut;
        this->strike = strike;
        this->expirationDate = expirationDate;
        this->highBidPrice = highBidPrice;
        this->highBidSize = highBidSize;
        this->lowBidPrice = lowBidPrice;
        this->lowBidSize = lowBidSize;
        this->highAskPrice = highAskPrice;
        this->highAskSize = highAskSize;
        this->lowAskPrice = lowAskPrice;
        this->lowAskSize = lowAskSize;
        this->highTradePrice = highTradePrice;
        this->highTradeSize = highTradeSize;
        this->lowTradePrice = lowTradePrice;
        this->lowTradeSize = lowTradeSize;
        this->volume = volume;
    }
};

class OptionTick {
  public:
    string ticker;
    string timeBarStart;
    char callPut;
    double strike;
    string expirationDate;
    double highBidPrice;
    double highBidSize;
    double lowBidPrice;
    double lowBidSize;
    double highAskPrice;
    double highAskSize;
    double lowAskPrice;
    double lowAskSize;
    double highTradePrice;
    double highTradeSize;
    double lowTradePrice;
    double lowTradeSize;
    double volume;

    OptionTick(string ticker, string timeBarStart, char callPut, double strike,
               string expirationDate, double highBidPrice, double highBidSize,
               double lowBidPrice, double lowBidSize, double highAskPrice,
               double highAskSize, double lowAskPrice, double lowAskSize,
               double highTradePrice, double highTradeSize,
               double lowTradePrice, double lowTradeSize, double volume) {
        this->ticker = ticker;
        this->timeBarStart = timeBarStart;
        this->callPut = callPut;
        this->strike = strike;
        this->expirationDate = expirationDate;
        this->highBidPrice = highBidPrice;
        this->highBidSize = highBidSize;
        this->lowBidPrice = lowBidPrice;
        this->lowBidSize = lowBidSize;
        this->highAskPrice = highAskPrice;
        this->highAskSize = highAskSize;
        this->lowAskPrice = lowAskPrice;
        this->lowAskSize = lowAskSize;
        this->highTradePrice = highTradePrice;
        this->highTradeSize = highTradeSize;
        this->lowTradePrice = lowTradePrice;
        this->lowTradeSize = lowTradeSize;
        this->volume = volume;
    }
};

typedef std::unordered_map<std::string, double> price_volume;
typedef std::unordered_map<double, price_volume> expiration_to_price_volume;
typedef std::unordered_map<std::string, expiration_to_price_volume>
    strike_to_expiration;
typedef std::unordered_map<char, strike_to_expiration> call_put_to_strike;
typedef std::unordered_map<std::string, call_put_to_strike> options_map;
