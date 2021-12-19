#include "lhs_vm.h"

int aa()
{
    return 1;
}

int main()
{
    int bb = aa() + aa();
    LHSVM* vm = lhsvm_create(0);
    lhsvm_dofile(vm, "./test.lhs");
    lhsvm_destroy(vm);
    return 0;
}
