#include "lhs_vm.h"
#include "lhs_link.h"
#include "lhs_function.h"
#include "lhs_value.h"
#include "lhs_variable.h"
#include "lhs_parser.h"
#include "lhs_code.h"
#include "lhs_baselib.h"
#include "lhs_execute.h"

#define lhsvm_gettopstring(vm)                                                  \
(lhsvalue_caststring(lhsvm_gettopvalue(vm)->gc))

static void lhsvm_allgcfree(LHSVM* vm, LHSGCObject* o, void* ud)
{
    if ((void*)vm == (void*)o)
    {
        printf("memory leak:[%llu] bytes.\n", vm->nalloc - o->size);
    }

    if (o->type == LHS_TGCFUNCTION)
    {
        lhsfunction_uninit(vm, lhsfunction_castfunc(o));
    }

    lhsmem_freeobject(vm, o, o->size);
}

static void lhsvm_init(LHSVM* vm, lhsmem_new fn)
{
    vm->alloc = fn;
    vm->mainframe = 0;
    vm->callcontext = 0;
    vm->top = 0;
    vm->ncallcontext = 0;

    lhslink_init(vm, allgc);
    lhslink_forward(vm, allgc, &vm->gc, next);
    lhslink_init(vm, errorjmp);
    lhshash_init(vm, &vm->shortstrhash, lhsvalue_hashstr, lhsvalue_equalstr, 16);
    lhshash_init(vm, &vm->conststrhash, lhsvar_hashvar, lhsvar_equalvar, 4);
    lhshash_init(vm, &vm->globalvars, lhsvar_hashvar, lhsvar_equalvar, 4);
    lhsvector_init(vm, &vm->conststrs, sizeof(LHSVar), 4);
    lhsvector_init(vm, &vm->globalvalues, sizeof(LHSVar), 4);
    lhsvector_init(vm, &vm->stack, sizeof(LHSValue), 16);
}

const LHSString* lhsvm_insertshortstr(LHSVM* vm, const char* str, 
    size_t l, int reserved)
{
    lhserr_check(vm, l < LHS_SHORTSTRLEN, "illegal size of short string.");

    LHSShortString fs;
    memcpy(fs.data, str, l);
    fs.data[l] = 0;
    fs.length = (int)l;  

    LHSString* ss = lhshash_find(vm, &vm->shortstrhash, &fs);
    if (ss)
    {
        return ss;
    }

    ss = lhsvalue_caststring
    (
        lhsmem_newgcobject(vm, sizeof(LHSShortString), LHS_TGCSTRING)
    );
    memcpy(ss->data, str, l);
    ss->data[l] = 0;
    ss->length = (int)l;
    ss->reserved = reserved;    
    lhshash_insert(vm, &vm->shortstrhash, ss, &ss->hash);
    return ss;
}

LHSValue* lhsvm_incrementstack(LHSVM* vm)
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
        fn = lhsmem_alloc;
    }

    size_t size = sizeof(LHSVM);
    LHSVM* vm = (LHSVM *)fn(0, 0, 0, size);
    vm->nalloc = size;
    lhsmem_initgc(lhsgc_castgc(vm), LHS_TGCVM, size);

    lhsvm_init(vm, fn);
    return vm;
}

void lhsvm_destroy(LHSVM* vm)
{
    lhshash_uninit(vm, &vm->conststrhash);
    lhshash_uninit(vm, &vm->shortstrhash);
    lhsvector_uninit(vm, &vm->conststrs);
    lhshash_uninit(vm, &vm->globalvars);
    lhsvector_uninit(vm, &vm->globalvalues);
    lhsvector_uninit(vm, &vm->stack);
    lhslink_foreach(LHSGCObject, vm, allgc, next, lhsvm_allgcfree, 0);
}

int lhsvm_dofile(LHSVM* vm, const char* name)
{
    if (!lhsreg_baselib(vm))
    {
        return LHS_FALSE;
    }

    if (!lhsparser_loadfile(vm, name))
    {
        return LHS_FALSE;
    }

    if (!lhsexec_pcall(vm, 0, LHS_UNCERTAIN, 0))
    {
        return LHS_FALSE;
    }

    return LHS_TRUE;
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
    const LHSValue* value = lhsvm_getvalue(vm, index);
    LHSValue* top = lhsvm_incrementstack(vm);
    memcpy(top, value, sizeof(LHSValue));
    return LHS_TRUE;
}

