#include "lhs_link.h"
#include "lhs_frame.h"
#include "lhs_vm.h"

#define lhsvm_gettopstring(vm)                                              \
(lhsvalue_caststring(lhsvm_gettopvalue(vm)->gc))

static void lhsvm_init(LHSVM* vm, lhsmem_new fn)
{
    vm->falloc = fn;
    vm->mainframe = 0;
    vm->currentframe = 0;
    vm->allgc = 0;
    vm->top = 0;
    vm->errorjmp = 0;

    lhsslink_push(vm, allgc, &vm->gc, next);
    lhshash_init(vm, &vm->shortstrhash, lhsvalue_hashstr, lhsvalue_equalstr, 16);
    lhshash_init(vm, &vm->conststrhash, lhsvariable_hashvar, lhsvariable_equalvar, 16);
    lhsvector_init(vm, &vm->conststrvalue, sizeof(LHSValue), 16);
    lhsvector_init(vm, &vm->stack, sizeof(LHSValue), 128);
    lhsbuf_init(vm, &vm->code);
}

static void lhsvm_allgcfree(LHSVM* vm, LHSGCObject* o, void* ud)
{
    lhs_unused(ud);

    if ((void*)vm == (void*)o)
    {
        printf("memory leak:[%llu] bytes\n", vm->nalloc - o->size);
    }

    switch (o->type)
    {
    case LHS_TGCFRAME:
    {
        lhsframe_uninit(vm, lhsframe_castframe(o));
        break;
    }
    }

    lhsmem_freeobject(vm, o, o->size);
}

static LHSValue* lhsvm_incrementstack(LHSVM* vm)
{
    if (vm->top < vm->stack.usize)
    {
        return lhsvalue_castvalue(lhsvector_at(vm, &vm->stack, vm->top++));
    }
    
    ++vm->top;
    return lhsvalue_castvalue(lhsvector_increment(vm, &vm->stack));
}

static LHSValue* lhsvm_gettopvalue(LHSVM* vm)
{
    return lhsvalue_castvalue(lhsvector_at(vm, &vm->stack, vm->top - 1));
}

LHSVM* lhsvm_create(lhsmem_new fn)
{
    if (!fn)
    {
        fn = lhsmem_default;
    }

    size_t size = sizeof(LHSVM);
    LHSVM* vm = (LHSVM *)fn(0, 0, 0, size);
    vm->nalloc = size;
    lhsmem_initgc(lhsgc_castgc(vm), LHS_TGCVM, size);

    lhsvm_init(vm, fn);
    return vm;
}

int lhsvm_pushnil(LHSVM* vm)
{
    LHSValue* value = lhsvm_incrementstack(vm);
    value->type = LHS_TNONE;
    return LHS_TRUE;
}

int lhsvm_pushboolean(LHSVM* vm, char b)
{
    LHSValue* value = lhsvm_incrementstack(vm);
    value->type = LHS_TBOOLEAN;
    value->b = (b ? 1 : 0);
    return LHS_TRUE;
}

int lhsvm_pushvalue(LHSVM* vm, int index)
{
    LHSValue* value = lhsvm_getvalue(vm, index);
    LHSValue* top = lhsvm_incrementstack(vm);
    memcpy(top, value, sizeof(LHSValue));
    return LHS_TRUE;
}

int lhsvm_pushlstring(LHSVM* vm, const char* str, size_t l)
{
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
    }

    LHSValue* value = lhsvm_incrementstack(vm);
    value->type = LHS_TGC;
    value->gc = lhsgc_castgc(string);
    return LHS_TRUE;
}

int lhsvm_pushnumber(LHSVM* vm, double number)
{
    LHSValue* value = lhsvm_incrementstack(vm);
    value->type = LHS_TNUMBER;
    value->n = number;

    return LHS_TRUE;
}

int lhsvm_pushinteger(LHSVM* vm, long long integer)
{
    LHSValue* value = lhsvm_incrementstack(vm);
    value->type = LHS_TINTEGER;
    value->i = integer;

    return LHS_TRUE;
}

LHSValue* lhsvm_getvalue(LHSVM* vm, int index)
{
    LHSValue* value = 0;
    if (index < 0)
    {
        value = lhsvector_at
        (
            vm, 
            &vm->stack, 
            vm->top + index
        );
    }
    else if (index > 0)
    {

    }

    return value;
}

