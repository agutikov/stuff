
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cassert>

struct trie_64_4
{
    struct ptr_table_t;
    struct bitmap_table_t;

    struct level_ptr_t
    {
        union {
            void* any = nullptr;
            ptr_table_t* table;
            bitmap_table_t* bitmap;
        } ptr;

        // number of elements in table or bitmap under ptr
        uint16_t count = 0;

        inline bool empty() const { return ptr.any == nullptr; }

        template <class T>
        inline void init()
        {
            ptr.any = T::_create();
            count = 0;
        }

        inline void _erase()
        {
            std::free(ptr.any);
            ptr.any = nullptr;
            count = 0;
        }

        template <class T>
        inline level_ptr_t* add_level(uint16_t idx)
        {
            level_ptr_t* t = ptr.table->_get(idx);
            t->init<T>();
            count++;
            return t;
        }

        inline level_ptr_t* get_level(uint16_t idx) const
        {
            return ptr.table->_get(idx);
        }

        template <class T>
        inline level_ptr_t* get_or_add_level(uint16_t idx)
        {
            level_ptr_t* t = ptr.table->_get(idx);
            if (t->empty()) {
                t->init<T>();
                count++;
            }
            return t;
        }

        inline bool ready_for_delete() const
        {
            return count == 0 && ptr.any != nullptr;
        }

        inline void remove_level(uint16_t idx)
        {
            level_ptr_t* t = ptr.table->_get(idx);
            if (! t->empty()) {
                t->_erase();
                count--;
            }
        }

        inline void set_bit(uint16_t idx)
        {
            if (! ptr.bitmap->_get(idx)) {
                ptr.bitmap->_set(idx);
                count++;
            }
        }

        inline bool get_bit(uint16_t idx)
        {
            return ptr.bitmap->_get(idx);
        }

        inline void unset_bit(uint16_t idx)
        {
            if (ptr.bitmap->_get(idx)) {
                ptr.bitmap->_unset(idx);
                count--;
            }
        }

    };

    struct ptr_table_t
    {
        level_ptr_t table[256*256];

        inline level_ptr_t* begin()
        {
            return table;
        }

        inline level_ptr_t* end()
        {
            return &table[256*256];
        }

        static inline ptr_table_t* _create()
        {
            //ptr_table_t* t = (ptr_table_t*) std::aligned_alloc(sizeof(ptr_table_t), sizeof(ptr_table_t));
            ptr_table_t* t = (ptr_table_t*) std::malloc(sizeof(ptr_table_t));
            std::memset(t->table, 0, sizeof(t->table));
            return t;
        }

        inline level_ptr_t* _get(uint16_t idx) const
        {
            return const_cast<level_ptr_t*>(&table[idx]);
        }


        inline int32_t _next(uint16_t idx) const
        {
            int32_t i = idx;
            while(i < 256*256) {
                //std::cout << "table " << i << std::endl;
                if (! table[i].empty()) {
                    //std::cout << "table return " << i << std::endl;
                    return i;
                }
                i++;
            }
            return -1;
        }
        inline int32_t _prev(uint16_t idx) const
        {
            int32_t i = idx;
            while(i >= 0) {
                if (table[i].count != 0) {
                    return i;
                }
                i--;
            }
            return -1;
        }
    };

    struct bitmap_table_t
    {
        uint64_t bitmap[256*256/64];

        inline uint64_t* begin()
        {
            return bitmap;
        }

        inline uint64_t* end()
        {
            return &bitmap[256*256/64];
        }


        static inline bitmap_table_t* _create()
        {
            //bitmap_table_t* b = (bitmap_table_t*) std::aligned_alloc(sizeof(bitmap_table_t), sizeof(bitmap_table_t));
            bitmap_table_t* b = (bitmap_table_t*) std::malloc(sizeof(bitmap_table_t));
            std::memset(b->bitmap, 0, sizeof(b->bitmap));
            return b;
        }

        inline void _set(uint16_t v)
        {
            bitmap[v >> 6] |= ((uint64_t)(0x01ull << (v & 0x3F)));
        }
        inline void _unset(uint16_t v)
        {
            bitmap[v >> 6] &= ~((uint64_t)(0x01ull << (v & 0x3F)));
        }
        inline bool _get(uint16_t v) const
        {
            return (bitmap[v >> 6] & ((uint64_t)(0x01ull << (v & 0x3F))) ) != 0;
        }

        inline int32_t _next(uint16_t v) const
        {
            uint16_t shift = v & 0x3F;
            int32_t index = v >> 6;
            while (index < (256*256/64)) {
                if (bitmap[index] != 0) {
                    while (shift < 64) {
                        if ((bitmap[index] & ((uint64_t)(0x01ull << shift))) != 0) {
                            //std::cout << "bitmap return " << index << " " << shift << std::endl;
                            return (index << 6) | shift;
                        }
                        shift++;
                    }
                    shift = 0;
                }
                index++;
            }
            return -1;
        }

