#include <cstdio>
#include <cassert>
#include <unordered_set>

struct Magic {
    Magic(Magic*& p_instance) {
        // assign non-const pointer to const pointer while it is not const yet
        p_instance = this;
    }
    int x = 12345;
};

struct Magic2 {
    Magic2() {
        objects.insert(this);
    }
    ~Magic2() {
        objects.erase(this);
    }
    int x = 999999;
    static std::unordered_set<Magic2*> objects;
};

std::unordered_set<Magic2*> Magic2::objects;

int main() {
    Magic* m;
    const Magic magic(m);
    assert(m == &magic);
    m->x = 42;
    assert(magic.x == 42);


    Magic2 m1;
    const Magic2 m2;

}











