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

namespace TradedOptions {
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

double approx(double highValue, double highSize, double lowValue,
              double lowSize) {
    return (highValue * highSize + lowValue * lowSize) / (highSize + lowSize);
}

typedef std::unordered_map<std::string, double> price_volume;
typedef std::unordered_map<double, price_volume> expiration_to_price_volume;
typedef std::unordered_map<std::string, expiration_to_price_volume>
    strike_to_expiration;
typedef std::unordered_map<char, strike_to_expiration> call_put_to_strike;
typedef std::unordered_map<std::string, call_put_to_strike> options_map;