        inline int32_t _prev(uint16_t v) const
        {
            int16_t shift = v & 0x3F;
            int32_t index = v >> 6;
            while (index >= 0) {
                if (bitmap[index] != 0) {
                    while (shift >= 0) {
                        if ((bitmap[index] & ((uint64_t)(0x01ull << shift))) != 0) {
                            return (index << 6) | shift;
                        }
                        shift--;
                    }
                    shift = 63;
                }
                index--;
            }
            return -1;
        }
    };

    struct index_t
    {
        union {
            uint64_t value = 0;
            // Little-endian (x86)
            // 0 - bitmap
            // 3 - root
            uint16_t levels[4];
        } u;

        inline index_t(uint64_t v)
        {
            u.value = v;
        }

        inline uint16_t get_lvl(int level) const
        {
            return u.levels[level];
        }

        inline void inc_level(int level)
        {
            u.value += (0x01ul << (level * 16));
        }

        inline void dec_level(int level)
        {
            u.value -= (0x01ul << (level * 16));
        }

        inline void reset_level(int level)
        {
            u.value &= ~((0x01ul << (level * 16)) - 1);
        }

        inline void r_reset_level(int level)
        {
            u.value &= ((0x01ul << (level * 16)) - 1);
        }

        inline index_t& operator = (uint64_t& v)
        {
            u.value = v;
            return *this;
        }
        explicit inline operator uint64_t() const
        {
            return u.value;
        }
        inline index_t& operator += (uint64_t v)
        {
            u.value += v;
            return *this;
        }
        inline index_t& operator -= (uint64_t v)
        {
            u.value -= v;
            return *this;
        }
    };

    struct itr_t
    {
        index_t idx;

        level_ptr_t* levels[4] = {0};

        uint64_t& value()
        {
            return idx.u.value;
        }

        inline itr_t(uint64_t v, level_ptr_t* l3) :
            idx(v)
        {
            levels[3] = l3;
        }

        inline bool traverse(uint64_t v)
        {
            idx = v;
            int level = 3;

            while (level > 0) {
                levels[level-1] = levels[level]->get_level(idx.get_lvl(level));
                if (levels[level-1]->empty()) {
                    return false;
                }
                level--;
            }

            return true;
        }

        inline void traverse_and_create(uint64_t v)
        {
            idx = v;
            int level = 3;

            while (level > 1) {
                levels[level-1] = levels[level]->get_or_add_level<ptr_table_t>(idx.get_lvl(level));
                level--;
            }
            levels[0] = levels[1]->get_or_add_level<bitmap_table_t>(idx.get_lvl(1));
        }

        /*
        *  Check if current index exist or find next
        */
        inline bool traverse_next()
        {
            int level = 3;

            while (level >= 0) {
                //std::cout << "level " << level << std::endl;

                if (level == 0) {
                    int32_t l0_idx = levels[0]->ptr.bitmap->_next(idx.get_lvl(0));
                    if (l0_idx < 0) {
                        //std::cout << "up" << std::endl;
                        // go up
                        level++;
                        idx.inc_level(1);
                        idx.reset_level(1);
                        continue;
                    }
                    idx.u.levels[0] = (uint16_t)l0_idx;
                    return true;
                } else {
                    levels[level-1] = levels[level]->get_level(idx.get_lvl(level));
                    if (levels[level-1]->empty()) {
                        //std::cout << "empty" << std::endl;
                        int32_t level_idx = levels[level]->ptr.table->_next(idx.get_lvl(level));
                        //std::cout << level_idx << std::endl;
                        if (level_idx < 0) {
                            if (level == 3) {
                                return false;
                            } else {
                                //std::cout << "up" << std::endl;
                                // go up
                                level++;
                                idx.inc_level(level);
                                idx.reset_level(level);
                                continue;
                            }
                        }
                        // go down
                        //std::cout << "down reset" << std::endl;
                        idx.u.levels[level] = (uint16_t)level_idx;
                        levels[level-1] = levels[level]->get_level(idx.get_lvl(level));
                        idx.reset_level(level);
                    }
                    // go down
                    //std::cout << "down" << std::endl;
                    level--;
                }
            }
            return false;
        }


        inline bool get_bit() const
        {
            if (!levels[0]->empty()) {
                return levels[0]->get_bit(idx.get_lvl(0));
            }
            return false;
        }

        inline void set_bit()
        {
            if (!levels[0]->empty()) {
                levels[0]->set_bit(idx.get_lvl(0));
            }
        }

