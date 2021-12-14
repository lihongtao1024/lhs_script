#include "lhs_execute.h"
#include "lhs_value.h"
#include "lhs_frame.h"
#include "lhs_code.h"
#include "lhs_error.h"
#include "lhs_vm.h"

typedef int (*lhsexec_instruct)(LHSVM*);

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

static int lhsexec_calldelegate(LHSVM*, lhsvm_delegate, int, int);
static int lhsexec_callframe(LHSVM* vm, LHSFrame* frame, int narg, int nret);

static LHSCallContext* lhsexec_nestcc(LHSVM* vm, int narg, int nret, 
    StkID errfn, IPID ip)
{
    LHSCallContext* cc = lhsmem_newobject(vm, sizeof(LHSCallContext));
    cc->base = vm->top - narg;
    cc->errfn = errfn;
    cc->top = vm->top;
    cc->ip = ip;
    cc->rp = vm->callcontext ? lhsexec_castcc(vm->callcontext)->ip : 0;
    cc->narg = narg;
    cc->nret = nret;
    cc->prev = vm->callcontext;
    vm->callcontext = cc;
    return cc;
}

static int lhsexec_nop(LHSVM* vm)
{
    return LHS_TRUE;
}

static int lhsexec_mov(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
    char mark = lhsexec_mark(cc);
    LHSValue* lvalue = 0, * rvalue = 0;
    switch (mark)
    {
    case LHS_MARKLOCAL:
    {
        lvalue = lhsvector_at
        (
            vm,
            &lhsframe_castcurframe(vm)->values,
            lhsexec_i(cc)
        );
        break;
    }
    case LHS_MARKGLOBAL:
    {
        lvalue = lhsvector_at
        (
            vm,
            &lhsframe_castmainframe(vm)->values,
            lhsexec_i(cc)
        );
        break;
    }
    case LHS_MARKSTRING:
    {
        lvalue = lhsvector_at
        (
            vm,
            &vm->conststrvalue,
            lhsexec_i(cc)
        );
        break;
    }
    case LHS_MARKSTACK:
    {
        lvalue = lhsvm_getvalue(vm, lhsexec_i(cc));
        break;
    }
    default:
    {
        lhserr_throw(vm, "unexpected byte code.");
    }
    }

    mark = lhsexec_mark(cc);
    switch (mark)
    {
    case LHS_MARKLOCAL:
    {
        rvalue = lhsvector_at
        (
            vm,
            &lhsframe_castcurframe(vm)->values,
            lhsexec_i(cc)
        );
        break;
    }
    case LHS_MARKGLOBAL:
    {
        rvalue = lhsvector_at
        (
            vm,
            &lhsframe_castmainframe(vm)->values,
            lhsexec_i(cc)
        );
        break;
    }
    case LHS_MARKSTRING:
    {
        rvalue = lhsvector_at
        (
            vm,
            &vm->conststrvalue,
            lhsexec_i(cc)
        );
        break;
    }
    case LHS_MARKSTACK:
    {
        rvalue = lhsvm_getvalue(vm, lhsexec_i(cc));
        break;
    }
    default:
    {
        lhserr_throw(vm, "unexpected byte code.");
    }
    }

    memcpy(lvalue, rvalue, sizeof(LHSValue));
    return LHS_TRUE;
}

