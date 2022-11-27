#include <string>
#include <unordered_map>

namespace Options {
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

typedef std::unordered_map<std::string, double> price_volume;
typedef std::unordered_map<double, price_volume> strike_to_price_volume;
typedef std::unordered_map<std::string, strike_to_price_volume>
    expiration_to_strike;
typedef std::unordered_map<char, expiration_to_strike> call_put_to_expiration;
typedef std::unordered_map<std::string, call_put_to_expiration> options_map;
