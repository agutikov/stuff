#include <cstdio>

#if defined(__clang__)
#define JMP_SIZE "5"
#elif defined(__GNUC__)
#define JMP_SIZE "2"
#else
#error "compiler not supported"
#endif

#define THROW asm (\
    "mov 8(%rbp), %rax;" \
    "add $" JMP_SIZE ", %rax;" /* size of "jmp RETURN_SUCCESS_LABEL;" */ \
    "mov %rax, 8(%rbp);" \
    "pop %rbp;" \
    "ret;" \
);

#define CALL(sym) asm ( \
    "call " sym ";" \
    "jmp RETURN_SUCCESS_LABEL;" \
    "jmp RETURN_ERROR_LABEL;" \
    "RETURN_SUCCESS_LABEL:" \
);

int foo(int x) {
    if (x < 0) {
        // instead of throw
        // modify return address
        THROW;
    }
    return x + 1;
}

int test(int a) {
    int b = 0;
    if (a > 1000) {
        // like throw in the same function that catch
        goto RETURN_ERROR_LABEL;
    }
#if 0
    b = foo(a);
#else
    asm("movl %0, %%edi;" :: "r"(a));
    CALL("_Z3fooi");
    asm("mov %%eax, %0;" : "=r"(b));
#endif
    return b;
RETURN_ERROR_LABEL:
asm("RETURN_ERROR_LABEL:");
    return 99;
}

int main() {
    int x = test(10);
    printf("Normal return: %d\n", x);

    x = test(10000);
    printf("Exception in test(): %d\n", x);

    x = test(-1);
    printf("Exception in foo(): %d\n", x);

    return 0;
}