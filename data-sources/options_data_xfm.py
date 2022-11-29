import os, sys
import pandas as pd

df = pd.read_csv(os.path.join(sys.path[0], 'AMZN_OPTIONS.csv'), 
                 index_col=False)

df_transformed = df.sort_values(['TimeBarStart', 'OpenAskTime']).fillna(0)

df_transformed.to_csv(os.path.join(sys.path[0], 'AMZN_OPTIONS_XFM.csv'),
                      index=False,
                      header=None,
                      columns=[
                        'Ticker', 
                        'TimeBarStart', 
                        'CallPut', 
                        'Strike',  
                        'ExpirationDate',
                        'HighBidPrice', 
                        'HighBidSize', 
                        'LowAskPrice', 
                        'LowAskSize'
                        ])
