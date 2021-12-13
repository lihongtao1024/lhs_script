#include "lhs_execute.h"
#include "lhs_value.h"
#include "lhs_frame.h"
#include "lhs_code.h"
#include "lhs_error.h"
#include "lhs_vm.h"

typedef const char* IPID;
typedef struct CallContext
{
    struct CallContext* prev;
    StkID base;
    StkID func;
    StkID errfn;
    IPID ip;
    int narg;
    int nret;
} CallContext;

typedef int (*lhsexec_instruct)(LHSVM*, CallContext*);

#define lhsexec_op(cc)                              \
(*((cc)->ip)++)

#define lhsexec_mark(cc)                            \
(*((cc)->ip)++)

#define lhsexec_i(cc)                               \
(*((int*)(cc)->ip)++)

#define lhsexec_l(cc)                               \
(*((long long*)(cc)->ip)++)

#define lhsexec_n(cc)                               \
(*((double*)(cc)->ip)++)

#define lhsexec_b(cc)                               \
(*((cc)->ip)++)

static int lhsexec_initcallcontext(LHSVM* vm, CallContext* cc, StkID func, 
    int narg, int nret, StkID errfn)
{
    cc->prev = 0;
    cc->base = vm->top - narg;
    cc->func = func;
    cc->errfn = errfn;
    cc->ip = vm->code.data;
    cc->narg = narg;
    cc->nret = nret;
    return LHS_TRUE;
}

static int lhsexec_checkcallcontext(LHSVM* vm, CallContext* cc)
{
    cc->prev = vm->callcontext;
    vm->callcontext = cc;
    return LHS_TRUE;
}

static int lhsexec_push(LHSVM* vm, CallContext* cc)
{
    char mark = lhsexec_mark(cc);
    switch (mark)
    {
    case LHS_MARKLOCAL:
    {
        LHSValue* value = lhsvector_at
        (
            vm, 
            &lhsframe_castcurframe(vm)->values, 
            lhsexec_i(cc)
        );
        memcpy(lhsvm_incrementstack(vm), value, sizeof(LHSValue));
        break;
    }
    case LHS_MARKGLOBAL:
    {
        LHSValue* value = lhsvector_at
        (
            vm, 
            &lhsframe_castmainframe(vm)->values, 
            lhsexec_i(cc)
        );
        memcpy(lhsvm_incrementstack(vm), value, sizeof(LHSValue));
        break;
    }
    case LHS_MARKSTRING:
    {
        LHSValue* value = lhsvector_at
        (
            vm, 
            &vm->conststrvalue,
            lhsexec_i(cc)
        );
        memcpy(lhsvm_incrementstack(vm), value, sizeof(LHSValue));
        break;
    }
    case LHS_MARKSTACK:
    {
        lhsvm_pushvalue(vm, lhsexec_i(cc));
        break;
    }
    case LHS_MARKINTEGER:
    {
        lhsvm_pushinteger(vm, lhsexec_l(cc));
        break;
    }
    case LHS_MARKNUMBER:
    {
        lhsvm_pushnumber(vm, lhsexec_n(cc));
        break;
    }
    case LHS_MARKBOOLEAN:
    {
        lhsvm_pushboolean(vm, lhsexec_b(cc));
        break;
    }
    default:
    {
        lhserr_throw(vm, "unexpected byte code.");
    }
    }

    return LHS_TRUE;
}

static int lhsexec_pushc(LHSVM* vm, CallContext* cc)
{
    char mark = lhsexec_mark(cc);
    long long l = lhsexec_l(cc);
    return LHS_TRUE;
}

static int lhsexec_call(LHSVM* vm, CallContext* cc)
{    
    LHSValue* value = 0; LHSSymbol* debug = 0; 
    int index = 0, narg = 0, nret = LHS_MULTRET;
    char mark = lhsexec_mark(cc);
    switch (mark)
    {
    case LHS_MARKLOCAL:
    {
        index = lhsexec_i(cc);
        value = lhsvector_at
        (
            vm,
            &lhsframe_castcurframe(vm)->values,
            index
        );
        debug = lhsvector_at
        (
            vm,
            &lhsframe_castcurframe(vm)->debugs.symbols,
            index
        );
        break;
    }
    case LHS_MARKGLOBAL:
    {
        index = lhsexec_i(cc);
        value = lhsvector_at
        (
            vm,
            &lhsframe_castmainframe(vm)->values,
            index
        );
        debug = lhsvector_at
        (
            vm,
            &lhsframe_castmainframe(vm)->debugs.symbols,
            index
        );
        break;
    }
    case LHS_MARKSTACK:
    {
        value = lhsvm_getvalue(vm, lhsexec_i(cc));
        break;
    }
    default:
    {
        lhserr_throw(vm, "system error.");
    }
    }

    narg = lhsexec_i(cc);
    nret = lhsexec_b(cc);

    CallContext subcc;
    lhsexec_initcallcontext
    (
        vm, 
        &subcc, 
        LHS_DEFAULTFN, 
        narg, 
        nret, 
        cc->errfn
    );
    subcc.prev = cc;
    if (value->type == LHS_TDELEGATE)
    {
        //subcc.
    }
    else if (value->type == LHS_TGC &&
        value->gc->type == LHS_TGCFRAME)
    {

    }
    else
    {

    }

    if (debug)
    {
        lhserr_runtimeerr(vm, debug, "call fail.");
    }
    else
    {
        lhserr_throw(vm, "call fail.");
    }

    
    

    return LHS_TRUE;
}

lhsexec_instruct instructions[] =
{
    0,//OP_NONE 
    0,//OP_ADD  
    0,//OP_SUB  
    0,//OP_MUL  
    0,//OP_DIV  
    0,//OP_MOD  
    0,//OP_ANDB 
    0,//OP_ORB  
    0,//OP_XORB 
    0,//OP_L    
    0,//OP_G    
    0,//OP_E    
    0,//OP_NE   
    0,//OP_GE   
    0,//OP_LE   
    0,//OP_AND  
    0,//OP_OR   
    0,//OP_SHL  
    0,//OP_SHR  
    0,//OP_NEG  
    0,//OP_NOT  
    0,//OP_NOTB 
    0,//OP_MOV  
    lhsexec_push,//OP_PUSH 
    0,//OP_POP  
    lhsexec_pushc,//OP_PUSHC
    0,//OP_POPC 
    0,//OP_JMP  
    0,//OP_JZ   
    0,//OP_JNZ  
    0,//OP_NOP  
    lhsexec_call,//OP_CALL 
    0,//OP_RET  
    0,//OP_RETUR
};

static int lhsexec_execute(LHSVM* vm, CallContext* cc)
{
    lhsexec_checkcallcontext(vm, cc);

    while (LHS_TRUE)
    {
        char op = lhsexec_op(cc);
        if (!instructions[op](vm, cc))
        {
            break;
        }
    }

    return LHS_TRUE;
}

int lhsexec_pcall(LHSVM* vm, StkID func, int narg, int nret, StkID errfn)
{
    CallContext cc;
    lhsexec_initcallcontext(vm, &cc, func, narg, nret, errfn);

    int errcode = lhserr_protectedcall(vm, lhsexec_execute, &cc);
    if (errcode)
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);
    }

    return errcode;
}
