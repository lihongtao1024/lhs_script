#include "lhs_vm.h"
#include "lhs_hash.h"
#include "lhs_vector.h"
#include "lhs_value.h"
#include "lhs_link.h"
#include "lhs_parser.h"

int main()
{
    LHSVM* vm = lhsvm_create(0);
    lhsparser_dofile(vm, "./test.lhs");
    lhsvm_destroy(vm);
    return 0;
}
