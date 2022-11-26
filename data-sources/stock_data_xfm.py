import os, sys
import pandas as pd

df = pd.read_csv(os.path.join(sys.path[0], 'AMZN_STOCK.csv'), 
                 index_col=False,
                 usecols=[
                     'Ticker',
                     'TimeBarStart', 
                     'HighTradePrice', 
                     'HighTradeSize', 
                     'LowTradePrice', 
                     'LowTradeSize', 
                     'Volume'
                     ])

df_transformed = df[(df['TimeBarStart'] >= '09:30') & 
                    (df['TimeBarStart'] <= '15:59')]

df_transformed.to_csv(os.path.join(sys.path[0], 'AMZN_STOCK_XFM.csv'), 
                      index=False, 
                      header=None)
