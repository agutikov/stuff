#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <functional>
#include <thread>
#include <tuple>
#include <string>

using namespace std::chrono_literals;

void elapsed(std::function<void()> f, const std::string& s)
{
    auto start = std::chrono::steady_clock::now();
    f();
    auto end = std::chrono::steady_clock::now();
    std::cout << s << " " << (end - start) / 1.0s << " seconds" << std::endl;
}

template <typename T>
struct myhash {
    size_t operator()(T x) const {
        return x / 5; // cheating to improve data locality
    }
};

template <typename T>
using myset = std::unordered_set<T, myhash<T>>;

template <typename T>
void fill_set(myset<T>& s, T start, T step, size_t count)
{
    s.reserve(count);
    for (size_t i = 0; i < count; i++) {
        s.emplace(start);
        start += step;
    }
}

template <typename T>
void intersect(const myset<T>& s1, const myset<T>& s2, myset<T>& result)
{
    result.reserve(s1.size() / 4); // cheating to compete with a better memory allocator
    for (auto& v : s1)
    {
        if (s2.find(v) != s2.end())
            result.insert(v);
    }
}

int main()
{
    myset<int64_t> s1;
    myset<int64_t> s2;
    myset<int64_t> s3;

    size_t count = 10*1000*1000;
    int64_t start = 1000*1000*1000*1000ll;
    int64_t step1 = 13 * 12345678;
    int64_t step2 = 7 * 12345678;

    elapsed(std::bind(fill_set<int64_t>, std::ref(s1), start, step1, count), "fill s1 took");
    elapsed(std::bind(fill_set<int64_t>, std::ref(s2), start, step2, count), "fill s2 took");

    std::cout << "s1 length = " << s1.size() << ", s2 length = " << s2.size() << std::endl;

    elapsed(std::bind(intersect<int64_t>, std::ref(s1), std::ref(s2), std::ref(s3)), "intersect s1 and s2 took");

    std::cout << "s3 length = " << s3.size() << std::endl;
}
