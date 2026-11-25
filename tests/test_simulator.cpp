#define MONTECARLO_TESTING
#include "../montecarlosim.cpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

int main() {
    Parameters p;
    assert(std::abs(theoretical_mean(p) - 13750.0) < 1e-9);
    assert(std::abs(theoretical_variance(p) - 1546875.0) < 1e-6);
    assert(std::abs(std::sqrt(theoretical_variance(p)) - 1243.7346988) < 0.01);
    assert(std::abs(percentile({0, 10, 20, 30, 40}, 0.25) - 10.0) < 1e-12);
    assert(std::abs(percentile({0, 10}, 0.5) - 5.0) < 1e-12);

    const auto first = simulate(p);
    const auto second = simulate(p);
    assert(first.paths.size() == static_cast<std::size_t>(p.simulations));
    assert(first.mean == second.mean);
    assert(first.sample_variance == second.sample_variance);
    assert(first.paths.front().final_capital == second.paths.front().final_capital);
    assert(std::abs(first.mean - theoretical_mean(p)) < 80.0);
    assert(std::abs(first.sample_variance - theoretical_variance(p)) / theoretical_variance(p) < .06);

    // Deterministic all-wins case: no drawdown or ruin.
    p.win_probability = 1.0;
    p.trades = 4;
    p.simulations = 3;
    const auto all_wins = simulate(p);
    assert(all_wins.mean == p.capital + 4 * p.win_profit);
    assert(all_wins.sample_variance == 0.0);
    assert(all_wins.worst_drawdown == 0.0);
    assert(all_wins.ruin_probability == 0.0);

    // A breached threshold counts even if trading could subsequently recover.
    p.win_probability = 0.0;
    p.loss_amount = 3000.0;
    const auto all_losses = simulate(p);
    assert(all_losses.ruin_probability == 1.0);
    assert(all_losses.worst_drawdown > 1.0); // Negative balances possible in this toy model.

    bool rejected = false;
    p.simulations = 0;
    try { simulate(p); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    std::cout << "All mathematical and reproducibility tests passed.\n";
}
