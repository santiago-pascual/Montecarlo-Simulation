#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

struct Parameters {
    double capital = 10000.0;
    double win_probability = 0.55;
    double win_profit = 150.0;
    double loss_amount = 100.0;
    int trades = 100;
    int simulations = 10000;
    double ruin_fraction = 0.50;
    std::uint32_t seed = 42;
};

struct Path {
    double final_capital;
    double max_drawdown;
    bool hit_ruin;
};

struct Results {
    std::vector<Path> paths;
    double mean;
    double sample_variance;
    double sample_stddev;
    double mean_drawdown;
    double worst_drawdown;
    double ruin_probability;
};

double theoretical_mean(const Parameters& p) {
    return p.capital + p.trades * (p.win_probability * p.win_profit - (1.0 - p.win_probability) * p.loss_amount);
}

double theoretical_variance(const Parameters& p) {
    const double step = p.win_profit + p.loss_amount;
    return p.trades * p.win_probability * (1.0 - p.win_probability) * step * step;
}

// Linear interpolation between sorted observations (index = q * (n - 1)).
double percentile(std::vector<double> values, double q) {
    if (values.empty() || q < 0.0 || q > 1.0) throw std::invalid_argument("Invalid percentile");
    std::sort(values.begin(), values.end());
    const double index = q * (values.size() - 1);
    const auto lower = static_cast<std::size_t>(index);
    const auto upper = std::min(lower + 1, values.size() - 1);
    const double fraction = index - lower;
    return values[lower] + fraction * (values[upper] - values[lower]);
}

Results simulate(const Parameters& p) {
    if (!std::isfinite(p.capital) || p.capital <= 0 || !std::isfinite(p.win_probability) ||
        p.win_probability < 0 || p.win_probability > 1 || !std::isfinite(p.win_profit) ||
        p.win_profit <= 0 || !std::isfinite(p.loss_amount) || p.loss_amount <= 0 ||
        p.trades <= 0 || p.simulations < 2 || !std::isfinite(p.ruin_fraction) ||
        p.ruin_fraction <= 0 || p.ruin_fraction >= 1) {
        throw std::invalid_argument("Invalid parameters (capital > 0, trades > 0, simulations >= 2, 0 < ruin fraction < 1)");
    }
    std::mt19937 generator(p.seed);
    std::bernoulli_distribution win(p.win_probability);
    Results r{};
    r.paths.reserve(static_cast<std::size_t>(p.simulations));
    double sum = 0.0;
    double sum_drawdown = 0.0;
    int ruined = 0;
    for (int j = 0; j < p.simulations; ++j) {
        double equity = p.capital;
        double peak = equity;
        double max_dd = 0.0;
        bool ruin = false;
        for (int i = 0; i < p.trades; ++i) {
            equity += win(generator) ? p.win_profit : -p.loss_amount;
            peak = std::max(peak, equity);
            // The initial capital is positive; peak therefore stays positive.
            max_dd = std::max(max_dd, (peak - equity) / peak);
            if (equity <= p.capital * p.ruin_fraction) ruin = true;
        }
        r.paths.push_back({equity, max_dd, ruin});
        sum += equity;
        sum_drawdown += max_dd;
        r.worst_drawdown = std::max(r.worst_drawdown, max_dd);
        if (ruin) ++ruined;
    }
    r.mean = sum / p.simulations;
    double squared = 0.0;
    for (const auto& path : r.paths) squared += (path.final_capital - r.mean) * (path.final_capital - r.mean);
    r.sample_variance = squared / (p.simulations - 1);
    r.sample_stddev = std::sqrt(r.sample_variance);
    r.mean_drawdown = sum_drawdown / p.simulations;
    r.ruin_probability = static_cast<double>(ruined) / p.simulations;
    return r;
}

void export_csv(const std::string& filename, const Results& result) {
    std::ofstream out(filename);
    if (!out) throw std::runtime_error("Cannot open CSV for writing: " + filename);
    out << "simulation,final_capital,max_drawdown_pct,hit_ruin\n";
    out << std::fixed << std::setprecision(8);
    for (std::size_t i = 0; i < result.paths.size(); ++i) {
        const auto& path = result.paths[i];
        out << (i + 1) << ',' << path.final_capital << ',' << path.max_drawdown * 100.0 << ',' << (path.hit_ruin ? 1 : 0) << '\n';
    }
    if (!out) throw std::runtime_error("Error writing CSV");
}

#ifndef MONTECARLO_TESTING
int main(int argc, char** argv) {
    try {
        Parameters p;
        // Explicit numeric command line interface; defaults are the documented example.
        // Usage: simulator [simulations trades capital win_rate_percent profit loss seed [csv_path]]
        if (argc != 1 && argc != 8 && argc != 9) {
            std::cerr << "Usage: " << argv[0] << " [simulations trades capital win_rate_percent profit loss seed [csv_path]]\n";
            return 1;
        }
        if (argc >= 8) {
            p.simulations = std::stoi(argv[1]);
            p.trades = std::stoi(argv[2]);
            p.capital = std::stod(argv[3]);
            p.win_probability = std::stod(argv[4]) / 100.0;
            p.win_profit = std::stod(argv[5]);
            p.loss_amount = std::stod(argv[6]);
            const auto seed = std::stoull(argv[7]);
            if (seed > std::numeric_limits<std::uint32_t>::max()) throw std::invalid_argument("Seed out of range");
            p.seed = static_cast<std::uint32_t>(seed);
        }
        const auto r = simulate(p);
        std::vector<double> terminals;
        terminals.reserve(r.paths.size());
        for (const auto& path : r.paths) terminals.push_back(path.final_capital);
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Monte Carlo Trading Simulator (fixed-dollar trades)\n"
                  << "Seed: " << p.seed << " | Simulations: " << p.simulations << " | Trades: " << p.trades << '\n'
                  << "Theoretical mean: $" << theoretical_mean(p) << '\n'
                  << "Empirical mean:   $" << r.mean << '\n'
                  << "Theoretical variance: " << theoretical_variance(p) << '\n'
                  << "Empirical sample variance: " << r.sample_variance << '\n'
                  << "Theoretical stddev: $" << std::sqrt(theoretical_variance(p)) << '\n'
                  << "Empirical sample stddev: $" << r.sample_stddev << '\n'
                  << "P5 / P50 / P95: $" << percentile(terminals, .05) << " / $"
                  << percentile(terminals, .50) << " / $" << percentile(terminals, .95) << '\n'
                  << "Mean max drawdown: " << 100.0 * r.mean_drawdown << "%\n"
                  << "Worst max drawdown: " << 100.0 * r.worst_drawdown << "%\n"
                  << "Ruin threshold: " << 100.0 * p.ruin_fraction << "% of starting capital\n"
                  << "Probability of reaching ruin threshold: " << 100.0 * r.ruin_probability << "%\n";
        if (argc == 9) {
            export_csv(argv[8], r);
            std::cout << "Saved simulations to: " << argv[8] << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
#endif
