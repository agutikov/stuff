
#include <initializer_list>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <variant>
#include <iostream>


template<class... Ts> struct visitor : Ts... { using Ts::operator()...; };
#if __cplusplus < 202000L
    // explicit deduction guide (not needed as of C++20)
    template<class... Ts> visitor(Ts...) -> visitor<Ts...>;
#endif



struct A
{};

struct B
{};

typedef std::variant<A, B> Value;

const char* name(const Value& v)
{
    return std::visit(visitor{
        [](const A&) { return "A"; },
        [](const B&) { return "B"; }
    }, v);
}

std::ostream& operator<<(std::ostream& os, const Value& v)
{
    os << name(v);
    return os;
}

void test_value()
{
    Value v = A{};

    std::cout << v << std::endl;
}


// Vector of variants
typedef std::vector<Value> Values;


std::ostream& operator<<(std::ostream& os, const Values& values)
{
    os << "{";
    for (const auto& v : values) {
        os << v;
    }
    os << "}";
    return os;
}


// Vectors for each type
typedef std::vector<A> Batch_A;
typedef std::vector<B> Batch_B;

// Variant of vectors for each types
typedef std::variant<Batch_A, Batch_B> Batch;

Batch NewBatch(const Value& v)
{
    return std::visit(visitor{
        [](const auto& v) -> Batch { return std::vector{v}; }
    }, v);
}

// TODO: Append


//TODO: any initializer_list to vector???
template<typename T>
Batch NewBatch(std::initializer_list<T>&& values)
{
    throw std::runtime_error("Not implemented");
}

std::ostream& operator<<(std::ostream& os, const Batch& batch)
{
    os << "[";
    std::visit(visitor{
        [&os](const auto& b) {
            for (const auto& v : b) {
                os << v;
            }
        }
    }, batch);
    os << "]";
    return os;
}


// TODO: operator<< for vector of variants
// TODO: operator<< for variant of vectors



void test_batch()
{
    Values values{ A{}, B{}, A{}, B{} };
    std::cout << values << std::endl;

    Batch batch_a = NewBatch(A{});
    Batch batch_b = NewBatch(B{});
    std::cout << batch_a << std::endl;
    std::cout << batch_b << std::endl;

    batch_a = Batch_A{ A{}, A{}, A{} };
    std::cout << batch_a << std::endl;
}


// Vector of batches
typedef std::vector<Batch> Batches;

std::ostream& operator<<(std::ostream& os, const Batches& batches)
{
    os << "{";
    for (const auto& batch : batches) {
        os << batch;
    }
    os << "}";
    return os;
}

Batches packB(const Values& values)
{
    Batches batches;

    for (const auto& v : values) {
        if (batches.empty()) {
            batches.emplace_back(NewBatch(v));
            continue;
        }

        std::visit(visitor{
            [](Batch_A& last, const A& value) {
                last.emplace_back(value);
            },
            [&batches](const Batch_A& last, const B& value) {
                batches.emplace_back(NewBatch(value));
            },
            [&batches](const Batch_B& last, const A& value) {
                batches.emplace_back(NewBatch(value));
            },
            [](Batch_B& last, const B& value) {
                last.emplace_back(value);
            }
        }, batches.back(), v);
    }

    return batches;
}

void test_packB()
{
    Values values{ A{}, B{}, A{}, A{}, A{}, B{}, B{}, A{} };

    Batches batches = packB(values);

    std::cout << values << " -> " << batches << std::endl;
}


// TODO: How to make a sum of variants type?

// Variant of variant of values or variant of vectors of values
typedef std::variant<Value, Batch> BatchValue;

// Vector of variants of variant of values or variant of vectors of values
typedef std::vector<BatchValue> BatchValues;

std::ostream& operator<<(std::ostream& os, const BatchValues& batches)
{
    os << "{";
    for (const auto& bv : batches) {
        std::visit(visitor{
            [&os](const auto& bv) {
                os << bv;
            }
        }, bv);
    }
    os << "}";
    return os;
}

BatchValues packBV(const Values& values)
{
    BatchValues bv;

    for (const auto& value : values) {
        if (bv.empty()) {
            bv.emplace_back(value);
            continue;
        }

        std::visit(visitor{
            [&bv, &value](const Value& last) {
                std::visit(visitor{
                    [&bv](const A& last, const A& value) {
                        auto batch = Batch_A{last, value};
                        bv.pop_back(); // last became invalid
                        bv.emplace_back(batch);
                    },
                    [&bv](const A& last, const B& value) {
                        bv.emplace_back(value);
                    },
                    [&bv](const B& last, const A& value) {
                        bv.emplace_back(value);
                    },
                    [&bv](const B& last, const B& value) {
                        auto batch = Batch_B{last, value};
                        bv.pop_back(); // last became invalid
                        bv.emplace_back(batch);
                    }
                }, last, value);
            },
            [&bv, &value](Batch& last) {
                std::visit(visitor{
                    [&bv](Batch_A& last, const A& value) {
                        last.emplace_back(value);
                    },
                    [&bv](const Batch_A& last, const B& value) {
                        bv.emplace_back(value);
                    },
                    [&bv](const Batch_B& last, const A& value) {
                        bv.emplace_back(value);
                    },
                    [&bv](Batch_B& last, const B& value) {
                        last.emplace_back(value);
                    }
                }, last, value);
            }
        }, bv.back());
    }

    return bv;
}

void test_packBV()
{
    Values values{ A{}, B{}, A{}, A{}, A{}, B{}, B{}, A{} };

    BatchValues batches = packBV(values);

    std::cout << values << " -> " << batches << std::endl;
}


int main()
{
    test_value();
    test_batch();
    test_packB();
    test_packBV();

    return 0;
}

