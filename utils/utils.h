#include <string>
#include <unordered_map>

namespace Options {
enum {
    Ticker,
    TimeBarStart,
    CallPut,
    Strike,
    HighBidPrice,
    HighBidSize,
    HighAskPrice,
    HighAskSize,
    HighTradePrice,
    HighTradeSize,
    LowBidPrice,
    LowBidSize,
    LowAskPrice,
    LowAskSize,
    LowTradePrice,
    LowTradeSize,
    Volume
};
}

namespace Stock {
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

typedef std::unordered_map<double, double *> strike_to_prices;
typedef std::unordered_map<std::string, strike_to_prices> expiration_to_strike;
typedef std::unordered_map<std::string, expiration_to_strike> options_map;