const char* lhsvm_tostring(LHSVM* vm, int index)
{
    LHSValue* value = lhsvm_getvalue(vm, index);

    const char* str = 0;
    char buf[128]; size_t l;
    switch (value->type)
    {
    case LHS_TINTEGER:
    {
        int l = sprintf(buf, "%lld", value->i);
        lhsvm_pushlstring(vm, buf, (size_t)l);
        str = lhsvm_gettopstring(vm)->data;
        break;
    }
    case LHS_TNUMBER:
    {
        int l = sprintf(buf, "%lf", value->n);
        lhsvm_pushlstring(vm, buf, (size_t)l);
        str = lhsvm_gettopstring(vm)->data;
        break;
    }
    case LHS_TBOOLEAN:
    {
        lhsvm_pushstring(vm, value->b ? "true" : "false");
        str = lhsvm_gettopstring(vm)->data;
        break;
    }
    case LHS_TGC:
    {
        switch (value->gc->type)
        {
        case LHS_TGCVM:
        {
            l = sprintf(buf, "vm:%p", value->gc);
            lhsvm_pushlstring(vm, buf, l);
            str = lhsvm_gettopstring(vm)->data;
            break;
        }
        case LHS_TGCFRAME:
        {
            l = sprintf(buf, "frame:%p", value->gc);
            lhsvm_pushlstring(vm, buf, l);
            str = lhsvm_gettopstring(vm)->data;
            break;
        }
        case LHS_TGCSTRING:
        {
            LHSValue* top = lhsvm_incrementstack(vm);
            memcpy(top, value, sizeof(LHSValue));
            str = lhsvalue_caststring(value->gc)->data;
            break;
        }
        case LHS_TGCFULLDATA:
        {
            l = sprintf(buf, "fdata:%p", value->gc);
            lhsvm_pushlstring(vm, buf, l);
            str = lhsvm_gettopstring(vm)->data;
            break;
        }
        default:
        {
            lhsvm_pushstring(vm, "null");
            str = lhsvm_gettopstring(vm)->data;
        }
        }
        break;
    }
    default:
    {
        lhsvm_pushstring(vm, "null");
        str = lhsvm_gettopstring(vm)->data;
        break;
    }
    }

    return str;
}

LHSString* lhsvm_findshort(LHSVM* vm, void* data, size_t l)
{
    LHSShortString ss;
    ss.length = l;
    memcpy(ss.data, data, l);

    return lhshash_find(vm, &vm->shortstrhash, &ss);
}

size_t lhsvm_gettop(LHSVM* vm)
{
    return 0;
}

int lhsvm_pop(LHSVM* vm, size_t n)
{
    if (n > vm->top)
    {
        lhserr_throw(vm, "incorrect stack top.");
    }
    
    vm->top -= n;
    return LHS_TRUE;
}

LHSVariable* lhsvm_insertconstant(LHSVM* vm)
{
    LHSValue* key = lhsvm_getvalue(vm, -1);

    size_t size = sizeof(LHSVariable);
    LHSVariable* nvariable = lhsmem_newobject(vm, size);
    nvariable->name = lhsvalue_caststring(key->gc);

    LHSVariable* ovariable = lhshash_find(vm, &vm->conststrhash, nvariable);
    if (ovariable)
    {
        lhsmem_freeobject(vm, nvariable, sizeof(LHSVariable));
        lhsvm_pop(vm, 1);
        return ovariable;
    }

    lhsmem_initgc(lhsgc_castgc(&nvariable->gc), LHS_TGCFULLDATA, size);
    lhsslink_push(lhsvm_castvm(vm), allgc, lhsgc_castgc(&nvariable->gc), next);
    lhshash_insert(vm, &vm->conststrhash, nvariable, 0);

    LHSValue* value = lhsvector_increment(vm, &vm->conststrvalue);
    memcpy(value, key, size);

    nvariable->index = (int)lhsvector_length(vm, &vm->conststrvalue) - 1;
    nvariable->mark = LHS_MARKSTRING;
    
    lhsvm_pop(vm, 1);
    return nvariable;
}

void lhsvm_destroy(LHSVM* vm)
{
    lhshash_uninit(vm, &vm->conststrhash);
    lhshash_uninit(vm, &vm->shortstrhash);
    lhsvector_uninit(vm, &vm->conststrvalue);
    lhsvector_uninit(vm, &vm->stack);
    lhsbuf_uninit(vm, &vm->code);
    lhsslink_foreach(LHSGCObject, vm, allgc, next, lhsvm_allgcfree, 0);
}
