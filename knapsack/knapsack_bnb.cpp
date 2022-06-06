#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <iostream>
#include <limits>
#include <iomanip>
#include <stack>
#include <fstream>
#include <sstream>


struct item_t
{
    size_t id;
    double weight;
    double profit;
};

bool density_gt(const item_t& left, const item_t& right)
{
    double d1 = left.profit / left.weight;
    double d2 = right.profit / right.weight;
    if (d1 == d2) {
        return left.profit > right.profit;
    }
    return d1 > d2;
}

struct knapsack_result_t
{
    knapsack_result_t(std::vector<item_t>& items) :
        items(items), picks(items.size(), false)
    {}

    knapsack_result_t& operator=(const knapsack_result_t& other)
    {
        items = other.items;
        picks = other.picks;
        count = other.count;
        P = other.P;
        W = other.W;
        return *this;
    }

    std::vector<item_t>& items;

    std::vector<bool> picks;
    size_t count = 0;
    double P = 0;
    double W = 0;

    void take(size_t index)
    {
        const item_t& item = items[index];
        count++;
        P += item.profit;
        W += item.weight;
        picks[index] = true;
    }

    void discard(size_t index)
    {
        const item_t& item = items[index];
        count--;
        P -= item.profit;
        W -= item.weight;
        picks[index] = false;
    }

    std::vector<std::pair<item_t, bool>> result() const
    {
        std::vector<std::pair<item_t, bool>> r;
        for (size_t i = 0; i < items.size(); i++) {
            r.emplace_back(items[i], picks[i]);
        }
        return r;
    }
};


struct knapsack_01
{
    std::vector<item_t> items;
    double capacity;

    knapsack_01(const std::vector<item_t>& _items, double c) :
        items(_items), capacity(c),
        max_p(items.size()),
        state(items), state_max(items), level_stats(items.size())
    {
        std::sort(items.begin(), items.end(), density_gt);

        double p_sum = 0;
        for (int i = items.size() - 1; i >= 0; i--) {
            p_sum += items[i].profit;
            max_p[i] = p_sum;
        }
    }

    std::vector<double> max_p;

    knapsack_result_t state;

    knapsack_result_t state_max;


    bool fit(size_t index) const
    {
        const item_t& item = items[index];
        return item.weight <= capacity - state.W;
    }

    size_t handle_result_calls = 0;
    size_t result_copies = 0;
    size_t cycles = 0;
    size_t leafs_touched = 0;

    struct stat_t
    {
        size_t node_enter_down = 0;
        size_t node_enter_up = 0;
        size_t left_branch = 0;
        size_t right_branch = 0;
        size_t bound = 0;
    };

    stat_t total_stats;
    std::vector<stat_t> level_stats;

    void handle_node_enter_down(size_t level)
    {
        total_stats.node_enter_down++;
        level_stats[level].node_enter_down++;
    }

    void handle_left_branch(size_t level)
    {
        total_stats.left_branch++;
        level_stats[level].left_branch++;
    }

    void handle_right_branch(size_t level)
    {
        total_stats.right_branch++;
        level_stats[level].right_branch++;
    }

    void handle_node_enter_up(size_t level)
    {
        total_stats.node_enter_up++;
        level_stats[level].node_enter_up++;
    }
    void handle_node_bound(size_t level)
    {
        total_stats.bound++;
        level_stats[level].bound++;
    }

    void handle_leaf()
    {
        leafs_touched++;
    }

    void print_total_stats() const
    {
        std::cout
        << "cycles = " << cycles << std::endl
        << "handle_result_calls = " << handle_result_calls << std::endl
        << "result_copies = " << result_copies << std::endl
        << "leafs_touched = " << leafs_touched << std::endl
        << "node_enter_down = " << total_stats.node_enter_down << std::endl
        << "node_enter_up = " << total_stats.node_enter_up << std::endl
        << "left_branch = " << total_stats.left_branch << std::endl
        << "right_branch = " << total_stats.right_branch << std::endl
        << "bound = " << total_stats.bound << std::endl
        ;
    }

