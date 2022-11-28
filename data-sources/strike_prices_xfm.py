import os, sys
import pandas as pd

df = pd.read_csv(os.path.join(sys.path[0], 'AMZN_OPTIONS.csv'), 
                 index_col=False)

f = open(os.path.join(os.path.dirname(sys.path[0]), 'mm/STRIKE_PRICES'), 'w')

for ticker in df.Ticker.unique():
    for strike in df[(df['Ticker'] == ticker)].Strike.unique():
        for call_put in df[(df['Ticker'] == ticker) & (df['Strike'] == strike)].CallPut.unique():
            f.writelines([f"{ticker},{call_put},{strike},{expiration}\n" 
                        for expiration in df[
                            (df['Ticker'] == ticker) & 
                            (df['Strike'] == strike) & 
                            (df['CallPut'] == call_put)].ExpirationDate.unique()])
f.close()
