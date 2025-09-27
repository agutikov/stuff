#include <iostream>
#include <cstdint>


struct Integer
{
    Integer(uint64_t e) : mantissa(0), exponent(e) {
        std::cout << "Integer ctor: mantissa: " << mantissa << ", exponent: " << exponent  << std::endl;
    }
    ~Integer() {
        std::cout << "Integer dtor: mantissa: " << mantissa << ", exponent: " << exponent  << std::endl;
    }
    Integer(const Integer& other) : mantissa(other.mantissa), exponent(other.exponent) {
        std::cout << "Integer copy: mantissa: " << mantissa << ", exponent: " << exponent  << std::endl;
    }
    Integer& operator=(const Integer& other) {
        mantissa = other.mantissa;
        exponent = other.exponent;
        std::cout << "Integer copy=: mantissa: " << mantissa << ", exponent: " << exponent  << std::endl;
        return *this;
    }
    Integer(Integer&& other) : mantissa(other.mantissa), exponent(other.exponent)  {
        std::cout << "Integer move: mantissa: " << mantissa << ", exponent: " << exponent  << std::endl;
    }
    Integer& operator=(Integer&& other) {
        mantissa = other.mantissa;
        exponent = other.exponent;
        std::cout << "Integer move=: mantissa: " << mantissa << ", exponent: " << exponent  << std::endl;
        return *this;
    }

    void setNumber(uint64_t n) {
        mantissa = n;
    }

private:
    uint64_t mantissa;
    int exponent;
};


Integer createI1(uint64_t n)
{
    if (n % 1024 == 0) {
        Integer v(10);
        v.setNumber(n / 1024);
        return v;
    } else if (n % 16 == 0) {
        Integer v(4);
        v.setNumber(n / 16);
        return v;
    } else {
        throw std::runtime_error("Not supported");
    }
}

Integer createI2(uint64_t n)
{
    Integer v(1);
    v.setNumber(n / 2);
    return v;
}


int main()
{
    auto a = createI1(512);
    auto b = createI2(1024);

    return 0;
}


