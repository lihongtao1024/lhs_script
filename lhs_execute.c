#include "lhs_execute.h"
#include "lhs_value.h"
#include "lhs_function.h"
#include "lhs_code.h"
#include "lhs_error.h"
#include "lhs_link.h"
#include "lhs_variable.h"
#include "lhs_parser.h"
#include "lhs_vm.h"

typedef int (*lhsexec_instruct)(LHSVM*);
typedef int (*lhsexec_opr)(LHSVM*, LHSValue*, const LHSValue*, char);

#define lhsexec_op(cc)                                              \
(*((cc)->ip)++)

#define lhsexec_c(cc)                                               \
(*((cc)->ip)++)

#define lhsexec_i(cc)                                               \
(*((int*)(cc)->ip)++)

#define lhsexec_l(cc)                                               \
(*((long long*)(cc)->ip)++)

#define lhsexec_n(cc)                                               \
(*((double*)(cc)->ip)++)

#define lhsexec_b(cc)                                               \
(*((cc)->ip)++)

#define lhsexec_valuetype(v)                                        \
((v)->type == LHS_TGC ? LHS_TGC + (v)->gc->type : v->type)

static int lhsexec_initparams(LHSVM* vm, LHSHash* hash, LHSVarDesc* desc, 
    LHSCallContext* cc)
{
    lhserr_check(vm, desc->index <= cc->localvars.usize, "system error.");

    int index = desc->index - 1;
    LHSVar* var = lhsvector_at(vm, &cc->localvars, index);
    var->desc = desc;

    if (index < cc->func->narg)
    {
        const LHSValue* value = lhsvm_getvalue(vm, index + 1);
        memcpy(&var->value, value, sizeof(LHSValue));
    }
    return LHS_TRUE;
}

static int lhsexec_initlocalvars(LHSVM* vm, LHSCallContext* cc)
{
    if (cc->func)
    {
        lhsvector_init(vm, &cc->localvars, sizeof(LHSVar), cc->func->nlocalvars);
        cc->localvars.usize = cc->func->nlocalvars;        
        for (size_t i = 0; i < cc->localvars.usize; i++)
        {
            LHSVar* var = lhsvector_at(vm, &cc->localvars, i);
            var->value.type = LHS_TNONE;
            var->desc = 0;
        }

        lhshash_foreach(vm, &cc->func->localvars, lhsexec_initparams, cc);  
    }
    else
    {
        lhsvector_init(vm, &cc->localvars, sizeof(LHSVar), 0);
    }
    return LHS_TRUE;
}

static LHSCallContext* lhsexec_forwardcc(LHSVM* vm, LHSFunction* function, 
    int narg, int nwant, StkID errfn, IPID ip, int type)
{
    (vm->ncallcontext >= LHS_MAXCALLLAYER) && 
        lhserr_runtime(vm, 0, "stack layers overflow.");

    LHSCallContext* cc = lhsmem_newobject(vm, sizeof(LHSCallContext));
    cc->func = function;
    cc->base = vm->top - narg;
    cc->errfn = errfn;
    cc->ip = ip;
    cc->rp = vm->callcontext ? lhsexec_castcc(vm->callcontext)->ip : 0;
    cc->nwant = nwant;
    cc->line = 0;
    cc->column = 0;
    cc->refer = 0;
    cc->type = type;

    lhslink_forward(vm, callcontext, cc, parent);
    vm->ncallcontext++;

    (type == LHS_FCALL) && lhsexec_initlocalvars(vm, cc);
    return cc;
}

static int lhsexec_backcc(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    lhslink_back((vm), callcontext, cc, parent);
    (vm)->ncallcontext--;

    if (cc->type == LHS_FCALL)
    {
        lhsvector_uninit(vm, &cc->localvars);
    }
    lhsmem_freeobject(vm, cc, sizeof(LHSCallContext));
    return LHS_TRUE;
}

static int lhsexec_oprnone(LHSVM* vm, LHSValue* lvalue, 
    const LHSValue* rvalue, char op)
{
    lhserr_runtime
    (
        vm,
        0,
        "lvalue type: '%s', rvalue type: '%s', illegal operation symbol '%s'.",
        lhsvalue_typename[lhsexec_valuetype(lvalue)],
        lhsvalue_typename[lhsexec_valuetype(rvalue)],
        lhsparser_symbols[op]
    );
    return LHS_TRUE;
}

