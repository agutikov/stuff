
#include <initializer_list>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <variant>
#include <iostream>


template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
#if __cplusplus < 202000L
    // explicit deduction guide (not needed as of C++20)
    template<class... Ts> overload(Ts...) -> overload<Ts...>;
#endif


struct A
{};
struct B
{};
struct C
{};
struct D
{};


std::ostream& operator<<(std::ostream& os, const A& v)
{
    os << "A";
    return os;
}
std::ostream& operator<<(std::ostream& os, const B& v)
{
    os << "B";
    return os;
}
std::ostream& operator<<(std::ostream& os, const C& v)
{
    os << "C";
    return os;
}
std::ostream& operator<<(std::ostream& os, const D& v)
{
    os << "D";
    return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T>& values)
{
    os << "[";
    for (const auto& v : values) {
        os << v;
    }
    os << "]";
    return os;
}

template<typename... Types>
std::ostream& operator<<(std::ostream& os, std::variant<Types...>& value)
{
    std::visit(overload{ [&os](const auto& v) { os << v; } }, value);
    return os;
}

template<typename... Types>
std::ostream& operator<<(std::ostream& os, const std::vector<std::variant<Types...>>& values)
{
    os << "{";
    for (const auto& v : values) {
        os << v;
    }
    os << "}";
    return os;
}

template<typename... Types>
struct VVHelper
{
    typedef std::variant<Types...> Value;
    typedef std::vector<Value> Values;
    typedef std::variant<std::vector<Types>...> Vector;
    typedef std::variant<Value, Vector> PackedValue;
    typedef std::vector<PackedValue> PackedValues;
};

typedef VVHelper<A, B, C, D> Helper;
using Value = Helper::Value;
using Values = Helper::Values;
using Vector = Helper::Vector;
using PackedValues = Helper::PackedValues;

void show_values()
{
    Value a = A{};
    std::cout << a << std::endl;

    std::vector<A> v = { A{}, A{} };
    std::cout << v << std::endl;

    Vector v1 = std::vector<B>{ B{}, B{} };
    std::cout << v1 << std::endl;
}


#if 0

template<typename Types...>
struct VectorVariantPacker
{
    typedef std::variant<Types...> Value;
    typedef std::vector<Value> Values;

    typedef std::variant<std::vector<Types>...> Vector;

    typedef std::variant<Value, Vector> PackedValue;
    typedef std::vector<PackedValue> PackedValues;

    PackedValues batch;

    template<typename T>
    void operator()(std::vector<T>& last, const T& value)
    {
        last.emplace_back(value);
    }

    template<typename T>
    void operator()(const T& last, const T& value)
    {
        auto packed = std::vector{last, value};
        batch.pop_back(); // last became invalid
        batch.emplace_back(packed);
    }

    void operator()(const auto&, const auto& value)
    {
        batch.emplace_back(value);
    }

    void pack(const Values& values)
    {
        for (const auto& v : values) {
            if (batch.empty()) {
                batch.emplace_back(v);
                continue;
            }

            std::visit(overload{
                [&v, this](auto& last) {
                    std::visit(*this, last, v);
                }
            }, batch.back());
        }
    }
};

void show_packer()
{
    VectorVariantPacker<A, B, C, D> p;

    p::Values values_1{ A{}, B{}, C{}, D{}, A{}};

    p.pack(values_1);

    p::Values values_2{ A{}, A{}, B{}, B{}, A{} };

    p.pack(values_2);

    std::cout << values_1 << " + " << values_2 << " -> " << p.batch << std::endl;
}

#endif




int main()
{
    show_values();


    return 0;
}

