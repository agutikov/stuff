#include "foo.h"

#if APP_VERSION == 2
#include "s_v2.h"
#else
#include "s_v1.h"
#endif

#include <cstdio>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <limits>
#include <random>
#include <unordered_set>
#include <iostream>
#include <iomanip>

using namespace std::chrono_literals;


struct S2
{
    const char* error = "ERROR";
    const char* msg = "S2 from app";
};

struct S3 : S1
{
    const char* error = "ERROR";
    const char* msg = "S3 from app";
};

struct S4
{
    const char* msg = "S4 from app";
};

struct S5 : S1
{
    const char* msg = "S5 from app";
};


static void throw_S1()
{
    printf("app throw S1");
    throw S1{"S1 from app"};
}

static void throw_S2()
{
    printf("app throw S2");
    throw S2{};
}

static void throw_S3()
{
    printf("app throw S3");
    throw S3{};
}

static void throw_S4()
{
    printf("app throw S4");
    throw S4{};
}

static void throw_S5()
{
    printf("app throw S5");
    throw S5{};
}

static std::map<std::string, type_info_t> app_types = {
    TYPE_MAP_ENTRY(S1),
    TYPE_MAP_ENTRY(S2),
    TYPE_MAP_ENTRY(S3),
    TYPE_MAP_ENTRY(S4),
    TYPE_MAP_ENTRY(S5)
};

template<typename E>
inline void app_handle_exception(const E& e)
{
    printf(" -> app caught: %s msg=\"%s\"\n", typeid(decltype(e)).name(), e.msg);
}

void app_try_catch(f_t f)
{
    try {
        f();
    } catch (const S5& e) {
        app_handle_exception(e);
    } catch (const S4& e) {
        app_handle_exception(e);
    } catch (const S3& e) {
        app_handle_exception(e);
    } catch (const S2& e) {
        app_handle_exception(e);
    } catch (const S1& e) {
        app_handle_exception(e);
    } catch (...) {
        printf(" -> app caught: unknown exception\n");
    }
}


void print_type(const type_info_t& type)
{
    printf("type %s, name=%s, hash=0x%016lX, size=%lu\n",
        type.local_name.c_str(),
        type.mangled_name.c_str(),
        type.type_info_hash,
        type.size
    );
}


template<typename KeyType, typename LeftValue, typename RightValue>
std::map<KeyType, std::pair<LeftValue, RightValue> >
intersect_maps(const std::map<KeyType, LeftValue>& left, const std::map<KeyType, RightValue>& right)
{
    std::map<KeyType, std::pair<LeftValue, RightValue> > result;
    auto il = left.begin();
    auto ir = right.begin();
    while (il != left.end() && ir != right.end()) {
        if (il->first < ir->first) {
            ++il;
        } else if (ir->first < il->first) {
            ++ir;
        } else {
            result.insert(std::make_pair(il->first, std::make_pair(il->second, ir->second)));
            ++il;
            ++ir;
        }
    }
    return result;
}


void lib_test()
{
    const auto& lib_types = get_lib_types();

    auto items = intersect_maps(lib_types, app_types);

    for (const auto& [name, types] : items) {
        auto& lib_type = types.first;
        auto& app_type = types.second;
        printf("%s:\n", name.c_str());
        printf("  lib: ");
        print_type(lib_type);
        printf("  app: ");
        print_type(app_type);
    }
    printf("\n");

    for (const auto& [_, types] : items) {
        f_t lib_f = types.first.throwing_f;
        f_t app_f = types.second.throwing_f;

        app_try_catch(app_f);
        lib_try_catch(lib_f);

        app_try_catch(lib_f);
        lib_try_catch(app_f);

        printf("\n");
    }
}

//TODO: number of times exception object copied

//TODO: collect Valgrind stats for exceptions benchmark
// number of instructions
// cache hit/miss
// ...

typedef int64_t Key_t;
typedef int64_t Value_t;


std::vector<Value_t> apply_map_with_try(const std::unordered_map<Key_t, Value_t>& m, const std::vector<Key_t>& keys, Value_t default_value)
{
    std::vector<Value_t> results;
    results.reserve(keys.size());
    for(Key_t key : keys) {
        try {
            results.emplace_back(m.at(key));
        } catch (const std::out_of_range& e) {
            results.emplace_back(default_value);
        }
    }
    return results;
}

std::vector<Value_t> apply_map_with_find(const std::unordered_map<Key_t, Value_t>& m, const std::vector<Key_t>& keys, Value_t default_value)
{
    std::vector<Value_t> results;
    results.reserve(keys.size());
    for(Key_t key : keys) {
        auto it = m.find(key);
        if (it != m.end()) {
            results.emplace_back(it->second);
        } else {
            results.emplace_back(default_value);
        }
    }
    return results;
}


