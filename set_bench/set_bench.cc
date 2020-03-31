#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <functional>
#include <thread>
#include <unordered_set>
#include <vector>
#include <typeinfo>
#include <deque>
#include <cstring>
#include <array>
#include <cassert>

void elapsed(std::function<void()> f, const std::string& s)
{
    auto start = std::chrono::steady_clock::now();
    f();
    std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
    std::cout << s << " " << elapsed.count() << " seconds" << std::endl;
}

template <typename T>
void fill(std::set<T>& s, T start, T step, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        s.emplace(start);
        start += step;
    }
}

template <typename T>
void fill(std::unordered_set<T>& s, T start, T step, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        s.emplace(start);
        start += step;
    }
}

template <typename T>
void fill(std::vector<T>& s, T start, T step, size_t count)
{
    //while(start < count) {
    for (size_t i = 0; i < count; i++) {
        s.push_back(start);
        start += step;
    }
    std::sort(s.begin(), s.end());
}

template <typename T>
void intersect(const std::set<T>& s1, const std::set<T>& s2, std::set<T>& result)
{
    auto ordered_refs = [&]()
    {
        if (s1.size() <= s2.size())
            return std::tie(s1, s2);
        else
            return std::tie(s2, s1);
    };

    auto [ l, r ] = ordered_refs();

    std::set_intersection(l.begin(), l.end(),
                            r.begin(), r.end(),
                            std::insert_iterator(result, result.begin()));
}

template <typename T>
void intersect(const std::unordered_set<T>& s1, const std::unordered_set<T>& s2, std::unordered_set<T>& result)
{
    //result.reserve(s1.size() / 4);
    auto ordered_refs = [&]()
    {
        if (s1.size() <= s2.size())
            return std::tie(s1, s2);
        else
            return std::tie(s2, s1);
    };

    auto [ l, r ] = ordered_refs();

    for (auto& v : l)
    {
        if (auto ifind = r.find(v) ; ifind != r.end())
            result.insert(*ifind);
    }
}

template <typename T>
void intersect(const std::vector<T>& v1, const std::vector<T>& v2, std::vector<T>& result)
{
    std::set_intersection(v1.begin(), v1.end(),
                            v2.begin(), v2.end(),
                            std::back_insert_iterator(result));
}

template <typename T, template<typename...> class C>
void bench(size_t count, T start, T step1, T step2)
{
    C<T> s1;
    C<T> s2;
    C<T> s3;

    std::string type_name = typeid(s1).name();

    elapsed([&](){fill<T>(std::ref(s1), start, step1, count);}, type_name + ": fill s1 took");
    elapsed([&](){fill<T>(std::ref(s2), start, step2, count);}, type_name + ": fill s1 took");

    std::cout << type_name << ": s1 length = " << s1.size() << ", s2 length = " << s2.size() << std::endl;

    elapsed([&](){intersect<T>(std::ref(s1), std::ref(s2), std::ref(s3));}, type_name + ": intersect s1 and s2 took");

    std::cout << type_name << ": s3 length = " << s3.size() << std::endl;
}

uint32_t hash6432shift(uint64_t key)
{
  key = (~key) + (key << 18); // key = (key << 18) - key - 1;
  key = key ^ (key >> 31);
  key = key * 21; // key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);
  return (uint32_t) key;
}


inline uint32_t hash_64_to_32_bit(uint64_t key)
{
    return key & 0xFFFFFFFF + (key >> 32);
    //return (key & 0x3FFFFFFF) + (key >> 34) + key >> 62;
    //return (key & 0xFFFFFFFF) / 11;
    //return ((key >> 32) ^ (key & 0xFFFFFFFF)) / 11;
}

