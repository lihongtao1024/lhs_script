#include "lhs_vm.h"

int fibonacci(x)
{
    if (x < 2)
    {
        return 1;
    }

    return fibonacci(x - 1) + fibonacci(x - 2);
}

int main()
{
    for (int i = 0; i < 5; ++i)
    {
        printf("%d ", fibonacci(i));
    }

    LHSVM* vm = lhsvm_create(0);
    lhsvm_dofile(vm, "./test.lhs");
    lhsvm_destroy(vm);
    return 0;
}
