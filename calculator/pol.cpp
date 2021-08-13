#include <cctype>
#include <iostream>
#include <fstream>
#include <limits>
#include <memory>
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
#include <string_view>
#include <iterator>
using namespace std;



class split_iterator : public std::iterator<std::input_iterator_tag, std::string_view>
{
public:
    split_iterator(std::string_view src, std::string_view delimeters) :
        src(src),
        delimeters(delimeters)
    {
        operator++();
    }

    std::string_view operator* () const
    {
        return {src.data() + from_offset, to_offset - from_offset};
    }

    void operator++ ()
    {
        from_offset = to_offset;
        while (delimeters.find(src[from_offset]) != std::string::npos) {
            from_offset++;
        }
        to_offset = from_offset;
        while (to_offset < src.size() && delimeters.find(src[++to_offset]) == std::string::npos) ;
    }

    split_iterator begin() const
    {
        return *this;
    }

    struct sentinel {};

    sentinel end() const
    {
        return sentinel();
    }

    bool operator != (const sentinel& rhs) const
    {
        return to_offset < src.size() || from_offset < src.size();
    }

private:
    std::string_view src;
    std::string_view delimeters;
    size_t from_offset = 0;
    size_t to_offset = 0;
};


struct op_t
{
    virtual double calc() const = 0;
};

struct num_t : public op_t
{
    num_t(double v) :
        v(v)
    {}
    double v;
    double calc() const override
    {
        return v;
    }
};

struct bin_op_t : public op_t
{
    bin_op_t(std::shared_ptr<op_t> lhs, std::shared_ptr<op_t> rhs) :
        lhs(lhs), rhs(rhs)
    {}
    std::shared_ptr<op_t> lhs;
    std::shared_ptr<op_t> rhs;
};

struct sum_t : public bin_op_t
{
    sum_t(std::shared_ptr<op_t> lhs, std::shared_ptr<op_t> rhs) :
        bin_op_t(lhs, rhs)
    {}
    double calc() const override
    {
        return lhs->calc() + rhs->calc();
    }
};

struct diff_t : public bin_op_t
{
    diff_t(std::shared_ptr<op_t> lhs, std::shared_ptr<op_t> rhs) :
        bin_op_t(lhs, rhs)
    {}
    double calc() const override
    {
        return lhs->calc() - rhs->calc();
    }
};

struct mul_t : public bin_op_t
{
    mul_t(std::shared_ptr<op_t> lhs, std::shared_ptr<op_t> rhs) :
        bin_op_t(lhs, rhs)
    {}
    double calc() const override
    {
        return lhs->calc() * rhs->calc();
    }
};

struct division_t : public bin_op_t
{
    division_t(std::shared_ptr<op_t> lhs, std::shared_ptr<op_t> rhs) :
        bin_op_t(lhs, rhs)
    {}
    double calc() const override
    {
        return lhs->calc() / rhs->calc();
    }
};


std::pair<std::shared_ptr<op_t>, std::vector<std::string>::const_iterator>
parse(std::vector<std::string>::const_iterator curr, std::vector<std::string>::const_iterator end);


std::pair<std::pair<std::shared_ptr<op_t>, std::shared_ptr<op_t>>, std::vector<std::string>::const_iterator>
parse2(std::vector<std::string>::const_iterator curr, std::vector<std::string>::const_iterator end)
{
    auto r = parse(curr, end);
    auto lhs = r.first;
    curr = r.second;
    r = parse(curr, end);
    auto rhs = r.first;
    curr = r.second;
    return {{lhs, rhs}, curr};
}


std::pair<std::shared_ptr<op_t>, std::vector<std::string>::const_iterator>
parse(std::vector<std::string>::const_iterator curr, std::vector<std::string>::const_iterator end)
{
    if (curr == end) {
        throw std::invalid_argument("Expression is incomplete");
    }

    std::shared_ptr<op_t> node;

    auto& s = *curr;
    curr++;
    if (s == "+") {
        auto [args, next] = parse2(curr, end);
        curr = next;
        node = std::make_shared<sum_t>(args.first, args.second);
    } else if (s == "-") {
        auto [args, next] = parse2(curr, end);
        curr = next;
        node = std::make_shared<diff_t>(args.first, args.second);
    } else if (s == "*") {
        auto [args, next] = parse2(curr, end);
        curr = next;
        node = std::make_shared<mul_t>(args.first, args.second);
    } else if (s == "/") {
        auto [args, next] = parse2(curr, end);
        curr = next;
        node = std::make_shared<division_t>(args.first, args.second);
    } else {
        double v = std::stod(s);
        node = std::make_shared<num_t>(v);
    }

    return {node, curr};
}

std::shared_ptr<op_t> parse(const std::string& s)
{
    std::vector<std::string> tokens;
    for (auto token : split_iterator(s, " ")) {
        tokens.emplace_back(token.begin(), token.end());
    }

    auto [op, end] = parse(tokens.begin(), tokens.end());
    return op;
}




int main()
{
    auto op = parse("/ - 100 20 -4");
    printf("%f\n", op->calc());

    op = parse("+ + + 1 10 100 1000");
    printf("%f\n", op->calc());

    op = parse("+ - * / 1 2 3 4 5");
    printf("%f\n", op->calc());

    op = parse("+ * 1 2 - / 3 4 5");
    printf("%f\n", op->calc());

    op = parse("- 5 * 6 7");
    printf("%f\n", op->calc());

    return 0;
}