class trie
{
#if 1
    typedef uint16_t LEVEL_TYPE;
    #define LEVEL_COUNT 4
    #define LEVEL_SIZE (256*256)
    #define LEVEL_MASK (0xFFFF)
    #define LEVEL_BIT_SHIFT 16
#else
    typedef uint8_t LEVEL_TYPE;
    #define LEVEL_COUNT 8
    #define LEVEL_SIZE (256)
    #define LEVEL_MASK (0xFF)
    #define LEVEL_BIT_SHIFT 8
#endif
    mutable int64_t iterator = -1;
    std::vector<int64_t> elements;

public:
    inline void insert(int64_t value)
    {
        this->elements.push_back(value);

#if 0
        uint32_t hash = hash_64_to_32_bit(value);
        //std::cout << "0x" << std::hex << v << " 0x" << v1 << std::dec << std::endl;

        uint16_t index1 = hash >> 16;
        uint16_t index0 = hash & 0xFFFF;

        trie_leaf* leaf = (trie_leaf*) root.table[index1];
        if (leaf == nullptr) {
            leaf = new trie_leaf();
            root.table[index1] = leaf;
        }
        leaf->set(index0, value);
#else
        size_t level = LEVEL_COUNT-1;
        trie_table* table = (trie_table*)&root;
        LEVEL_TYPE level_indexes[LEVEL_COUNT];
        memcpy(&level_indexes, &value, sizeof(level_indexes));
        LEVEL_TYPE index = level_indexes[level];

        trie_leaf* leaf;
        while (level > 0) {
            if (table->table[index] == 0) {
                if (level == 1) {
                    leaf = new trie_leaf();
                    table->table[index] = leaf;
                } else {
                    trie_table* t = new trie_table();
                    table->table[index] = t;
                    table = t;
                }
            } else {
                if (level == 1) {
                    leaf = (trie_leaf*)table->table[index];
                } else {
                    table = (trie_table*)table->table[index];
                }
            }
            level--;
            index = level_indexes[level];
        }
        leaf->set(index, value);
#endif
    }
    inline bool contains(int64_t value) const
    {
#if 0
        uint32_t hash = hash_64_to_32_bit(value);

        uint16_t index1 = hash >> 16;
        uint16_t index0 = hash & 0xFFFF;

        //std::cout << index1 << " " << index0 << std::endl;

        const trie_leaf* leaf = (const trie_leaf*) root.table[index1];
        return (leaf != nullptr) && leaf->get(index0, value);
#else
        size_t level = LEVEL_COUNT-1;
        const trie_table* table = (trie_table*)&root;
        LEVEL_TYPE level_indexes[LEVEL_COUNT];
        memcpy(&level_indexes, &value, sizeof(level_indexes));
        LEVEL_TYPE index = level_indexes[level];

        trie_leaf* leaf;
        while (level > 0) {
            if (table->table[index] != 0) {
                if (level == 1) {
                    leaf = (trie_leaf*)table->table[index];
                } else {
                    table = (trie_table*)table->table[index];
                }
            } else {
                return false;
            }
            level--;
            index = level_indexes[level];
        }
        return leaf->get(index, value);
#endif
    }
    inline void dump() const
    {
        this->root.dump();
    }
    inline size_t size() const
    {
        return this->elements.size();
    }
    inline void begin() const
    {
        iterator = 0;
    }
    inline int64_t next() const
    {
        if (iterator >= 0) {
            if (iterator < elements.size()) {
                int64_t e = elements[iterator];
                iterator++;
                return e;
            } else {
                iterator = -1;
            }
        }
        return iterator;
    }
    inline void _begin() const
    {
        this->iterator_index = 0;
    }
    inline int64_t _next() const
    {
        if (this->iterator_index < 0) {
            return -1;
        }

        int64_t v = this->iterator_index;

        const trie_table* level_tables[LEVEL_COUNT] = {0};
        size_t level_indexes[LEVEL_COUNT];
        for (size_t i = 0; i < LEVEL_COUNT; i++) {
            level_indexes[i] = ((v >> ((LEVEL_COUNT-1-i) * LEVEL_BIT_SHIFT)) & LEVEL_MASK);
        }
        size_t level = 0;
        level_tables[0] = (trie_table*)&root;

        while (v >= 0) {
            while (level_indexes[level] < LEVEL_SIZE) {
                if (level_tables[level]->table[level_indexes[level]] != 0) {
                    //std::cout << v << " " << level << " " << level_indexes[level]  <<  std::endl;
                    v &= ~(((uint64_t)LEVEL_MASK) << ((LEVEL_COUNT-1-level) * LEVEL_BIT_SHIFT));
                    v |= ((uint64_t)level_indexes[level] & LEVEL_MASK) << ((LEVEL_COUNT-1-level) * LEVEL_BIT_SHIFT);
                    //std::cout << v << std::endl;
                    if (level == LEVEL_COUNT-1) {
                        this->iterator_index = v + 1;
                        return v;
                    } else {
                        level_tables[level+1] = (trie_table*)level_tables[level]->table[level_indexes[level]];
                        level++;
                        break;
                    }
                }
                level_indexes[level]++;
            }
            if (level_indexes[level] >= LEVEL_SIZE) {
                if (level == 0) {
                    break;
                }
                level_indexes[level] = 0;
                level--;
                level_indexes[level]++;
            }
        }

        this->iterator_index = -1;
        return -1;
    }
private:
    struct trie_table
    {
        void* table[LEVEL_SIZE] = {0};
        inline void dump(int level = 0) const
        {
            for (size_t i = 0; i < LEVEL_SIZE; i++) {
                if (this->table[i] != 0) {
                    std::cout << level << " : " << (int)i << " : " << this->table[i] << std::endl;
                    if (level < LEVEL_COUNT-2) {
                        ((const trie_table*)(this->table[i]))->dump(level + 1);
                    } else {
                        ((const trie_leaf*)(this->table[i]))->dump();
                    }
                }
            }
        }
    } root;

