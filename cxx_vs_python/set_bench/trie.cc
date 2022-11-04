
#include "trie.hh"
#include <iostream>
#include <cassert>
#include <vector>
#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <functional>
#include <thread>


void ptr_table_test()
{
    trie_64_4::ptr_table_t t;

}

void bitmap_table_test()
{

}

void trie_test()
{
    std::vector<uint64_t> values = {
        0, 1, 2, 3, 4, 5,
        102, 500, 512, 550, 1000,
        30, 50, 100, 101,
        10, 32, 64, 65, 20,
        63,
    };
    std::vector<uint64_t> offsets = {
        1000000000, 1000000000000, 1000000000000000000,
        0, 10000, 256*256, 100000, 1000000,
    };


    trie_64_4 t, t1;
    size_t count = 0;
    assert(t.size() == count);

    for (auto offset : offsets) {
        for (auto value : values) {
            uint64_t v = offset + value;
            assert(!t.contains(v));
        }
    }

    for (auto offset : offsets) {
        for (auto value : values) {
            uint64_t v = offset + value;
            t.add(v);
            t1.add(v+1);
            count++;
            //std::cout << t.size() << " " << count << " " << v << std::endl;
            assert(t.size() == count);
        }
    }

#if 1
    std::cout << t.begin() << std::endl;
    std::cout << t.end() << std::endl;

    uint64_t x = t.begin();
    for (int i = 0; i < 200; i++) {
        x= t.next(x);
        //std::cout << x << std::endl;
    }

    //std::cout << "\n\n\n\n\n\n\n" << std::endl;

    trie_64_4 t2;
    t2.intersect_slow(t, t1);

    x = t2.begin();
    while(x < t2.end()) {
        //std::cout << x << std::endl;
        x = t2.next(x);
    }

    //std::cout << "size " << t2.size() << std::endl;
#endif

    for (auto offset : offsets) {
        for (auto value : values) {
            uint64_t v = offset + value;
            assert(!t.contains(v-123));
            assert(t.contains(v));
            //std::cout << v << std::endl;
            assert(!t.contains(v+123));
        }
    }


    for (auto offset : offsets) {
        for (auto value : values) {
            uint64_t v = offset + value;
            t.remove(v);
            count--;
            //std::cout << t.size() << " " << count << " " << v << std::endl;
            assert(t.size() == count);
        }
    }
    std::cout << t.size() << std::endl;

    for (auto offset : offsets) {
        for (auto value : values) {
            uint64_t v = offset + value;
            assert(!t.contains(v-123));
            assert(!t.contains(v-1));
            assert(!t.contains(v));
            assert(!t.contains(v+1));
            assert(!t.contains(v+123));
        }
    }



    std::cout << "Test succeeded" << std::endl;
}


void elapsed(std::function<void()> f, const std::string& s)
{
    auto start = std::chrono::steady_clock::now();
    f();
    std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
    std::cout << s << " " << elapsed.count() << " seconds" << std::endl;
}

void fill_set(trie_64_4& t, uint64_t start, uint64_t count, uint64_t step)
{
    for (uint64_t i = 0; i < count; i++) {
        t.add(start);
        start += step;
    }
}

int main()
{
    trie_test();

    trie_64_4 s1;
    trie_64_4 s2;

    elapsed(std::bind(fill_set, std::ref(s1), 0, 1000*1000*1000, 13), "fill s1 took");

    //while(true) std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    elapsed(std::bind(fill_set, std::ref(s2), 0, 1000*1000*1000, 7), "fill s2 took");

    std::cout << "s1 length = " << s1.size() << ", s2 length = " << s2.size() << std::endl;

#if 1
    trie_64_4 s3;

    elapsed(std::bind(&trie_64_4::intersect_slow, &s3, std::ref(s1), std::ref(s2)), "slow intersect s1 and s2 took");

    std::cout << "s3 length = " << s3.size() << std::endl;
#endif

#if 1
    trie_64_4 s4;

    elapsed(std::bind(&trie_64_4::intersect_fast, &s4, std::ref(s1), std::ref(s2)), "fast intersect s1 and s2 took");

    std::cout << "s4 length = " << s4.size() << std::endl;
#endif

    //while(true) std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    return 0;
}

