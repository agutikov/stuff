#include <cctype>
#include <iostream>
#include <fstream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <variant>
#include <optional>
using namespace std;


template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


struct eq_t;

typedef std::variant<double, eq_t> node_t;


double calculate(const node_t& node);


struct plus_t
{
    double operator()(const node_t& lhs, const node_t& rhs) const;
};

struct minus_t
{
    double operator()(const node_t& lhs, const node_t& rhs) const;
};

struct mult_t
{
    double operator()(const node_t& lhs, const node_t& rhs) const;
};

struct division_t
{
    double operator()(const node_t& lhs, const node_t& rhs) const;
};

typedef std::variant<plus_t, minus_t, mult_t, division_t> op_t;


struct eq_t
{
    std::vector<std::pair<op_t, node_t>> nodes;
    double calc() const
    {
        return std::accumulate(nodes.begin() + 1, nodes.end(), calculate(nodes[0].second),
            [](double v, const std::pair<op_t, node_t>& el) -> double
            {
                return std::visit(overloaded{
                        [&](auto arg) { return arg(v, el.second); },
                    }, el.first);
            }
        );
    }
};


double plus_t::operator()(const node_t& lhs, const node_t& rhs) const
{
    return calculate(lhs) + calculate(rhs);
}

double minus_t::operator()(const node_t& lhs, const node_t& rhs) const
{
    return calculate(lhs) - calculate(rhs);
}

double mult_t::operator()(const node_t& lhs, const node_t& rhs) const
{
    return calculate(lhs) * calculate(rhs);
}

double division_t::operator()(const node_t& lhs, const node_t& rhs) const
{
    return calculate(lhs) / calculate(rhs);
}


double calculate(const node_t& node)
{
    return std::visit(overloaded{
        [](double arg) { return arg; },
        [](const eq_t& arg) { return arg.calc(); },
    }, node);
}








int main()
{
    node_t e = eq_t{
        {
            {plus_t{}, 0.0},
            {minus_t{}, 1.0},
            {plus_t{}, eq_t{
                {
                    {mult_t{}, 7.0},
                    {mult_t{}, 3.0},
                }
            }},
        }
    };

    double v = calculate(e);

    printf("%f\n", v);

    return 0;
}