        inline void unset_bit()
        {
            if (!levels[0]->empty()) {
                levels[0]->unset_bit(idx.get_lvl(0));
            }
        }

        inline void gc()
        {
            for (int level = 0; level < 3; level++) {
                if (levels[level]->ready_for_delete()) {
                    levels[level+1]->remove_level(idx.get_lvl(level+1));
                } else {
                    break;
                }
            }
        }

    };

    level_ptr_t _l3;

    itr_t get_initial_itr() const
    {
        return itr_t(0, const_cast<level_ptr_t*>(&_l3));
    }

    size_t total_count = 0;

    // If not empty _end will be _begin+1
    // TODO: implement iterator
    uint64_t _begin = 0;
    uint64_t _end = 0;

    inline void update_begin_end_add(uint64_t v)
    {
        if (total_count == 0) {
            _begin = v;
            _end = v+1;
        } else {
            if (_begin > v) {
                _begin = v;
            }
            if (_end <= v) {
                _end = v+1;
            }
        }
        total_count += 1;
    }

    inline void update_begin_end_rm(uint64_t v)
    {
        total_count -= 1;
        if (total_count == 0) {
            _begin = 0;
            _end = 0;
        } else {
            if (_begin == v) {
                _begin = next(v);
            }
            if (_end == v+1) {
                _end = prev(v)+1;
            }
        }
    }

    inline uint64_t begin() const { return _begin; }
    inline uint64_t end() const { return _end; }

    inline itr_t begin_itr() const
    {
        itr_t itr = get_initial_itr();

        itr.traverse(_begin);

        return itr;
    }

    inline size_t size() const { return total_count; }
    inline bool empty() const { return total_count == 0; }

    inline trie_64_4()
    {
        _l3.init<ptr_table_t>();
    }

    inline bool contains(uint64_t v) const
    {
        itr_t itr = get_initial_itr();

        if (itr.traverse(v)) {
            return itr.get_bit();
        }
        return false;
    }

    inline void add(uint64_t v)
    {
        itr_t itr = get_initial_itr();

        itr.traverse_and_create(v);
        itr.set_bit();

        update_begin_end_add(v);
    }

    inline void remove(uint64_t v)
    {
        itr_t itr = get_initial_itr();

        if (itr.traverse(v)) {
            itr.unset_bit();
            itr.gc();
            update_begin_end_rm(v);
        }
    }


    inline uint64_t next(uint64_t v) const
    {
        v++;

        itr_t itr = get_initial_itr();
        if (itr.traverse(v)) {
            if (itr.traverse_next()) {
                return (uint64_t)itr.idx;
            }
        }

        return _end;
    }

    inline uint64_t prev(uint64_t v) const
    {


        return 0;
    }



    inline void intersect_slow(const trie_64_4& left, const trie_64_4& right)
    {
        //std::cout << left.size() << " " << right.size() << std::endl;

        if (left.size() == 0 || right.size() == 0) {
            return;
        }

        const trie_64_4* big = &left;
        const trie_64_4* small = &right;

        if (small->size() > big->size()) {
            small = &left;
            big = &right;
        }

        itr_t x = small->begin_itr();

#if 1
        level_ptr_t* l3_end = x.levels[3]->ptr.table->end();
        level_ptr_t* l3_current = x.levels[3]->get_level(x.idx.get_lvl(3));

        level_ptr_t* l2_end = l3_current->ptr.table->end();
        level_ptr_t* l2_current = l3_current->get_level(x.idx.get_lvl(2));

        level_ptr_t* l1_end = l2_current->ptr.table->end();
        level_ptr_t* l1_current = l2_current->get_level(x.idx.get_lvl(1));

        uint64_t* l0_end = l1_current->ptr.bitmap->end();
        uint64_t* l0_current = & l1_current->ptr.bitmap->bitmap[x.idx.get_lvl(0)];

        uint64_t current_value = x.idx.u.value;

        uint64_t current_bitmap = *l0_current;
        uint64_t current_bit = 1ull << (current_value & 0x3F);


        while (l3_current < l3_end) {
            if (!l3_current->empty()) {

                l2_end = l3_current->ptr.table->end();
                l2_current = & l3_current->ptr.table->table[(current_value & 0xFFFF00000000) >> 32];

                while (l2_current < l2_end) {
                    if (!l2_current->empty()) {

                        l1_end = l2_current->ptr.table->end();
                        l1_current = & l2_current->ptr.table->table[(current_value & 0xFFFF0000) >> 16];

                        while (l1_current < l1_end) {
                            if (!l1_current->empty()) {

                                l0_end = l1_current->ptr.bitmap->end();
                                l0_current = & l1_current->ptr.bitmap->bitmap[(current_value & 0xFFFF) >> 6];

                                while (l0_current < l0_end) {
                                    current_bitmap = *l0_current;
                                    if (current_bitmap != 0) {
                                        while (current_bit != 0) {

                                            if ((current_bitmap & current_bit) != 0) {
                                                // visitor goes here with current_value
                                                //std::cout << current_value << std::endl;

                                                if (big->contains(current_value)) {
                                                    //std::cout << "add" << std::endl;
                                                    add(current_value);
                                                }

                                            }

                                            current_bit <<= 1;
                                            current_value++;
                                        } // while (current_bit != 0)
                                        current_bit = 1ull;
                                    } else {
                                        current_value += 64;
                                    } // if (current_bitmap > 0)

                                    l0_current++;
                                } // while (l0_current < l0_end)

                            } else {
                                current_value += 1ull << 16;
                            } // if (l1_current->ptr.bitmap != nullptr)

                            l1_current++;
                        } // while (l1_current < l1_end)

                    } else {
                        current_value += 1ull << 32;
                    } // if (l2_current->ptr.table != nullptr)

                    l2_current++;
                } // while (l2_current < l2_end)

            } else {
                current_value += 1ull << 48;
            } // if (l3_current->ptr.table != nullptr)

            l3_current++;
        } // while (l3_current < l3_end)

#else
        do {
            //std::cout << x.value() << std::endl;
            if (big->contains(x.value())) {
                //std::cout << "add" << std::endl;
                add(x.value());
            }
            x.value()++;
        } while (x.traverse_next());
#endif
    }