int lhsvm_pushlstring(LHSVM* vm, const char* str, size_t l)
{
    LHSString* string = 0;
    if (l < LHS_SHORTSTRLEN)
    {
        string = lhsvalue_caststring
        (
            lhsvm_insertshortstr(vm, (void*)str, (char)l, 0)
        );
    }
    else
    {
        string = lhsvalue_caststring
        (
            lhsmem_newgcobject
            (
                vm, 
                sizeof(LHSString) + l + sizeof(char), 
                LHS_TGCSTRING
            )
        );
        memcpy(string->data, str, l);
        string->data[l] = 0;
        string->length = (int)l;
        string->reserved = 0;
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

int lhsvm_pushdelegate(LHSVM* vm, lhsvm_delegate delegate)
{
    LHSValue* value = lhsvm_incrementstack(vm);
    value->type = LHS_TDELEGATE;
    value->dg = delegate;

    return LHS_TRUE;
}

const LHSValue* lhsvm_getvalue(LHSVM* vm, int index)
{
    LHSValue* value = 0;

    if (index < 0)
    {
        value = lhsvector_at(vm, &vm->stack,  vm->top + index);
    }
    else if (index > 0)
    {
        value = lhsvector_at
        (
            vm, 
            &vm->stack, 
            lhsexec_castcc(vm->callcontext)->base + index - 1
        );
    }

    return value;
}

const char* lhsvm_tostring(LHSVM* vm, int index)
{
    const LHSValue* value = lhsvm_getvalue(vm, index);

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
        case LHS_TGCFUNCTION:
        {
            l = sprintf(buf, "function:%p", value->gc);
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
            l = sprintf(buf, "fulldata:%p", value->gc);
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

double lhsvm_tonumber(LHSVM* vm, int index)
{
    const LHSValue* value = lhsvm_getvalue(vm, index);
    switch (value->type)
    {
    case LHS_TINTEGER:
    {
        return (double)value->i;
    }
    case LHS_TNUMBER:
    {
        return value->n;
    }
    case LHS_TBOOLEAN:
    {
        return value->b;
    }
    case LHS_TGC:
    {
        if (value->gc->type != LHS_TGCSTRING)
        {
            break;
        }
        return atof(lhsvalue_caststring(value->gc)->data);
    }
    default:
    {
        break;
    }
    }

    return 0;
}

long long lhsvm_tointeger(LHSVM* vm, int index)
{
    const LHSValue* value = lhsvm_getvalue(vm, index);
    switch (value->type)
    {
    case LHS_TINTEGER:
    {
        return value->i;
    }
    case LHS_TNUMBER:
    {
        return (long long)value->n;
    }
    case LHS_TBOOLEAN:
    {
        return value->b;
    }
    case LHS_TGC:
    {
        if (value->gc->type != LHS_TGCSTRING)
        {
            break;
        }
        return atoll(lhsvalue_caststring(value->gc)->data);
    }
    default:
    {
        break;
    }
    }

    return 0;
}

int lhsvm_gettop(LHSVM* vm)
{
    if (!vm->callcontext)
    {
        return (int)vm->top;
    }

    return (int)(vm->top - lhsexec_castcc(vm->callcontext)->base);
}

int lhsvm_setglobal(LHSVM* vm, const char* name)
{
    const LHSValue* value = lhsvm_getvalue(vm, -1);
    lhsvm_pushstring(vm, name);

    LHSVarDesc fdesc;
    fdesc.chunk = 0;
    fdesc.name = lhsvalue_caststring(lhsvm_getvalue(vm, -1)->gc);

    LHSVarDesc* desc = lhshash_find(vm, &vm->globalvars, &fdesc);
    if (desc)
    {
        LHSVar* var = lhsvector_at(vm, &vm->globalvalues, desc->index);
        memcpy(&var->value, value, sizeof(LHSValue));
        lhsvm_pop(vm, 2);
        return LHS_TRUE;
    }

    desc = lhsvar_castvardesc
    (
        lhsmem_newgcobject(vm, sizeof(LHSVarDesc), LHS_TGCFULLDATA)
    );
    desc->chunk = fdesc.chunk;
    desc->name = fdesc.name;

    LHSVar *var = lhsvector_increment(vm, &vm->globalvalues);
    memcpy(&var->value, value, sizeof(LHSValue));
    var->desc = desc;

    desc->line = 0;
    desc->column = 0;  
    desc->index = (int)vm->globalvalues.usize - 1;
    desc->mark = LHS_MARKGLOBAL;

    lhshash_insert(vm, &vm->globalvars, desc, 0);
    lhsvm_pop(vm, 2);
    return LHS_TRUE;
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
