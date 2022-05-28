#include <iostream>
#include <optional>



struct Trace
{
    Trace() { std::cout << "    ctor" << std::endl; }
    ~Trace() { std::cout << "    dtor" << std::endl; }
    Trace(const Trace&) { std::cout << "    copy ctor" << std::endl; }
    Trace& operator=(const Trace&) { std::cout << "    copy operator=" << std::endl; return *this; }
    Trace(Trace&&) { std::cout << "    move ctor" << std::endl; }
    Trace& operator=(Trace&&) { std::cout << "    move operator=" << std::endl; return *this; }

    void foo() const { std::cout << "    foo const" << std::endl; }
    void bar() { std::cout << "    bar" << std::endl; }
};


void Foo(const Trace& t)
{
    t.foo();
}

void Bar(Trace& t)
{
    t.bar();
}

int Baz(Trace& t)
{
    return 123;
}


typedef void(*void_trace_f)(Trace& t);

template <typename T, void_trace_f f>
void optional_apply(std::optional<T>& v)
{
    if (v.has_value()) {
        f(v.value());
    }
}


typedef void(*void_const_trace_f)(const Trace& t);

template <typename T, void_const_trace_f f>
void optional_apply(const std::optional<T>& v)
{
    if (v.has_value()) {
        f(v.value());
    }
}




int main()
{
    std::optional<Trace> t1(std::in_place);

    optional_apply<Trace, Bar>(t1);
    optional_apply<Trace, Foo>(t1);


    return 0;
}
