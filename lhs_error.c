#include "lhs_error.h"
#include "lhs_vm.h"

int lhserr_protectedcall(void* vm, protectedf fn, void* udata)
{
    LHSError jmp;
    jmp.prev = lhsvm_castvm(vm)->errorjmp;
    lhsvm_castvm(vm)->errorjmp = &jmp;

    jmp.errcode = setjmp(jmp.buf);
    if (!jmp.errcode)
    {
        fn(vm, udata);
    }

    lhsvm_castvm(vm)->errorjmp = jmp.prev;
    return jmp.errcode;
}

int lhserr_throw(void* vm, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t l = (size_t)_vscprintf(fmt, args) + 1;
    char* buf = lhsmem_newobject(lhsvm_castvm(vm), l);
    vsprintf(buf, fmt, args);
    lhsvm_pushlstring(lhsvm_castvm(vm), buf, l);
    lhsmem_freeobject(lhsvm_castvm(vm), buf, l);

    longjmp(lhsvm_castvm(vm)->errorjmp->buf, 1);
    return LHS_TRUE;
}