#include "lhs_baselib.h"
#include "lhs_vm.h"

int lhsbase_print(LHSVM* vm)
{
    int n = lhsvm_gettop(vm);
    lhserr_check(vm, n > 0, "with print.");

    LHSSTRBUF buf;
    lhsbuf_init(vm, &buf);

    for (int i = 0; i < n; ++i)
    {
        const char* str = lhsvm_tostring(vm, i + 1);
        lhsbuf_pushs(vm, &buf, str);
        lhsbuf_pushc(vm, &buf, ' ');
        lhsvm_pop(vm, 1);
    }

    printf("%s\n", buf.data);
    lhsbuf_uninit(vm, &buf);
    return 0;
}

static void* baselib[] =
{
    "print", lhsbase_print
};

int lhsreg_baselib(LHSVM* vm)
{
    for (int i = 0; i < _countof(baselib); ++i)
    {
        lhsvm_pushdelegate(vm, baselib[i + 1]);
        lhsvm_setglobal(vm, baselib[i]);
    }
    return LHS_TRUE;
}
