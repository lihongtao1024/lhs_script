#include "lhs_error.h"
#include "lhs_frame.h"
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

int lhserr_runtimeerr(void* vm, LHSSymbol* dbg, const char* fmt, ...)
{
    LHSSTRBUF buf;
    lhsbuf_init(vm, &buf);
    char tmp[64];

    if (dbg)                                         
    {  
        lhsbuf_pushs(vm, &buf, "runtime error at:[");
        lhsbuf_pushls(vm, &buf, dbg->identifier->data, dbg->identifier->length);
        lhsbuf_pushs(vm, &buf, ":");
        sprintf(tmp, "%lld", dbg->line);
        lhsbuf_pushs(vm, &buf, tmp);
        lhsbuf_pushs(vm, &buf, ":");
        sprintf(tmp, "%lld", dbg->column);
        lhsbuf_pushs(vm, &buf, tmp);
        lhsbuf_pushs(vm, &buf, "],");
    }                                                
    else                                             
    {                                                
        lhsbuf_pushs(vm, &buf, "runtime error,");
    }

    va_list args;
    va_start(args, fmt);
    size_t l = (size_t)_vscprintf(fmt, args) + 1;
    char* suffix = lhsmem_newobject(lhsvm_castvm(vm), l);
    vsprintf(suffix, fmt, args);
    lhsbuf_pushls(vm, &buf, suffix, l - 1);
    lhsmem_freeobject(lhsvm_castvm(vm), suffix, l);
    lhsbuf_pushs(vm, &buf, "\n");
    
    LHSFrame* frame = lhsvm_castvm(vm)->currentframe;
    while (frame)
    {
        LHSSymbol* stack = lhsdebug_at(vm, &frame->debugs, frame->name);
        lhsbuf_pushs(vm, &buf, "stack at:[");
        lhsbuf_pushls(vm, &buf, stack->identifier->data, stack->identifier->length);
        lhsbuf_pushs(vm, &buf, ":");
        sprintf(tmp, "%lld", dbg->line);
        lhsbuf_pushs(vm, &buf, tmp);
        lhsbuf_pushs(vm, &buf, ":");
        sprintf(tmp, "%lld", dbg->column);
        lhsbuf_pushs(vm, &buf, tmp);
        lhsbuf_pushs(vm, &buf, "].\n");
        frame = frame->parent;
    }

    lhsvm_pushlstring(lhsvm_castvm(vm), buf.data, buf.usize);
    lhsbuf_uninit(vm, &buf);

    longjmp(lhsvm_castvm(vm)->errorjmp->buf, 1);
    return LHS_TRUE;
}
