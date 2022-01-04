#include "lhs_error.h"
#include "lhs_function.h"
#include "lhs_variable.h"
#include "lhs_execute.h"
#include "lhs_link.h"
#include "lhs_vm.h"

int lhserr_protectedcall(void* vm, protectedf fn, void* udata)
{
    LHSError jmp;
    lhslink_forward(lhsvm_castvm(vm), errorjmp, &jmp, prev);

    jmp.errcode = setjmp(jmp.buf);
    if (!jmp.errcode)
    {
        fn(vm, udata);
    }

    lhslink_back(lhsvm_castvm(vm), errorjmp, &jmp, prev);
    return jmp.errcode;
}

int lhserr_protectedcallex(void* vm, protectedfex fn, void* ud1, void* ud2)
{
    LHSError jmp;
    lhslink_forward(lhsvm_castvm(vm), errorjmp, &jmp, prev);

    jmp.errcode = setjmp(jmp.buf);
    if (!jmp.errcode)
    {
        fn(vm, ud1, ud2);
    }

    lhslink_back(lhsvm_castvm(vm), errorjmp, &jmp, prev);
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

int lhserr_runtime(void* vm, const void* desc, const char* fmt, ...)
{
    LHSBuf buf;
    lhsbuf_init(vm, &buf);
    char tmp[64];

    if (!desc)
    {  
        lhsbuf_pushs(vm, &buf, "runtime error: ");
    }                                                
    else                                             
    {                                               
        lhsbuf_pushs(vm, &buf, "runtime error at: [");
        lhsbuf_pushls
        (
            vm, 
            &buf, 
            lhsvar_castvardesc(desc)->name->data, 
            lhsvar_castvardesc(desc)->name->length
        );
        lhsbuf_pushs(vm, &buf, ":");
        sprintf(tmp, "%d", lhsvar_castvardesc(desc)->line);
        lhsbuf_pushs(vm, &buf, tmp);
        lhsbuf_pushs(vm, &buf, ":");
        sprintf(tmp, "%d", lhsvar_castvardesc(desc)->column);
        lhsbuf_pushs(vm, &buf, tmp);
        lhsbuf_pushs(vm, &buf, "],"); 
    }

    va_list args;
    va_start(args, fmt);
    size_t l = (size_t)_vscprintf(fmt, args) + 1;
    char* suffix = lhsmem_newobject(lhsvm_castvm(vm), l);
    vsprintf(suffix, fmt, args);
    lhsbuf_pushls(vm, &buf, suffix, l - 1);
    lhsmem_freeobject(lhsvm_castvm(vm), suffix, l);
    lhsbuf_pushs(vm, &buf, "\n");

    for (LHSCallContext* cc = lhsvm_castvm(vm)->callcontext; 
         cc; 
         cc = cc->parent)
    {
        LHSVar* name = lhsvector_at
        (
            vm,
            &lhsvm_castvm(vm)->conststrs,
            cc->func->name
        );
        LHSVar* refer = lhsvector_at
        (
            vm, 
            &lhsvm_castvm(vm)->conststrs, 
            cc->refer
        );
        lhsbuf_pushs(vm, &buf, "stack at");
        lhsbuf_pushs(vm, &buf, "[");
        if (cc->type == LHS_FCALL)
        {
            lhsbuf_pushc(vm, &buf, 'F');
        }
        else
        {
            lhsbuf_pushc(vm, &buf, 'C');
        }
        lhsbuf_pushs(vm, &buf, "]: [");
        lhsbuf_pushls
        (
            vm,
            &buf,
            name->desc->name->data,
            name->desc->name->length
        );
        lhsbuf_pushs(vm, &buf, ":");
        sprintf(tmp, "%d", cc->line);
        lhsbuf_pushs(vm, &buf, tmp);
        lhsbuf_pushs(vm, &buf, ":");
        sprintf(tmp, "%d", cc->column);
        lhsbuf_pushs(vm, &buf, tmp);
        lhsbuf_pushs(vm, &buf, "->");
        lhsbuf_pushls
        (
            vm,
            &buf,
            refer->desc->name->data,
            refer->desc->name->length
        );
        lhsbuf_pushs(vm, &buf, "].\n");
    }

    lhsvm_pushlstring(lhsvm_castvm(vm), buf.data, buf.usize);
    lhsbuf_uninit(vm, &buf);
    longjmp(lhsvm_castvm(vm)->errorjmp->buf, 1);
    return LHS_TRUE;
}