    struct trie_leaf
    {
        uint64_t table[LEVEL_SIZE/64] = {0};
        //int64_t table_values[LEVEL_SIZE] = {0};

        inline void dump(int level = 0) const
        {
            for (size_t i = 0; i < LEVEL_SIZE/64; i++) {
                if (table[i] != 0) {
                    std::cout << i << ": " << table[i] << std::endl;
                }
            }
        }
        inline void set(LEVEL_TYPE v, int64_t value)
        {
            LEVEL_TYPE index = v >> 6;
            uint64_t table_mask = table[index];
            uint64_t value_mask = 0x1ull << (v & 0x3F);

            table[index] = value_mask | table_mask;

            //table_values[index] = value;
        }
        inline bool get(LEVEL_TYPE v, int64_t value) const
        {
            LEVEL_TYPE index = v >> 6;
            uint64_t table_mask = table[index];
            uint64_t value_mask = 0x1ull << (v & 0x3F);

            return (table_mask & value_mask) == value_mask;
        }

#if 0
        #define MAX_COLLISION 256
#if 0
        uint64_t table[LEVEL_SIZE/64] = {0};
        std::array<int64_t, MAX_COLLISION> collisions[LEVEL_SIZE/64];
        uint8_t col_count[LEVEL_SIZE/64] = {0};

        inline void dump(int level = 0) const
        {
            for (size_t i = 0; i < LEVEL_SIZE/64; i++) {
                if (table[i] != 0) {
                    std::cout << i << ": 0x" << std::hex << table[i] << std::dec << " = " << col_count[i] << ":";
                    for (uint8_t col_i = 0; col_i < col_count[i]; col_i++) {
                        std::cout << " " << collisions[i][col_i];
                    }
                    std::cout << std::endl;
                }
            }
        }

        inline void set(LEVEL_TYPE v, int64_t value)
        {
            LEVEL_TYPE index = v >> 6;
            uint64_t table_mask = table[index];
            uint64_t value_mask = 0x1ull << (v & 0x3F);

            table[index] = value_mask | table_mask;

            //collisions[index].push_back(value);
            uint8_t count = col_count[index];
            assert(count < MAX_COLLISION);
            collisions[index][count] = value;
            col_count[index] += 1;
        }

        inline bool get(LEVEL_TYPE v, int64_t value) const
        {
            //return 0 != (table[v >> 6] & (0x1ull << (v & 0x3F)));

            LEVEL_TYPE index = v >> 6;
            uint64_t table_mask = table[index];
            uint64_t value_mask = 0x1ull << (v & 0x3F);

            //std::cout << std::hex << table_mask << " & " << value_mask << " = " << (table_mask & value_mask) << std::dec << std::endl;
            if ((table_mask & value_mask) == value_mask) {
                //std::cout << "col_count[" << index << "] = " << col_count[index] << std::endl;
                const std::array<int64_t, MAX_COLLISION>& cols = collisions[index];
                for (uint8_t col_i = 0; col_i < col_count[index]; col_i++) {
                    //std::cout << value << " " << cols[col_i] << std::endl;
                    if (cols[col_i] == value) {
                        return true;
                    }
                }
                //return std::find(col_v.begin(), col_v.end(), value) != std::end(col_v);
            }
            return false;
        }

#else
        struct trie_leaf_item
        {
            uint64_t mask = 0;
            size_t count = 0;
            int64_t collisions[MAX_COLLISION] = {0};
        } table[LEVEL_SIZE/64];

        inline void dump(int level = 0) const
        {
            for (size_t i = 0; i < LEVEL_SIZE/64; i++) {
                if (table[i].mask != 0) {
                    std::cout << i << ": 0x" << std::hex << table[i].mask << std::dec << " = " << table[i].count << ":";
                    for (size_t col_i = 0; col_i < table[i].count; col_i++) {
                        std::cout << " " << table[i].collisions[col_i];
                    }
                    std::cout << std::endl;
                }
            }
        }
        inline void set(LEVEL_TYPE v, int64_t value)
        {
            LEVEL_TYPE index = v >> 6;
            uint64_t table_mask = table[index].mask;
            uint64_t value_mask = 0x1ull << (v & 0x3F);
            table[index].mask = value_mask | table_mask;

            //collisions[index].push_back(value);
            assert(table[index].count < MAX_COLLISION);
            table[index].collisions[table[index].count] = value;
            table[index].count += 1;
        }
        inline bool get(LEVEL_TYPE v, int64_t value) const
        {
            //return 0 != (table[v >> 6] & (0x1ull << (v & 0x3F)));

            LEVEL_TYPE index = v >> 6;
            uint64_t table_mask = table[index].mask;
            uint64_t value_mask = 0x1ull << (v & 0x3F);
            if (! (table_mask & value_mask)) {
                return false;
            } else {
                const int64_t* ptr = table[index].collisions;
                for (uint8_t i = 0; i < table[index].count; i++) {
                    if (ptr[i] == value) {
                        return true;
                    }
                }
                return false;
                //return std::find(col_v.begin(), col_v.end(), value) != std::end(col_v);
            }
        }
#endif
#endif
    };

