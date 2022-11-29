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
    LowAskPrice,
    LowAskSize
};
}

namespace StockTick {
enum {
    Ticker,
    TimeBarStart,
    HighTradePrice,
    HighTradeSize,
    LowTradePrice,
    LowTradeSize
};
}

namespace TradedOptionTick {
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

    bool operator==(const Option &otherOption) const {
        if (this->ticker == otherOption.ticker &&
            this->callPut == otherOption.callPut &&
            this->strike == otherOption.strike &&
            this->expirationDate == otherOption.expirationDate)
            return true;
        else
            return false;
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

struct OptionHashFunc {
    size_t operator()(const Option &option) const {
        return ((((hash<string>()(option.ticker) ^
                   (hash<char>()(option.callPut) << 1)) >>
                  1) ^
                 (hash<double>()(option.strike) << 1)) >>
                1) ^
               (hash<string>()(option.expirationDate) << 1);
    }
};

typedef unordered_map<string, double> price_volume;
typedef unordered_map<Option, price_volume, OptionHashFunc> options_map;
