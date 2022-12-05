## Virtual Options Market Maker

This is an exercise in:
- Design paradigms in C++
- Interprocess communication (IPC) on a Unix system
- Thread-safe programming
- Electronic market making techniques
- Binomial options pricing

This electronic options market maker makes a market around several options contracts at various strike prices derived from historical financial data. At each timestep granularity (1s), it provides two-sided quotes for each contract depending on the movement of the underlying stock. Quotes are provided through spread-setting and the Cox-Ross-Rubenstein options pricing model for American options via dynamic programming (binomial lattice).

For simplicity, we make a few assumptions:
- Annualized volatility of the security is fixed
- Risk-free interest rate is fixed
- The market making simulation runs over a single arbitrarily chosen day from open to close
- The simulated market data is tweaked to be more volatile in the short-term to allow for more apparent results
- No new contracts can be introduced in the middle of the simulation
- All contracts share the same expiry date
- All contracts are options for the same underlying security

PnL and unrealized PnL are calculated at each timestep.

Tutorial coming soon.
