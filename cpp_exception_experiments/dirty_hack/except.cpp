
#include <cstdio>


struct A
{
  bool fire;
  A() : fire(false) {}
  ~A()
  {
    printf("~A()\n");

    if (fire) {
      throw 666;
    }
  }
};

void foo()
{
  printf("foo start\n");
  {
    try {
      printf("foo:try\n");

      A a;
      a.fire = false;
      A b;
      b.fire = true;

      throw 1;

    } catch (int e) {
      printf("foo:catch %d\n", e);
    }
  }
  printf("foo end\n");
}

int main()
{

  try {
    foo();

  } catch (int e) {
    printf("main:catch %d\n", e);
  }

  printf("Finish\n");
}
