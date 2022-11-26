import os, sys
import pandas as pd

df = pd.read_csv(os.path.join(sys.path[0], 'AMZN_OPTIONS.csv'), 
                 index_col=False,
                 usecols=[
                     'Ticker', 
                     'TimeBarStart', 
                     'CallPut', 
                     'Strike', 
                     'HighBidPrice', 
                     'HighBidSize', 
                     'LowBidPrice', 
                     'LowBidSize', 
                     'HighAskPrice', 
                     'HighAskSize', 
                     'LowAskPrice', 
                     'LowAskSize', 
                     'HighTradePrice', 
                     'HighTradeSize', 
                     'LowTradePrice', 
                     'LowTradeSize', 
                     'Volume'
                     ])

df_transformed = pd.DataFrame()

for strike in df.Strike.unique():
    df_transformed = pd.concat([df_transformed, 
                              df[(df['Strike'] == strike) & 
                                 (df['CallPut'] == 'C')]])
    df_transformed = pd.concat([df_transformed, 
                              df[(df['Strike'] == strike) & 
                                 (df['CallPut'] == 'P')]])

df_transformed.to_csv(os.path.join(sys.path[0], 'AMZN_OPTIONS_XFM.csv'),
                      index=False)