static int lhsexec_oprll(LHSVM* vm, LHSValue* lvalue, 
    const LHSValue* rvalue, char op)
{
    long long rl = rvalue->i;
    switch (op)
    {
    case OP_ADD:
    {
        lvalue->i += rl;
        break;
    }
    case OP_SUB:
    {
        lvalue->i -= rl;
        break;
    }
    case OP_MUL:
    {
        lvalue->i *= rl;
        break;
    }
    case OP_DIV:
    {
        if (lvalue->i % rl)
        {
            lvalue->type = LHS_TNUMBER;
            lvalue->n = (double)lvalue->i / rl;
        }
        else
        {
            lvalue->i /= rl;
        }
        
        break;
    }
    case OP_MOD:
    {
        lvalue->i %= rl;
        break;
    }
    case OP_ANDB:
    {
        lvalue->i &= rl;
        break;
    }
    case OP_ORB:
    {
        lvalue->i |= rl;
        break;
    }
    case OP_XORB:
    {
        lvalue->i ^= rl;
        break;
    }
    case OP_L:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->i < rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_G:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->i > rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_E:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->i == rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_NE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->i != rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_GE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->i >= rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_LE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->i <= rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_SHL:
    {
        lvalue->i <<= rl;
        break;
    }
    case OP_SHR:
    {
        lvalue->i >>= rl;
        break;
    }
    default:
    {
        lhserr_runtime
        (
            vm,
            0,
            "lvalue type: '%s', rvalue type: '%s', illegal operation symbol '%s'.",
            lhsvalue_typename[lhsexec_valuetype(lvalue)],
            lhsvalue_typename[lhsexec_valuetype(rvalue)],
            lhsparser_symbols[op]
        );
    }
    }
    return LHS_TRUE;
}

static int lhsexec_oprln(LHSVM* vm, LHSValue* lvalue, 
    const LHSValue* rvalue, char op)
{
    long long ll = lvalue->i; double rn = rvalue->n;
    switch (op)
    {
    case OP_ADD:
    {
        lvalue->type = LHS_TNUMBER;
        lvalue->n = ll + rn;
        break;
    }
    case OP_SUB:
    {
        lvalue->type = LHS_TNUMBER;
        lvalue->n = ll - rn;
        break;
    }
    case OP_MUL:
    {
        lvalue->type = LHS_TNUMBER;
        lvalue->n = ll * rn;
        break;
    }
    case OP_DIV:
    {
        lvalue->type = LHS_TNUMBER;
        lvalue->n = ll / rn;
        break;
    }
    case OP_L:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ll < rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_G:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ll > rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_E:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ll == rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_NE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ll != rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_GE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ll >= rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_LE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ll <= rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    default:
    {
        lhserr_runtime
        (
            vm,
            0,
            "lvalue type: '%s', rvalue type: '%s', illegal operation symbol '%s'.",
            lhsvalue_typename[lhsexec_valuetype(lvalue)],
            lhsvalue_typename[lhsexec_valuetype(rvalue)],
            lhsparser_symbols[op]
        );
    }
    }
    return LHS_TRUE;
}

static int lhsexec_oprnl(LHSVM* vm, LHSValue* lvalue, 
    const LHSValue* rvalue, char op)
{
    double ln = lvalue->n; long long rl = rvalue->i;
    switch (op)
    {
    case OP_ADD:
    {
        lvalue->n = ln + rl;
        break;
    }
    case OP_SUB:
    {
        lvalue->n = ln - rl;
        break;
    }
    case OP_MUL:
    {
        lvalue->n = ln * rl;
        break;
    }
    case OP_DIV:
    {
        lvalue->n = ln / rl;
        break;
    }
    case OP_L:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ln < rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_G:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ln > rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_E:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ln == rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_NE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ln != rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_GE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ln >= rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_LE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (ln <= rl) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    default:
    {
        lhserr_runtime
        (
            vm,
            0,
            "lvalue type: '%s', rvalue type: '%s', illegal operation symbol '%s'.",
            lhsvalue_typename[lhsexec_valuetype(lvalue)],
            lhsvalue_typename[lhsexec_valuetype(rvalue)],
            lhsparser_symbols[op]
        );
    }
    }
    return LHS_TRUE;
}

