#include "lhs_baselib.h"
#include "lhs_vm.h"

int lhsbase_print(LHSVM* vm)
{
    int n = lhsvm_gettop(vm);
    lhserr_check(vm, n > 0, "with print.");

    LHSBuf buf;
    lhsbuf_init(vm, &buf);

    for (int i = 0; i < n; i++)
    {
        const char* str = lhsvm_tostring(vm, i + 1);
        i && lhsbuf_pushc(vm, &buf, ' ');
        lhsbuf_pushs(vm, &buf, str);
        lhsvm_pop(vm, 1);
    }

    printf("%s", buf.data);
    lhsbuf_uninit(vm, &buf);
    return 0;
}

static void* baselib[] =
{
    "print", lhsbase_print
};

int lhsreg_baselib(LHSVM* vm)
{
    for (int i = 0; i < _countof(baselib); i += 2)
    {
        lhsvm_pushdelegate(vm, baselib[i + 1]);
        lhsvm_setglobal(vm, baselib[i]);
    }
    return LHS_TRUE;
}
