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

static LHSCallContext* lhsexec_nestcc(LHSVM* vm, int narg, int nret, 
    StkID errfn, IPID ip)
{
    if (vm->ncallcontext >= LHS_MAXCALLLAYER)
    {
        lhserr_runtimeerr(vm, 0, "stack layer overflow.");
    }

    LHSCallContext* cc = lhsmem_newobject(vm, sizeof(LHSCallContext));
    cc->frame = vm->currentframe;
    cc->base = vm->top - narg;
    cc->errfn = errfn;
    cc->top = vm->top;
    cc->ip = ip;
    cc->rp = vm->callcontext ? lhsexec_castcc(vm->callcontext)->ip : 0;
    cc->narg = narg;
    cc->nret = nret;
    cc->line = 0;
    cc->column = 0;
    cc->refer = 0;
    cc->parent = vm->callcontext;

    vm->callcontext = cc;
    vm->ncallcontext++;
    return cc;
}

static int lhsexec_add(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_sub(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
} 

static int lhsexec_mul(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_div(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_mod(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_andb(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_orb(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_xorb(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_less(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_great(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_equal(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_ne(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_ge(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_le(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_and(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_or(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_shl(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_shr(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_neg(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_not(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_notb(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_mov(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_movs(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
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
            &lhsframe_castcurframe(vm)->localvalues, 
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
            &vm->globalvalues,
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
            &vm->conststrs,
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

static int lhsexec_jmp(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    int i = lhsexec_i(cc);
    cc->ip = vm->code.data + i;
    return LHS_TRUE;
}

static int lhsexec_jz(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_jnz(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_nop(LHSVM* vm)
{
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

    LHSCallContext* prev = cc->parent;
    prev->top = cc->top;
    vm->callcontext = prev;

    lhsmem_freeobject(vm, cc, sizeof(LHSCallContext));
    return LHS_TRUE;
}

static int lhsexec_callframe(LHSVM* vm, LHSFrame* frame, int narg, int nret,
    const LHSVarDesc* desc)
{
    if (frame->narg != narg)
    {
        lhserr_runtimeerr
        (
            vm,
            desc,
            "the number of parameters does not match."
        );
    }

    lhsframe_setframe(vm, frame);
    for (int i = 1; i <= narg; ++i)
    {
        memcpy
        (
            lhsvector_at(vm, &frame->localvalues, i - 1), 
            lhsvm_getvalue(vm, i), 
            sizeof(LHSValue)
        );
    }

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

static int lhsexec_call(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    const LHSVarDesc* desc = 0;
    const LHSValue* func = 0;

    char mark = lhsexec_mark(cc);
    switch (mark)
    {
    case LHS_MARKLOCAL:
    {
        LHSVar *var = lhsvector_at
        (
            vm,
            &lhsframe_castcurframe(vm)->localvalues,
            lhsexec_i(cc)
        );
        func = &var->value;
        desc = var->desc;
        break;
    }
    case LHS_MARKGLOBAL:
    {
        LHSVar *var = lhsvector_at
        (
            vm,
            &vm->globalvalues,
            lhsexec_i(cc)
        );
        func = &var->value;
        desc = var->desc;
        break;
    }
    case LHS_MARKSTACK:
    {
        func = lhsvm_getvalue(vm, lhsexec_i(cc));
        break;
    }
    default:
    {
        lhserr_runtimeerr(vm, 0, "unexpected byte code.");
    }
    }

    int narg = lhsexec_i(cc);
    int nret = lhsexec_i(cc);

    if (vm->top < lhsexec_castcc(vm->callcontext)->top)
    {
        vm->top = lhsexec_castcc(vm->callcontext)->top;
    }

    if (func->type == LHS_TDELEGATE)
    {
        lhsexec_calldelegate(vm, func->dg, narg, nret);
    }
    else if (func->type == LHS_TGC && func->gc->type == LHS_TGCFRAME)
    {
        lhsexec_callframe(vm, lhsframe_castframe(func->gc), narg, nret, desc);
    }
    else
    {
        lhserr_runtimeerr
        (
            vm,
            desc,
            "expected 'function', got '%s'",
            lhsvalue_typename
            [
                func->type == LHS_TGC ? LHS_TGC + func->gc->type : func->type
            ]
        );
    }

    return LHS_TRUE;
}

static int lhsexec_ret(LHSVM* vm)
{
    LHSCallContext* ccc = vm->callcontext;

    LHSValue* result = lhsvector_at(vm, &vm->stack, ccc->base);
    result->type = LHS_TNONE;

    vm->top = ccc->base;
    ccc->top = vm->top + 1;

    LHSCallContext* pcc = ccc->parent;
    pcc->top = ccc->top;
    pcc->ip = ccc->rp;
    vm->callcontext = pcc;

    lhsmem_freeobject(vm, ccc, sizeof(LHSCallContext));
    lhsframe_setframe(vm, pcc->frame);
    return LHS_TRUE;
}

static int lhsexec_ret1(LHSVM* vm)
{
    LHSCallContext* ccc = vm->callcontext;

    LHSValue* result = lhsvector_at(vm, &vm->stack, ccc->base);
    memcpy(result, lhsvm_getvalue(vm, -1), sizeof(LHSValue));

    vm->top = ccc->base;
    ccc->top = vm->top + 1;

    LHSCallContext* pcc = ccc->parent;
    pcc->top = ccc->top;
    pcc->ip = ccc->rp;
    vm->callcontext = pcc;

    lhsmem_freeobject(vm, ccc, sizeof(LHSCallContext));
    lhsframe_setframe(vm, pcc->frame);
    return LHS_TRUE;
}

static int lhsexec_swap(LHSVM* vm)
{
    lhserr_throw(vm, "aaaa");
    return LHS_TRUE;
}

static int lhsexec_exit(LHSVM* vm)
{
    lhserr_check
    (
        vm, 
        !lhsexec_castcc(vm->callcontext)->parent, 
        "unexpected byte code."
    );

    lhsmem_freeobject(vm, vm->callcontext, sizeof(LHSCallContext));
    vm->callcontext = 0;
    return LHS_FALSE;
}

lhsexec_instruct instructions[] =
{
    0, lhsexec_add, lhsexec_sub,  lhsexec_mul, lhsexec_div,
    lhsexec_mod, lhsexec_andb,  lhsexec_orb, lhsexec_xorb, lhsexec_less,
    lhsexec_great, lhsexec_equal, lhsexec_ne, lhsexec_ge, lhsexec_le,
    lhsexec_and, lhsexec_or, lhsexec_shl, lhsexec_shr, lhsexec_neg,
    lhsexec_not, lhsexec_notb, lhsexec_mov, lhsexec_movs, lhsexec_push,
    lhsexec_pop, lhsexec_jmp, lhsexec_jz, lhsexec_jnz, lhsexec_nop,
    lhsexec_call, lhsexec_ret, lhsexec_ret1, lhsexec_swap, lhsexec_exit
};

static int lhsexec_execute(LHSVM* vm, void* ud)
{
    lhs_unused(ud);
    while (LHS_TRUE)
    {
        LHSCallContext* cc = lhsexec_castcc(vm->callcontext);

        char op = lhsexec_op(cc);
        cc->line = lhsexec_i(cc);
        cc->column = lhsexec_i(cc);
        cc->refer = lhsexec_i(cc);

        if (!instructions[op](vm))
        {
            break;
        }
    }

    return LHS_TRUE;
}

static int lhsexec_reset(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;
    while (cc)
    {
        LHSCallContext* prev = cc->parent;
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