    void print_level_stats() const
    {
        std::cout << "level, weight, profit, take,      max_p,"
            << "     down,       up,     left,    right,    bound"
            << std::endl;
        for (size_t index = 0; index < items.size(); index++) {
            const item_t& item = items[index];
            std::cout << std::setw(5) << index << ", "
                      << std::setw(6) << item.weight << ", "
                      << std::setw(6) << item.profit << ", "
                      << std::setw(4) << state_max.picks[index] << ", "
                      << std::setw(10) << max_p[index] << ", "
                      << std::setw(8) << level_stats[index].node_enter_down << ", "
                      << std::setw(8) << level_stats[index].node_enter_up << ", "
                      << std::setw(8) << level_stats[index].left_branch << ", "
                      << std::setw(8) << level_stats[index].right_branch << ", "
                      << std::setw(8) << level_stats[index].bound
                      << std::endl;
        }
    }

    void handle_result()
    {
        handle_result_calls++;

        if (state.count % 2 == 0) { // odd number of items in knapsack
            // return;
        }

        if (state.P > state_max.P
            || (state.P == state_max.P && state.W < state_max.W))
        {
            state_max = state;
            result_copies++;
        }
    }

    void optimize()
    {
        size_t index = 0;
        bool up = false;

        while (true) {
            cycles++;

            if (up) {
                if (index == 0) {
                    // end of three walk-through
                    break;
                }
                // go up
                index--;
                handle_node_enter_up(index);
                if (state.picks[index]) {
                    // handle result before discard item
                    handle_result();

                    // right branch
                    state.discard(index);

                    // go down
                    up = false;
                    handle_right_branch(index);
                    index++;
                } else {
                    // continue moving up
                    continue;
                }
            }

            if (index >= items.size()) {
                // no more items to process
                handle_leaf();
                up = true;
                continue;
            }
            if (state.W == capacity) {
                // up = true;
                // continue;
                //
                // already full
                //
                handle_result();
                break;
            }
            if (state_max.P >= state.P + max_p[index]) {
                // potential profit is less than already found one
                handle_node_bound(index);
                up = true;
                continue;
            }

            handle_node_enter_down(index);

            if (fit(index)) {
                // left branch
                state.take(index);
                handle_left_branch(index);
            } else {
                handle_right_branch(index);
            }

            // go down
            index++;
        }
    }

    auto result() const
    {
        return state_max.result();
    }

    void print() const
    {
        std::cout << "weight, profit, take,    max_p" << std::endl;
        for (size_t index = 0; index < items.size(); index++) {
            const item_t& item = items[index];
            std::cout << std::setw(6) << item.weight << ", "
                      << std::setw(6) << item.profit << ", "
                      << std::setw(4) << state_max.picks[index] << ", "
                      << std::setw(8) << max_p[index]
                      << std::endl;
        }
        std::cout << "count=" << state_max.count << ", W=" << state_max.W << ", P=" << state_max.P << std::endl;
    }
};


std::vector<item_t> read_csv(const std::string& filename)
{
    std::vector<item_t> items;

    std::ifstream ifs(filename);

    bool header = false;
    std::string line;
    size_t id = 0;
    while (std::getline(ifs, line)) {
        if (line.size() == 0) {
            continue;
        }
        if (items.size() == 0 && !header) {
            if (line != "weight,profit") {
                throw std::runtime_error("invalid csv header: \"" + line + "\"");
            } else {
                header = true;
                continue;
            }
        }
        std::stringstream ss(line);
        double w, p;
        ss >> w;
        while (ss.peek() == ',' || ss.peek() == ' ') {
            ss.ignore();
        }
        ss >> p;

        items.emplace_back(item_t{id, w, p});
        id++;
    }

    return items;
}


void save_csv(const char* filename, const std::vector<std::pair<item_t, bool>>& r)
{
    auto data = r;
    std::sort(data.begin(), data.end(), [](const std::pair<item_t, bool>& p1, const std::pair<item_t, bool>& p2) {
        return p1.first.id < p2.first.id;
    });

    std::ostream* os = &std::cout;
    std::ofstream ofs;
    if (filename) {
        ofs.open(filename, std::ios::out | std::ios::trunc);
        os = &ofs;
    }

    *os << "weight,profit,take" << std::endl;

    for (const auto& p : data) {
        *os << std::fixed << std::setprecision(1) << p.first.weight << ","
            << std::fixed << std::setprecision(1) << p.first.profit << ","
            << p.second << std::endl;
    }

    if (ofs.is_open()) {
        ofs.close();
    }
}


int main(int argc, const char* argv[])
{
    auto items = read_csv(argv[1]);
    double capacity = std::stod(argv[2]);

    knapsack_01 k{ items, capacity };
    k.optimize();
    k.print_level_stats();
    k.print_total_stats();
    std::cout << std::endl;

    auto r = k.result();
    save_csv(argv[3], r);


    return 0;
}