static int lhsexec_oprnn(LHSVM* vm, LHSValue* lvalue, 
    const LHSValue* rvalue, char op)
{
    double rn = rvalue->n;
    switch (op)
    {
    case OP_ADD:
    {
        lvalue->n += rn;
        break;
    }
    case OP_SUB:
    {
        lvalue->n -= rn;
        break;
    }
    case OP_MUL:
    {
        lvalue->n *= rn;
        break;
    }
    case OP_DIV:
    {
        lvalue->n /= rn;
        break;
    }
    case OP_L:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->n < rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_G:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->n > rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_E:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->n == rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_NE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->n != rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_GE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->n >= rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_LE:
    {
        lvalue->type = LHS_TBOOLEAN;
        lvalue->b = (lvalue->n <= rn) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    default:
    {
        lhserr_runtime
        (
            vm,
            0,
            "lvalue type: '%s', rvalue type: '%s', illegal operation symbol '%s'.",
            lhsvalue_typename[lhsexec_valuetype(lvalue)],
            lhsvalue_typename[lhsexec_valuetype(rvalue)],
            lhsparser_symbols[op]
        );
    }
    }
    return LHS_TRUE;
}

static int lhsexec_oprbb(LHSVM* vm, LHSValue* lvalue, 
    const LHSValue* rvalue, char op)
{
    char rb = rvalue->b;
    switch (op)
    {
    case OP_E:
    {
        lvalue->b = (lvalue->b == rb) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_NE:
    {
        lvalue->b = (lvalue->b != rb) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_AND:
    {
        lvalue->b = (lvalue->b && rb) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    case OP_OR:
    {
        lvalue->b = (lvalue->b || rb) ? LHS_TRUE : LHS_FALSE;
        break;
    }
    default:
    {
        lhserr_runtime
        (
            vm,
            0,
            "lvalue type: '%s', rvalue type: '%s', illegal operation symbol '%s'.",
            lhsvalue_typename[lhsexec_valuetype(lvalue)],
            lhsvalue_typename[lhsexec_valuetype(rvalue)],
            lhsparser_symbols[op]
        );
    }
    }
    return LHS_TRUE;
}

static int lhsexec_oprerr(LHSVM* vm, LHSValue* lvalue, 
    const LHSValue* rvalue, char op)
{
    lhserr_runtime
    (
        vm,
        0,
        "lvalue type: '%s', rvalue type: '%s', illegal operation symbol '%s'.",
        lhsvalue_typename[lhsexec_valuetype(lvalue)],
        lhsvalue_typename[lhsexec_valuetype(rvalue)],
        lhsparser_symbols[op]
    );
    return LHS_TRUE;
}

static lhsexec_opr lhsexec_operations[][6] =
{
    {lhsexec_oprnone, lhsexec_oprnone, lhsexec_oprnone, 
     lhsexec_oprnone, lhsexec_oprnone, lhsexec_oprnone},
    {lhsexec_oprnone, lhsexec_oprll,   lhsexec_oprln, 
     lhsexec_oprerr,  lhsexec_oprerr,  lhsexec_oprerr},
    {lhsexec_oprnone, lhsexec_oprnl,   lhsexec_oprnn, 
     lhsexec_oprerr,  lhsexec_oprerr,  lhsexec_oprerr},
    {lhsexec_oprnone, lhsexec_oprerr,  lhsexec_oprerr, 
     lhsexec_oprbb,   lhsexec_oprerr,  lhsexec_oprerr},
    {lhsexec_oprnone, lhsexec_oprerr,  lhsexec_oprerr, 
     lhsexec_oprerr,  lhsexec_oprerr,  lhsexec_oprerr},
    {lhsexec_oprnone, lhsexec_oprerr,  lhsexec_oprerr, 
     lhsexec_oprerr,  lhsexec_oprerr,  lhsexec_oprerr}
};

static int lhsexec_add(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm, 
        (LHSValue*)lvalue, 
        rvalue, 
        OP_ADD
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_sub(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_SUB
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
} 

static int lhsexec_mul(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_MUL
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_div(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_DIV
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_mod(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_MOD
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_andb(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_ANDB
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_orb(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_ORB
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_xorb(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_XORB
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_less(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_L
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_great(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_G
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_equal(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_E
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_ne(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_NE
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_ge(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_GE
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_le(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_LE
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_and(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_AND
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_or(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_OR
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_shl(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_SHL
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_shr(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    lhsexec_operations[lvalue->type][rvalue->type]
    (
        vm,
        (LHSValue*)lvalue,
        rvalue,
        OP_SHR
    );
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_neg(LHSVM* vm)
{
    LHSValue* lvalue = (LHSValue*)lhsvm_getvalue(vm, -1);
    switch (lvalue->type)
    {
    case LHS_TINTEGER:
    {
        lvalue->i = 0 - lvalue->i;
        break;
    }
    case LHS_TNUMBER:
    {
        lvalue->n = 0 - lvalue->n;
        break;
    }
    default:
    {
        lhserr_runtime
        (
            vm,
            0,
            "illegal operation symbol '%s'.",
            lhsparser_symbols[OP_NEG]
        );
    }
    }
    return LHS_TRUE;
}

static int lhsexec_not(LHSVM* vm)
{
    LHSValue* lvalue = (LHSValue*)lhsvm_getvalue(vm, -1);
    (lvalue->type != LHS_TBOOLEAN) &&
        lhserr_runtime
        (
            vm,
            0,
            "illegal operation symbol '%s'.",
            lhsparser_symbols[OP_NOT]
        );

    lvalue->b = !lvalue->b;
    return LHS_TRUE;
}

static int lhsexec_notb(LHSVM* vm)
{
    LHSValue* lvalue = (LHSValue*)lhsvm_getvalue(vm, -1);
    (lvalue->type != LHS_TINTEGER) &&
        lhserr_runtime
        (
            vm,
            0,
            "illegal operation symbol '%s'.",
            lhsparser_symbols[OP_NOT]
        );

    lvalue->i = ~lvalue->i;
    return LHS_TRUE;
}

static int lhsexec_mov(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    LHSValue* lvalue = 0;
    char lmark = lhsexec_c(cc);
    switch (lmark)
    {
    case LHS_MARKGLOBAL:
    {
        lvalue = lhsvector_at
        (
            vm,
            &vm->globalvalues,
            lhsexec_i(cc)
        );
        break;
    }
    case LHS_MARKLOCAL:
    {
        lvalue = lhsvector_at
        (
            vm,
            &cc->localvars,
            (size_t)lhsexec_i(cc) - 1
        );
        break;
    }
    case LHS_MARKSTACK:
    {
        lvalue = (LHSValue*)lhsvm_getvalue(vm, lhsexec_i(cc));
        break;
    }
    default:
    {
        lhserr_runtime(vm, 0, "illegal assignment operation.");
    }
    }

    LHSValue* rvalue = 0;
    char rmark = lhsexec_c(cc);
    switch (rmark)
    {
    case LHS_MARKGLOBAL:
    {
        rvalue = lhsvector_at
        (
            vm,
            &vm->globalvalues,
            lhsexec_i(cc)
        );
        break;
    }
    case LHS_MARKLOCAL:
    {
        lvalue = lhsvector_at
        (
            vm,
            &cc->localvars,
            (size_t)lhsexec_i(cc) - 1
        );
        break;
    }
    case LHS_MARKSTACK:
    {
        rvalue = (LHSValue*)lhsvm_getvalue(vm, lhsexec_i(cc));
        break;
    }
    case LHS_MARKSTRING:
    {
        rvalue = lhsvector_at
        (
            vm,
            &vm->conststrs,
            lhsexec_i(cc)
        );
        break;
    }
    default:
    {
        lhserr_runtime(vm, 0, "illegal assignment operation.");
    }
    }

    memcpy(lvalue, rvalue, sizeof(LHSValue));
    return LHS_TRUE;
}

static int lhsexec_movs(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    LHSValue* lvalue = 0;
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);
    char mark = lhsexec_c(cc);
    switch (mark)
    {
    case LHS_MARKGLOBAL:
    {
        lvalue = lhsvector_at
        (
            vm,
            &vm->globalvalues,
            lhsexec_i(cc)
        );
        break;
    }
    case LHS_MARKLOCAL:
    {
        lvalue = lhsvector_at
        (
            vm,
            &cc->localvars,
            (size_t)lhsexec_i(cc) - 1
        );
        break;
    }
    case LHS_MARKSTACK:
    {
        lvalue = (LHSValue*)lhsvm_getvalue(vm, lhsexec_i(cc));
        break;
    }
    default:
    {
        lhserr_runtime(vm, 0, "illegal assignment operation.");
    }
    }

    memcpy(lvalue, rvalue, sizeof(LHSValue));
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_push(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    char mark = lhsexec_c(cc);
    switch (mark)
    {
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
    case LHS_MARKLOCAL:
    {
        LHSValue* value = lhsvector_at
        (
            vm,
            &cc->localvars,
            (size_t)lhsexec_i(cc) - 1
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

    long long i = lhsexec_i(cc);
    cc->ip = cc->func->code.data + i;
    return LHS_TRUE;
}

static int lhsexec_istrue(const LHSValue* value)
{
    int istrue = LHS_TRUE;
    switch (value->type)
    {
    case LHS_TINTEGER:
    {
        istrue = !!value->i;
        break;
    }
    case LHS_TNUMBER:
    {
        istrue = !!value->n;
        break;
    }
    case LHS_TBOOLEAN:
    {
        istrue = !!value->b;
        break;
    }
    case LHS_TNONE:
    {
        istrue = LHS_FALSE;
        break;
    }
    default:
    {
        break;
    }
    }

    return istrue;
}

static int lhsexec_jz(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    long long i = lhsexec_i(cc);
    const LHSValue* value = lhsvm_getvalue(vm, -1);
    if (lhsexec_istrue(value))
    {
        lhsvm_pop(vm, 1);
        return LHS_TRUE;
    }

    cc->ip = cc->func->code.data + i;
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_jnz(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    long long i = lhsexec_i(cc);
    const LHSValue* value = lhsvm_getvalue(vm, -1);
    if (!lhsexec_istrue(value))
    {
        lhsvm_pop(vm, 1);
        return LHS_TRUE;
    }

    cc->ip = cc->func->code.data + i;
    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

static int lhsexec_je(LHSVM* vm)
{
    return LHS_TRUE;
}

static int lhsexec_nop(LHSVM* vm)
{
    return LHS_TRUE;
}

static int lhsexec_calldelegate(LHSVM* vm, lhsvm_delegate dg, int narg,
    int nwant)
{
    LHSCallContext* cc = lhsexec_forwardcc
    (
        vm,
        0,
        narg,
        nwant,
        lhsexec_castcc(vm->callcontext)->errfn,
        lhsexec_castcc(vm->callcontext)->ip,
        LHS_CCALL
    );

    int nret = dg(vm);

    if (cc->nwant != LHS_VOID)
    {
        LHSValue* value = lhsvector_at(vm, &vm->stack, cc->base);
        if (nret != LHS_VOID)
        {
            memcpy(value, lhsvm_getvalue(vm, -1), sizeof(LHSValue));
        }
        else
        {
            lhsvm_pushnil(vm);
            memcpy(value, lhsvm_getvalue(vm, -1), sizeof(LHSValue));
        }
    }

    vm->top = cc->base + cc->nwant;

    lhsexec_backcc(vm);
    return LHS_TRUE;
}

static int lhsexec_callframe(LHSVM* vm, LHSFunction* function, int narg, 
    int nwant, const LHSVarDesc* desc)
{
    (function->narg != narg) &&
        lhserr_runtime
        (
            vm,
            desc,
            "the number of parameters does not match."
        );

    lhsexec_forwardcc
    (
        vm,
        function,
        narg,
        nwant,
        lhsexec_castcc(vm->callcontext)->errfn,
        function->code.data,
        LHS_FCALL
    );

    return LHS_TRUE;
}

static int lhsexec_call(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    const LHSVarDesc* desc = 0;
    const LHSValue* func = 0;

    char mark = lhsexec_c(cc);
    switch (mark)
    {
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
    case LHS_MARKLOCAL:
    {
        LHSVar *var = lhsvector_at
        (
            vm,
            &cc->localvars,
            (size_t)lhsexec_i(cc) - 1
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
        lhserr_runtime(vm, 0, "unexpected byte code.");
    }
    }

    int narg = lhsexec_i(cc), nwant = lhsexec_c(cc);
    if (func->type == LHS_TDELEGATE)
    {
        lhsexec_calldelegate(vm, func->dg, narg, nwant);
    }
    else if (func->type == LHS_TGC && func->gc->type == LHS_TGCFUNCTION)
    {
        lhsexec_callframe(vm, lhsfunction_castfunc(func->gc), narg, 
            nwant, desc);
    }
    else
    {
        lhserr_runtime
        (
            vm,
            desc,
            "expected 'function', got '%s'",
            lhsvalue_typename[lhsexec_valuetype(func)]
        );
    }

    return LHS_TRUE;
}

static int lhsexec_ret(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    LHSValue* result = lhsvector_at(vm, &vm->stack, cc->base);
    result->type = LHS_TNONE;

    vm->top = cc->base + cc->nwant;

    IPID rp = cc->rp;
    lhsexec_backcc(vm);
    lhsexec_castcc(vm->callcontext)->ip = rp;

    return LHS_TRUE;
}

static int lhsexec_ret1(LHSVM* vm)
{
    LHSCallContext* cc = vm->callcontext;

    LHSValue* result = lhsvector_at(vm, &vm->stack, cc->base);
    memcpy(result, lhsvm_getvalue(vm, -1), sizeof(LHSValue));

    vm->top = cc->base + cc->nwant;

    IPID rp = cc->rp;
    lhsexec_backcc(vm);
    lhsexec_castcc(vm->callcontext)->ip = rp;

    return LHS_TRUE;
}

static int lhsexec_swap(LHSVM* vm)
{
    const LHSValue* lvalue = lhsvm_getvalue(vm, -2);
    const LHSValue* rvalue = lhsvm_getvalue(vm, -1);

    LHSValue value;
    memcpy(&value, lvalue, sizeof(LHSValue));
    memcpy((void*)lvalue, rvalue, sizeof(LHSValue));
    memcpy((void*)rvalue, &value, sizeof(LHSValue));
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

    lhsexec_backcc(vm);
    return LHS_FALSE;
}

lhsexec_instruct instructions[] =
{
    0,             lhsexec_add,   lhsexec_sub, lhsexec_mul,  lhsexec_div,
    lhsexec_mod,   lhsexec_andb,  lhsexec_orb, lhsexec_xorb, lhsexec_less,
    lhsexec_great, lhsexec_equal, lhsexec_ne,  lhsexec_ge,   lhsexec_le,
    lhsexec_and,   lhsexec_or,    lhsexec_shl, lhsexec_shr,  lhsexec_neg,
    lhsexec_not,   lhsexec_notb,  lhsexec_mov, lhsexec_movs, lhsexec_push,
    lhsexec_pop,   lhsexec_jmp,   lhsexec_jz,  lhsexec_jnz,  lhsexec_je,
    lhsexec_nop,   lhsexec_call,  lhsexec_ret, lhsexec_ret1, lhsexec_swap, 
    lhsexec_exit
};

static int lhsexec_execute(LHSVM* vm, void* ud)
{
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

    return LHS_TRUE;
}

int lhsexec_pcall(LHSVM* vm, LHSFunction* function, int nwant, StkID errfn)
{
    lhsexec_forwardcc
    (
        vm,
        function,
        function->narg,
        nwant,
        errfn, 
        function->code.data,
        LHS_FCALL
    );

    int errcode = lhserr_protectedcall(vm, lhsexec_execute, 0);
    if (errcode)
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);
        lhsexec_reset(vm);
    }

    return errcode;
}
