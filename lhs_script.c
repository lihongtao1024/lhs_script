#include "lhs_vm.h"

int main()
{
    LHSVM* vm = lhsvm_create(0);
    lhsvm_dofile(vm, "./test.lhs");
    lhsvm_destroy(vm);
    return 0;
}