    static inline uint64_t kbi_popcount64(uint64_t y) // standard popcount; from wikipedia
    {
        y -= ((y >> 1) & 0x5555555555555555ull);
        y = (y & 0x3333333333333333ull) + (y >> 2 & 0x3333333333333333ull);
        return ((y + (y >> 4)) & 0xf0f0f0f0f0f0f0full) * 0x101010101010101ull >> 56;
    }

    inline void intersect_fast(const trie_64_4& left, const trie_64_4& right)
    {
        if (left.size() == 0 || right.size() == 0) {
            return;
        }

        level_ptr_t* left_l3_end = left._l3.ptr.table->end();
        level_ptr_t* left_l3_current = left._l3.ptr.table->begin();

        level_ptr_t* right_l3_current = right._l3.ptr.table->begin();

        level_ptr_t* l3_current = _l3.ptr.table->begin();

        while (left_l3_current < left_l3_end) {
            if (!left_l3_current->empty() && !right_l3_current->empty()) {

                l3_current->init<ptr_table_t>();
                _l3.count++;

                level_ptr_t* l2_current = l3_current->ptr.table->begin();

                level_ptr_t* left_l2_end = left_l3_current->ptr.table->end();
                level_ptr_t* left_l2_current = left_l3_current->ptr.table->begin();

                level_ptr_t* right_l2_current = right_l3_current->ptr.table->begin();

                while (left_l2_current < left_l2_end) {
                    if (!left_l2_current->empty() && !right_l2_current->empty()) {

                        l2_current->init<ptr_table_t>();
                        l3_current->count++;

                        level_ptr_t* l1_current = l2_current->ptr.table->begin();

                        level_ptr_t* left_l1_end = left_l2_current->ptr.table->end();
                        level_ptr_t* left_l1_current = left_l2_current->ptr.table->begin();

                        level_ptr_t* right_l1_current = right_l2_current->ptr.table->begin();

                        while (left_l1_current < left_l1_end) {
                            if (!left_l1_current->empty() && !right_l1_current->empty()) {

                                l1_current->init<bitmap_table_t>();
                                l2_current->count++;

                                uint64_t* l0_current = l1_current->ptr.bitmap->begin();

                                uint64_t* left_l0_end = left_l1_current->ptr.bitmap->end();
                                uint64_t* left_l0_current = left_l1_current->ptr.bitmap->begin();

                                uint64_t* right_l0_current = right_l1_current->ptr.bitmap->begin();

                                while (left_l0_current < left_l0_end) {

                                    uint64_t intersection = *left_l0_current & *right_l0_current;
                                    if (intersection != 0) {
                                        *l0_current = intersection;

                                        l1_current->count += kbi_popcount64(intersection);
                                    }

                                    left_l0_current++;
                                    right_l0_current++;
                                    l0_current++;
                                }
                                total_count += l1_current->count;
                            }

                            left_l1_current++;
                            right_l1_current++;
                            l1_current++;
                        }
                    }

                    left_l2_current++;
                    right_l2_current++;
                    l2_current++;
                }
            }

            left_l3_current++;
            right_l3_current++;
            l3_current++;
        }

    }




};