    mutable int64_t iterator_index = 0;
};


void fill(trie& s, int64_t start, int64_t step, size_t count)
{
    //while(start < count){
    for (size_t i = 0; i < count; i++) {
        s.insert(start);
        start += step;
    }
}

void intersect(const trie& s1, const trie& s2, trie& result)
{
#if 1
    s1.begin();
    int64_t v = s1.next();
    while(v >= 0) {
        if (s2.contains(v)) {
            result.insert(v);
        }
        v = s1.next();
    }
#else
    s1.begin();
    s2.begin();
    int64_t v1 = s1.next();
    int64_t v2 = s1.next();
    while (v1 > 0 && v2 > 0) {
        if (v1 == v2) {
            result.insert(v1);
            v1 = s1.next();
            v2 = s1.next();
        } else if (v1 > v2) {
            v2 = s1.next();
        } else {
            v1 = s1.next();
        }
    }
#endif
}

void trie_test()
{
    trie t1;
    trie t2;

    t1.insert(0);
    t1.insert(1);
    t1.insert(1000000);
    t1.insert(10000000);
    t1.insert(100000000000);
    std::cout << std::endl;
    t1.dump();

    t1.begin();
    assert(t1.next() == 0);
    assert(t1.next() == 1);
    assert(t1.next() == 1000000);
    assert(t1.next() == 10000000);
    assert(t1.next() == 100000000000);
    assert(t1.next() == -1);

    assert(t1.contains(0));
    assert(t1.contains(1));
    assert(!t1.contains(5));
    assert(!t1.contains(100000));
    assert(t1.contains(1000000));
    assert(t1.contains(10000000));
    assert(!t1.contains(100000000));
    assert(!t1.contains(1000000000));

    t2.insert(1);
    t2.insert(5);
    t2.insert(1000000);
    t2.insert(1000000000);
    t2.insert(1000000000000);
    std::cout << std::endl;
    t2.dump();

    t2.begin();
    assert(t2.next() == 1);
    assert(t2.next() == 5);
    assert(t2.next() == 1000000);
    assert(t2.next() == 1000000000);
    assert(t2.next() == 1000000000000);
    assert(t2.next() == -1);

    assert(!t2.contains(0));
    assert(t2.contains(1));
    assert(t2.contains(5));
    assert(!t2.contains(100000));
    assert(t2.contains(1000000));
    assert(!t2.contains(10000000));
    assert(!t2.contains(100000000));
    assert(t2.contains(1000000000));

    trie t3;

    intersect(t1, t2, t3);

    std::cout << std::endl;
    std::cout << t3.size() << std::endl;
    t3.dump();
    assert(t3.size() == 2);
    assert(t3.contains(1));
    assert(t3.contains(1000000));

    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

/*
    TODO:
    - test sorted deque


*/

void trie_bench()
{
#if 0
    trie_test();
#else

    size_t count = 100*1000*1000;
    int64_t start = 8;//1000*1000*1000*1000ll;
    int64_t step1 = 13;
    int64_t step2 = 7;

    trie s1;
    trie s2;
    trie s3;

    std::string type_name = "trie";

    elapsed([&](){fill(std::ref(s1), start, step1, count);}, type_name + ": fill s1 took");
    elapsed([&](){fill(std::ref(s2), start, step2, count);}, type_name + ": fill s1 took");

    std::cout << type_name << ": s1 length = " << s1.size() << ", s2 length = " << s2.size() << std::endl;

    elapsed([&](){intersect(std::ref(s1), std::ref(s2), std::ref(s3));}, type_name + ": intersect s1 and s2 took");

    std::cout << type_name << ": s3 length = " << s3.size() << std::endl;

    //while (true) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#endif
}

int main()
{
#if 1
    trie_bench();
#endif

#if 1
    size_t count = 100*1000*1000;
    int64_t start = 8;//1000*1000*1000*1000ll;
    int64_t step1 = 13;// * 12345678;
    int64_t step2 = 7;// * 12345678;

    //bench<int64_t, std::set>(count, start, step1, step2);
    //bench<int64_t, std::unordered_set>(count, start, step1, step2);
    bench<int64_t, std::vector>(count, start, step1, step2);
#endif
}
