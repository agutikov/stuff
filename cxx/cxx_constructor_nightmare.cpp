//
// CppCon 2017: Nicolai Josuttis “The Nightmare of Move Semantics for Trivial Classes”
// https://www.youtube.com/watch?v=PNRju6_yn3o
//

#include <iostream>



struct Trace
{
    Trace() { std::cout << "    ctor" << std::endl; }
    Trace(const char* s) { std::cout << "    ctor(const char*)" << std::endl; }
    ~Trace() { std::cout << "    dtor" << std::endl; }
    Trace(const Trace&) { std::cout << "    copy ctor" << std::endl; }
    Trace& operator=(const Trace&) { std::cout << "    copy operator=" << std::endl; return *this; }
    Trace(Trace&&) { std::cout << "    move ctor" << std::endl; }
    Trace& operator=(Trace&&) { std::cout << "    move operator=" << std::endl; return *this; }

    void foo() const { std::cout << "    foo const" << std::endl; }
    void bar() { std::cout << "    bar" << std::endl; }
};


class Cust
{
public:
    template<typename S1, typename S2 = Trace>
    Cust(S1&& f, S2&& l = "", int i = 0) :
        first(std::forward<S1>(f)),
        last(std::forward<S2>(l)),
        id(i)
    {}

private:
    Trace first;
    Trace last;
    int id;
};

struct Cust2
{
    Trace first;
    Trace last = "";
    int id = 0;
};



int main()
{
    {
        Cust2 c{"Joe", "Fix", 42};
        std::cout << "=================================" << std::endl;

        Trace t = "Joe";
        std::cout << "=================================" << std::endl;

        Cust2 d{t, "Fix", 0};
        std::cout << "=================================" << std::endl;

        Cust2 e{std::move(t), "Fix", 0};
        std::cout << "=================================" << std::endl;

        Cust2 f{"Nico"};
        std::cout << "=================================" << std::endl;

        //Cust2 g = "xxx";
    }

    std::cout << "#################################################" << std::endl;

    {
        Cust c{"Joe", "Fix", 42};
        std::cout << "=================================" << std::endl;

        Trace t = "Joe";
        std::cout << "=================================" << std::endl;

        Cust d{t, "Fix", 0};
        std::cout << "=================================" << std::endl;

        Cust e{std::move(t), "Fix", 0};
        std::cout << "=================================" << std::endl;

        Cust f{"Nico"};
        std::cout << "=================================" << std::endl;

        Cust g = "xxx";
        std::cout << "=================================" << std::endl;
    }


    return 0;
}


