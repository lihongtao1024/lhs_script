#include "lhs_execute.h"
#include "lhs_alloc.h"
#include "lhs_vm.h"

int lhsexecute_protectedrun(void* vm, protectedf fn, void* userdata)
{
    LHSJmp jmp;
    jmp.prev = lhsvm_castvm(vm)->errorjmp;
    lhsvm_castvm(vm)->errorjmp = &jmp;

    jmp.errcode = setjmp(jmp.buf);
    if (!jmp.errcode)
    {
        fn(vm, userdata);
    }
    else
    {
        lhsvm_castvm(vm)->errorjmp = jmp.prev;
    }

    return jmp.errcode;
}

int lhsexecute_protectederr(void* vm, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t l = (size_t)_vscprintf(fmt, args) + 1;
    char* buf = lhsmem_newobject(lhsvm_castvm(vm), l);
    vsprintf(buf, fmt, args);
    lhsvm_pushlstring(lhsvm_castvm(vm), buf, l);
    lhsmem_freeobject(lhsvm_castvm(vm), buf, l);

    longjmp(lhsvm_castvm(vm)->errorjmp->buf, 1);
    return true;
}