std::vector<Value_t> apply_map_slow(const std::unordered_map<Key_t, Value_t>& m, const std::vector<Key_t>& keys, Value_t default_value)
{
    std::vector<Value_t> results;
    results.reserve(keys.size());
    for(Key_t key : keys) {
        if (m.contains(key)) {
            results.emplace_back(m.at(key));
        } else {
            results.emplace_back(default_value);
        }
    }
    return results;
}


std::random_device r;
std::default_random_engine e1(r());
std::uniform_int_distribution<Key_t> uniform_dist(0, std::numeric_limits<Key_t>::max());

std::pair<std::unordered_map<Key_t, Value_t>, std::vector<Key_t>>
prepare(size_t m_size, size_t k_hits_size, size_t k_miss_size)
{
    std::unordered_map<Key_t, Value_t> m;
    std::vector<Key_t> keys;
    std::unordered_set<Key_t> s;

    size_t keys_size = k_miss_size + k_hits_size;

    while (s.size() < keys_size) {
        s.insert(uniform_dist(e1));
    }

    auto it = s.begin();

    while (m.size() < k_hits_size) {
        Key_t k = *it;
        ++it;
        m[k] = 1;
        keys.push_back(k);
    }

    while (keys.size() < keys_size) {
        Key_t k = *it;
        ++it;
        keys.push_back(-k);
    }

    Key_t i = -1;
    while (m.size() < m_size) {
        m[i--] = 0;
    }

    return {m, keys};
}

std::string timeToString(std::chrono::system_clock::time_point t)
{
    std::time_t time = std::chrono::system_clock::to_time_t(t);
    std::string time_str = std::ctime(&time);
    time_str.resize(time_str.size() - 1);
    return time_str;
}

inline std::ostream& operator<< (std::ostream& s, std::chrono::system_clock::time_point tp)
{
    const auto t = std::chrono::system_clock::to_time_t(tp);
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()) % 1000000;
    s << std::put_time(std::localtime(&t), "%Y-%m-%dT%H:%M:%S") << '.' << std::setfill('0') << std::setw(6) << us.count();
    return s;
}

void exc_bench(size_t map_size, size_t keys_size, const std::vector<double>& percents, size_t times)
{
    printf("Map size, Keys size, miss%%, hits, miss, find() ms, try-catch ms, find/try, slow ms, slow/find, try/slow\n");
    for (double d : percents) {
        size_t hits = keys_size * (1.0 - d);
        size_t miss = keys_size - hits;

        //std::this_thread::sleep_for(1s);

        auto [m, k] = prepare(map_size, hits, miss);

        using clock_t = std::chrono::system_clock;

        double find_elapsed = 0.0;
        double try_catch_elapsed = 0.0;
        double slow_elapsed = 0.0;

        for (size_t i=0; i < times; i++) {
            auto started = clock_t::now();
            auto r = k; // apply_map_with_try(m, k, -1);
            auto finished = clock_t::now();

            std::cout << started << std::endl;
            std::cout << finished << std::endl;

            try_catch_elapsed += std::chrono::duration<double>(finished - started).count();

            if (r.size() != keys_size) {
                throw std::runtime_error("ERROR try");
            }
        }

        for (size_t i=0; i < times; i++) {
            auto started = clock_t::now();
            auto r = k; // apply_map_with_find(m, k, -1);
            auto finished = clock_t::now();

            find_elapsed += std::chrono::duration<double>(finished - started).count();

            if (r.size() != keys_size) {
                throw std::runtime_error("ERROR find");
            }
        }

        for (size_t i=0; i < times; i++) {
            auto started = clock_t::now();
            auto r = k; // apply_map_slow(m, k, -1);
            auto finished = clock_t::now();

            slow_elapsed += std::chrono::duration<double>(finished - started).count();

            if (r.size() != keys_size) {
                throw std::runtime_error("ERROR slow");
            }
        }

        find_elapsed /= times;
        try_catch_elapsed /= times;
        slow_elapsed /= times;

        printf("%lu, %lu, %.05g%%, %lu, %lu, %.03f, %.03f, %.02f, %.03f, %.03f, %.02f\n",
            m.size(), k.size(), d*100, hits, miss,
            find_elapsed * 1000, try_catch_elapsed * 1000, try_catch_elapsed / find_elapsed,
            slow_elapsed * 1000, slow_elapsed / find_elapsed, try_catch_elapsed / slow_elapsed
        );
    }
}

std::vector<double> setup{
    0.0,
    0.0,
    0.000001,
    0.00001,
    0.0001,
    0.001,
    0.002,
    0.005,
    0.01,
    0.02,
    0.05,
    0.1,
    0.2,
    0.5,
    1.0
};


int main()
{
    exc_bench(10'000'000, 100'000, setup, 1);


    return 0;
}
