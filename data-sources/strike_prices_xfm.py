import os, sys
import pandas as pd

df = pd.read_csv(os.path.join(sys.path[0], 'AMZN_OPTIONS.csv'), 
                 index_col=False)

f = open(os.path.join(os.path.dirname(sys.path[0]), 'mm/STRIKE_PRICES'), 'w')

for strike in df.Strike.unique():
    print(strike)

f.writelines([str(x) + '\n' for x in df.Strike.unique()])
f.close()
