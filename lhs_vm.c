#include "lhs_assert.h"
#include "lhs_alloc.h"
#include "lhs_value.h"
#include "lhs_variable.h"
#include "lhs_link.h"
#include "lhs_frame.h"
#include "lhs_vm.h"

static void lhsvm_init(LHSVM* vm, lhsmem_new fn)
{
    lhsassert_truereturn(vm && fn);

    vm->rax = 0;
    vm->rip = 0;
    vm->rbp = 0;
    vm->rsp = 0;
    vm->falloc = fn;
    vm->mainframe = 0;
    vm->currentframe = 0;
    vm->allgc = 0;
    vm->errorjmp = 0;

    lhsslink_push(vm, allgc, &vm->gc, next);
    lhshash_init(vm, &vm->conststr, lhsvalue_hashstring, lhsvalue_equalstring);
    lhsvector_init(vm, &vm->stack, sizeof(LHSValue));
}

static void lhsvm_allgcfree(LHSVM* vm, LHSGCObject* o)
{
    lhsassert_truereturn(vm && o);
    lhsmem_freeobject(vm, o, sizeof(LHSVM));
}

LHSVM* lhsvm_create(lhsmem_new fn)
{
    if (!fn)
    {
        fn = lhsmem_realloc;
    }

    LHSVM* vm = (LHSVM *)fn(0, 0, 0, sizeof(LHSVM));
    lhsassert_trueresult(vm, 0);
    lhsmem_initgc(lhsgc_castgc(vm), LHS_TGCVM);

    lhsvm_init(vm, fn);
    return vm;
}

int lhsvm_pushnil(LHSVM* vm)
{
    lhsassert_trueresult(vm, false);

    LHSValue* top = lhsvalue_castvalue(lhsvector_increment(vm, &vm->stack));
    lhsassert_trueresult(top, false);

    top->type = LHS_TNONE;
    return true;
}

int lhsvm_pushboolean(LHSVM* vm, char b)
{
    lhsassert_trueresult(vm, false);

    LHSValue* top = lhsvalue_castvalue(lhsvector_increment(vm, &vm->stack));
    lhsassert_trueresult(top, false);

    top->type = LHS_TBOOLEAN;
    top->b = (b ? 1 : 0);
    return true;
}

int lhsvm_pushvalue(LHSVM* vm, LHSValue* value)
{
    lhsassert_trueresult(vm && value, false);

    LHSValue* top = lhsvalue_castvalue(lhsvector_increment(vm, &vm->stack));
    lhsassert_trueresult(top && top != value, false);

    memcpy(top, value, sizeof(LHSValue));
    return true;
}

int lhsvm_pushlstring(LHSVM* vm, const char* str, size_t l)
{
    lhsassert_trueresult(vm && str, false);

    LHSString* string = 0;
    if (l < LHS_SHORTSTRLEN)
    {
        LHSString* ostring = lhsvm_findshort(vm, (void*)str, l);
        if (ostring)
        {
            string = ostring;
        }
    }
    
    if (!string)
    {
        string = lhsvalue_caststring
        (
            lhsmem_newgclstring(vm, (void*)str, l, 0)
        );
        lhsassert_trueresult(string, false);
    }

    LHSValue* top = lhsvalue_castvalue(lhsvector_increment(vm, &vm->stack));
    lhsassert_trueresult(top, false);

    top->type = LHS_TGC;
    top->gc = lhsgc_castgc(string);
    return true;
}

int lhsvm_pushnumber(LHSVM* vm, double number)
{
    lhsassert_trueresult(vm, false);

    LHSValue* top = lhsvalue_castvalue(lhsvector_increment(vm, &vm->stack));
    lhsassert_trueresult(top, false);

    top->type = LHS_TDOUBLE;
    top->n = number;

    return true;
}

int lhsvm_pushinteger(LHSVM* vm, long long integer)
{
    lhsassert_trueresult(vm, false);

    LHSValue* top = lhsvalue_castvalue(lhsvector_increment(vm, &vm->stack));
    lhsassert_trueresult(top, false);

    top->type = LHS_TINTEGER;
    top->i = integer;

    return true;
}

LHSValue* lhsvm_getvalue(LHSVM* vm, int index)
{
    lhsassert_trueresult(vm, 0);

    LHSValue* value = 0;
    if (index < 0)
    {
        value = lhsvector_at
        (
            vm, 
            &vm->stack, 
            lhsvector_length(vm, &vm->stack) + index
        );
    }
    else if (index > 0)
    {

    }

    return value;
}

int lhsvm_insertvariable(LHSVM* vm)
{
    lhsassert_trueresult(vm, false);

    LHSValue* value = lhsvm_top(vm);
    return true;
}

const char* lhsvm_tostring(LHSVM* vm, int index)
{
    lhsassert_trueresult(vm, 0);

    LHSValue* value = 0;
    if (index < 0)
    {
        value = lhsvector_at
        (
            vm, 
            &vm->stack, 
            lhsvector_length(vm, &vm->stack) + index
        );
        lhsassert_trueresult(value, 0);
    }
    else if (index > 0)
    {

    }

    if (!value)
    {
        return 0;
    }

    const char* str = 0;
    switch (value->type)
    {
    case LHS_TINTEGER:
    {
        char buf[32];
        int l = sprintf(buf, "%lld", value->i);
        if (l == -1)
        {
            lhsvm_pushnil(vm);
        }
        else
        {
            lhsvm_pushlstring(vm, buf, (size_t)l);
        }        
        str = lhsvm_tostring(vm, -1);
        break;
    }
    case LHS_TDOUBLE:
    {
        char buf[32];
        int l = sprintf(buf, "%.2f", value->n);
        if (l == -1)
        {
            lhsvm_pushnil(vm);
        }
        else
        {
            lhsvm_pushlstring(vm, buf, (size_t)l);
        }        
        str = lhsvm_tostring(vm, -1);
        break;
    }
    case LHS_TGC:
    {
        if (value->gc->type != LHS_TGCSTRING)
        {
            lhsexecute_protectederr
            (
                vm,
                "type:'%s' cannot be converted to 'string'",
                lhsgc_type[value->gc->type]
            );
        }
        else
        {
            lhsvm_pushvalue(vm, value);
            str = lhsvalue_caststring(value->gc)->data;
        }
        break;
    }
    default:
    {
        break;
    }
    }

    return str;
}

LHSString* lhsvm_findshort(LHSVM* vm, void* data, size_t l)
{
    lhsassert_trueresult(vm && data, 0);

    LHSShortString ss;
    ss.length = l;
    memcpy(ss.data, data, l);

    return lhshash_find(vm, &vm->conststr, &ss);
}

LHSValue* lhsvm_top(LHSVM* vm)
{
    lhsassert_trueresult(vm, false);

    return (LHSValue*)lhsvector_back(vm, &vm->stack);
}

size_t lhsvm_gettop(LHSVM* vm)
{
    lhsassert_trueresult(vm && vm->currentframe, 0);

    return lhsvector_length(vm, &vm->stack) - lhsframe_castcurframe(vm)->base;
}

int lhsvm_pop(LHSVM* vm, size_t n)
{
    lhsassert_trueresult(vm && n > 0, false);

    lhsvector_pop(vm, &vm->stack, n);
    return true;
}

void lhsvm_destroy(LHSVM* vm)
{
    lhsassert_truereturn(vm);

    lhshash_uninit(vm, &vm->conststr);
    lhsvector_uninit(vm, &vm->stack);
    lhsslink_foreach(LHSGCObject, vm, allgc, next, lhsvm_allgcfree);
}
