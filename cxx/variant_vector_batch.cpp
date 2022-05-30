
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

typedef std::variant<A, B> Value;

const char* name(const Value& v)
{
    return std::visit(overload{
        [](const A&) { return "A"; },
        [](const B&) { return "B"; }
    }, v);
}

std::ostream& operator<<(std::ostream& os, const Value& v)
{
    os << name(v);
    return os;
}

void show_value()
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
    return std::visit(overload{
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
    std::visit(overload{
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



void show_batch()
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

        std::visit(overload{
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

void show_packB()
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
        std::visit(overload{
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

        std::visit(overload{
            [&bv, &value](const Value& last) {
                std::visit(overload{
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
                std::visit(overload{
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

void show_packBV()
{
    Values values{ A{}, B{}, A{}, A{}, A{}, B{}, B{}, A{} };

    BatchValues batches = packBV(values);

    std::cout << values << " -> " << batches << std::endl;
}



struct BatchPacker
{
    Batches batches;

    template<typename T>
    void operator()(std::vector<T>& last, const T& value)
    {
        last.emplace_back(value);
    }

    void operator()(const auto&, const auto& value)
    {
        batches.emplace_back(NewBatch(value));
    }

    void pack(const Values& values)
    {
        for (const auto& v : values) {
            if (batches.empty()) {
                batches.emplace_back(NewBatch(v));
                continue;
            }

            std::visit(*this, batches.back(), v);
        }
    }
};


void show_packer(auto&& packer)
{
    Values values_1{ A{}, B{}, A{}};

    packer.pack(values_1);

    Values values_2{ A{}, A{}, B{}, B{}, A{} };

    packer.pack(values_2);

    std::cout << values_1 << " + " << values_2 << " -> " << packer.batches << std::endl;
}


struct BVPacker
{
    BatchValues batches;

    template<typename T>
    void operator()(std::vector<T>& last, const T& value)
    {
        last.emplace_back(value);
    }

    template<typename T>
    void operator()(const T& last, const T& value)
    {
        auto batch = std::vector{last, value};
        batches.pop_back(); // last became invalid
        batches.emplace_back(batch);
    }

    void operator()(const auto&, const auto& value)
    {
        batches.emplace_back(value);
    }

    void pack(const Values& values)
    {
        for (const auto& v : values) {
            if (batches.empty()) {
                batches.emplace_back(v);
                continue;
            }

            std::visit(overload{
                [&v, this](auto& last) {
                    std::visit(*this, last, v);
                }
            }, batches.back());
        }
    }
};



int main()
{
    show_value();
    show_batch();

    show_packB();
    show_packBV();

    show_packer(BatchPacker{});
    show_packer(BVPacker{});

    return 0;
}

