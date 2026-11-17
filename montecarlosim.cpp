// Monte Carlo Trading Simulator
#include <iostream>
#include <random>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

// Monte Carlo Simulation Parameters
double capital = 0.0;
double initialCapital = 0.0;
double winRate = 0.0;
int trades = 0;
double profit = 0.0;
double loss = 0.0;
int WinningTrades = 0;
int LosingTrades = 0;
int simulations = 0;

// Random number generator setup
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> dist(0.0, 1.0);

// Function declarations
double expected_value();
double tradingSimulation();
double winrate_converter();
double percentile(double p);
vector<double> finalCapitals;

int main()
{
    cout << fixed << setprecision(2);

    cout << "Monte Carlo Trading Simulator" << endl;
    cout << "Please enter your trading data below" << endl;

    cout << "Enter the number of simulations: ";
    cin >> simulations;

    if (!cin || simulations <= 0)
    {
        cout << "Error: Invalid number of simulations." << endl;
        return 1;
    }

    cout << "Enter your capital: ";
    cin >> capital;

    cout << "Enter your WR (%): ";
    cin >> winRate;

    cout << "Enter your number of trades: ";
    cin >> trades;

    cout << "Enter your profit per trade: ";
    cin >> profit;

    cout << "Enter your loss per trade: ";
    cin >> loss;

    // Input validation
    if (!cin || capital < 0 || winRate < 0 ||
        winRate > 100 || trades <= 0 ||
        profit <= 0 || loss <= 0)
    {
        cout << "Error: Invalid input parameters." << endl;
        return 1;
    }

    initialCapital = capital;

    double wr = winrate_converter();
    double ev = expected_value();

    // Display simulation parameters

    cout << "\nStarting Trading Simulation...\n"
         << endl;

    cout << "Initial Capital: $" << initialCapital << endl;
    cout << "Win Rate: " << wr * 100 << "%" << endl;
    cout << "Number of Trades: " << trades << endl;
    cout << "Profit per Trade: $" << profit << endl;
    cout << "Loss per Trade: $" << loss << endl;
    cout << "Expected Value: $" << ev << endl;
    cout << "Expected Final Capital: $"
         << initialCapital + (ev * trades) << endl;

    cout << "\n--- SIMULATING TRADES ---\n"
         << endl;

    // Run simulations
    for (int j = 0; j < simulations; ++j)
    {
        capital = initialCapital;
        WinningTrades = 0;
        LosingTrades = 0;

        for (int i = 0; i < trades; ++i)
        {
            double tradeResult = tradingSimulation();
            capital += tradeResult;

            if (tradeResult > 0)
            {
                WinningTrades++;
            }
            else
            {
                LosingTrades++;
            }
        }
        finalCapitals.push_back(capital);
    }

    // Calculate and display average final capital
    double averageFinalCapital = 0;
    for (double cap : finalCapitals)
    {
        averageFinalCapital += cap;
    }
    averageFinalCapital /= finalCapitals.size();

    cout << "\n===== SIMULATION RESULTS =====\n"
         << endl;
    cout << "Average Final Capital: $" << averageFinalCapital << endl;

    sort(finalCapitals.begin(), finalCapitals.end());
    cout << "5th Percentile Final Capital: $" << percentile(0.05) << endl;
    cout << "50th Percentile Final Capital: $" << percentile(0.5) << endl;
    cout << "95th Percentile Final Capital: $" << percentile(0.95) << endl;

    return 0;
}

double percentile(double p)
{
    int k = p * (finalCapitals.size() - 1);
    return finalCapitals[k];
}

// Calculate expected profit per trade
double expected_value()
{
    double wr = winrate_converter();
    return (wr * profit) - ((1.0 - wr) * loss);
}

// Simulate a single trade
double tradingSimulation()
{
    double randomNumber = dist(gen);
    double wr = winrate_converter();

    if (randomNumber < wr)
    {
        return profit;
    }
    else
    {
        return -loss;
    }
}

// Convert percentage to decimal
double winrate_converter()
{
    return winRate / 100.0;
}