static int lhsexec_push(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
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

static int lhsexec_pop(LHSVM* vm)
{
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_pushc(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
    char mark = lhsexec_mark(cc);
    long long l = lhsexec_l(cc);

    LHSFrame* frame = lhsframe_castcurframe(vm);
    LHSChunk** chunk = lhsvector_at(vm, &frame->allchunks, (size_t)l);
    lhsframe_setchunk(vm, frame, *chunk);
    return LHS_TRUE;
}

static int lhsexec_popc(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
    LHSFrame* frame = lhsframe_castcurframe(vm);
    lhsframe_resetchunk(vm, frame);
    return LHS_TRUE;
}

static int lhsexec_jmp(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
    char mark = lhsexec_mark(cc);
    lhserr_check(vm, mark == LHS_MARKINTEGER, "system error.");

    long long l = lhsexec_l(cc);
    cc->ip = vm->code.data + l;
    return LHS_TRUE;
}

static int lhsexec_jz(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
    char mark = lhsexec_mark(cc);
    lhserr_check(vm, mark == LHS_MARKINTEGER, "system error.");
    long long l = lhsexec_l(cc);

    int result = LHS_TRUE;
    LHSValue* value = lhsvm_getvalue(vm, -1);
    switch (value->type)
    {
    case LHS_TNONE:
    {
        result = LHS_FALSE;
        break;
    }
    case LHS_TINTEGER:
    {
        result = !!value->i;
        break;
    }
    case LHS_TNUMBER:
    {
        result = !!value->n;
        break;
    }
    case LHS_TBOOLEAN:
    {
        result = !!value->b;
        break;
    }
    case LHS_TDELEGATE:
    {
        result = !!value->dg;
        break;
    }
    default:
    {
        break;
    }
    }

    if (result)
    {
        return LHS_TRUE;
    }

    cc->ip = vm->code.data + l;
    return LHS_TRUE;
}

static int lhsexec_jnz(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
    char mark = lhsexec_mark(cc);
    lhserr_check(vm, mark == LHS_MARKINTEGER, "system error.");
    long long l = lhsexec_l(cc);

    int result = LHS_TRUE;
    LHSValue* value = lhsvm_getvalue(vm, -1);
    switch (value->type)
    {
    case LHS_TNONE:
    {
        result = LHS_FALSE;
        break;
    }
    case LHS_TINTEGER:
    {
        result = !!value->i;
        break;
    }
    case LHS_TNUMBER:
    {
        result = !!value->n;
        break;
    }
    case LHS_TBOOLEAN:
    {
        result = !!value->b;
        break;
    }
    case LHS_TDELEGATE:
    {
        result = !!value->dg;
        break;
    }
    default:
    {
        break;
    }
    }

    if (!result)
    {
        return LHS_TRUE;
    }

    cc->ip = vm->code.data + l;
    return LHS_TRUE;
}

static int lhsexec_call(LHSVM* vm)
{  
    LHSCallContext* cc = vm->callcontext;
    LHSValue* value = 0; LHSSymbol* debug = 0; 
    int index = 0, narg = 0, nret = LHS_MULTRET, rret = 0;
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
        lhserr_runtimeerr(vm, debug, "system error.");
    }
    }

    narg = lhsexec_i(cc);
    nret = lhsexec_b(cc);

    if (vm->top < lhsexec_castcc(vm->callcontext)->top)
    {
        vm->top = lhsexec_castcc(vm->callcontext)->top;
    }

    if (value->type == LHS_TDELEGATE)
    {
        lhsexec_calldelegate(vm, value->dg, narg, nret);
    }
    else if (value->type == LHS_TGC &&
        value->gc->type == LHS_TGCFRAME)
    {
        LHSFrame* frame = lhsframe_castframe(value->gc);
        if (frame->narg != narg)
        {
            lhserr_runtimeerr
            (
                vm, 
                debug, 
                "the number of parameters does not match."
            );
        }

        lhsexec_callframe(vm, frame, narg, nret);
    }
    else
    {
        lhserr_runtimeerr
        (
            vm, 
            debug, 
            "expected 'function', got '%s'",
            lhsvalue_typename
            [
                value->type == LHS_TGC ? 
                LHS_TGC + value->gc->type : 
                value->type
            ]
        );
    }
    
    return LHS_TRUE;
}

static int lhsexec_ret(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
    char mark = lhsexec_mark(cc);
    lhserr_check(vm, mark == LHS_MARKSTACK, "system error.");

    int index = lhsexec_i(cc);
    lhserr_check(vm, index == -1, "system error.");

    LHSValue* retv = lhsvector_at(vm, &vm->stack, cc->base);
    LHSValue* value = lhsvm_getvalue(vm, -1);
    memcpy(retv, value, sizeof(LHSValue));

    vm->top = cc->base;
    cc->top = vm->top + 1;

    LHSCallContext* c = cc->prev;
    c->top = cc->top;
    c->ip = cc->rp;
    vm->callcontext = c;

    lhsmem_freeobject(vm, cc, sizeof(LHSCallContext));
    lhsframe_resetframe(vm);
    return LHS_TRUE;
}

