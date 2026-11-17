# Monte Carlo Trading Simulator — C++

A lightweight Monte Carlo trading simulator developed from scratch in C++ to explore trading expectancy, stochastic equity outcomes, and risk distributions.

## Overview

The simulator generates thousands of independent trading scenarios based on user-defined parameters. Each simulation executes a sequence of randomly generated winning and losing trades, allowing users to compare theoretical expectancy with simulated results.

## Features

- Configurable initial capital
- User-defined win rate and number of trades
- Fixed profit and loss per trade
- Multiple independent Monte Carlo simulations
- Expected value calculation
- Average terminal equity
- Terminal equity distribution percentiles (5th, 50th, 95th)
- Basic input validation

## Mathematical Model

Each trade has two possible outcomes:

- Win: probability \(p\), profit \(W\)
- Loss: probability \(1-p\), loss \(L\)

Expected profit per trade:

\[
E[X]=pW-(1-p)L
\]

Expected terminal equity after \(N\) trades:

\[
E[C_N]=C_0+N E[X]
\]

The simulator uses `std::mt19937` for pseudorandom number generation.

## Requirements

- C++ compiler supporting C++11 or later
- Standard C++ libraries only

## Build and Run

Compile:

```bash
g++ -std=c++17 -O2 main.cpp -o simulator
```

Run on Linux/macOS:

```bash
./simulator
```

On Windows, compile with `-o simulator.exe` and run `.\simulator.exe` in PowerShell.

## Example Parameters

| Parameter | Value |
|---|---:|
| Initial capital | $10,000 |
| Win rate | 55% |
| Trades per simulation | 100 |
| Profit per win | $150 |
| Loss per loss | $100 |
| Simulations | 10,000 |

The theoretical expected terminal equity for these parameters is $13,750.

## Limitations

The current model assumes independent trade outcomes, a constant win probability, and fixed monetary profit/loss per trade. It does not account for commissions, slippage, changing market conditions, or position sizing.

## Planned Improvements

- Maximum drawdown analysis
- Probability of ruin
- Percentage-based position sizing
- CSV export of simulation results
- Performance benchmarking

## Disclaimer

This project is intended for educational and quantitative research purposes. Simulated results do not guarantee future trading performance.