static int lhsexec_return(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
    lhsvm_pushnil(vm);
    LHSValue* retv = lhsvector_at(vm, &vm->stack, cc->base);
    LHSValue* value = lhsvm_getvalue(vm, -1);
    memcpy(retv, value, sizeof(LHSValue));

    vm->top = cc->base;
    cc->top = vm->top + 1;

    LHSCallContext* c = cc->prev;
    c->top = cc->top;
    c->ip = cc->rp;
    vm->callcontext = c;

    lhsmem_freeobject(vm, cc, sizeof(LHSCallContext));
    lhsframe_resetframe(vm);
    return LHS_TRUE;
}

static int lhsexec_calldelegate(LHSVM* vm, lhsvm_delegate dg, 
    int narg, int nret)
{
    LHSCallContext* cc = lhsexec_nestcc
    (
        vm,
        narg, 
        nret, 
        lhsexec_castcc(vm->callcontext)->errfn,
        lhsexec_castcc(vm->callcontext)->ip
    );

    cc->nret = dg(vm);
    LHSValue* value = lhsvector_at(vm, &vm->stack, cc->base);
    if (cc->nret)
    {
        memcpy(value, lhsvm_getvalue(vm, -1), sizeof(LHSValue));
    }
    else
    {
        lhsvm_pushnil(vm);
        memcpy(value, lhsvm_getvalue(vm, -1), sizeof(LHSValue));
    }

    vm->top = cc->base;
    cc->top = vm->top + 1;

    LHSCallContext* prev = cc->prev;
    prev->top = cc->top;
    vm->callcontext = prev;

    lhsmem_freeobject(vm, cc, sizeof(LHSCallContext));
    return LHS_TRUE;
}

static int lhsexec_callframe(LHSVM* vm, LHSFrame* frame, int narg, int nret)
{
    lhserr_check(vm, frame->narg == narg, "system error.");
    lhsframe_setframe(vm, frame);
    lhsexec_nestcc
    (
        vm,
        narg, 
        nret, 
        lhsexec_castcc(vm->callcontext)->errfn,
        vm->code.data + frame->entry
    );

    return LHS_TRUE;
}

static int lhsexec_exit(LHSVM* vm)
{
    lhserr_check(vm, !lhsexec_castcc(vm->callcontext)->prev, "system error.");
    lhsmem_freeobject(vm, vm->callcontext, sizeof(LHSCallContext));
    vm->callcontext = 0;
    return LHS_FALSE;
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
    lhsexec_mov,//OP_MOV  
    lhsexec_push,//OP_PUSH 
    lhsexec_pop,//OP_POP  
    lhsexec_pushc,//OP_PUSHC
    lhsexec_popc,//OP_POPC 
    lhsexec_jmp,//OP_JMP  
    lhsexec_jz,//OP_JZ   
    lhsexec_jnz,//OP_JNZ  
    lhsexec_nop,//OP_NOP  
    lhsexec_call,//OP_CALL 
    lhsexec_ret,//OP_RET  
    lhsexec_return,//OP_RETUR
    lhsexec_exit//OP_EXIT
};

static int lhsexec_execute(LHSVM* vm, void* ud)
{
    lhs_unused(ud);
    while (LHS_TRUE)
    {
        char op = lhsexec_op(lhsexec_castcc(vm->callcontext));
        if (!instructions[op](vm))
        {
            break;
        }
    }

    return LHS_TRUE;
}

static int lhsexec_reset(LHSVM* vm)
{
    LHSFrame* frame = vm->currentframe;
    while (frame)
    {
        frame->curchunk = 0;
        frame = frame->parent;
    }

    LHSCallContext* cc = vm->callcontext;
    while (cc)
    {
        LHSCallContext* prev = cc->prev;
        lhsmem_freeobject(vm, cc, sizeof(LHSCallContext));
        cc = prev;
    }

    vm->currentframe = vm->mainframe;
    return LHS_TRUE;
}

int lhsexec_pcall(LHSVM* vm, int narg, int nret, StkID errfn)
{
    lhsexec_nestcc(vm, narg, nret, errfn, vm->code.data);
    int errcode = lhserr_protectedcall(vm, lhsexec_execute, 0);
    if (errcode)
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);
        lhsexec_reset(vm);
    }

    return errcode;
}
