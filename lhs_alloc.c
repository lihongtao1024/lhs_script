#include "lhs_alloc.h"
#include "lhs_assert.h"
#include "lhs_vm.h"
#include "lhs_hash.h"
#include "lhs_value.h"
#include "lhs_link.h"

void* lhsmem_realloc(void* vm, void* original, size_t osize, 
    size_t nsize)
{
    unused(vm);
    unused(osize);

    if (!nsize)
    {
        free(original);
        return 0;
    }

    return realloc(original, nsize);
}

LHSGCObject* lhsmem_newgcobject(void* vm, size_t size, int type)
{
    lhsassert_trueresult(vm, 0);

    LHSGCObject* o = lhsgc_castgc(lhsmem_newobject(lhsvm_castvm(vm), size));
    lhsassert_trueresult(o, 0);
    lhsmem_initgc(o, type);   

    lhsslink_push(lhsvm_castvm(vm), allgc, o, next);
    return o;
}

LHSGCObject* lhsmem_newgclstring(void* vm, void* data, size_t l, int extra)
{
    lhsassert_trueresult(vm, 0);

    int is_short = (l < LHS_SHORTSTRLEN);
    LHSString* str = 0;
    if (is_short)
    {
        str = lhsvalue_caststring
        (
            lhsmem_newgcobject(vm, sizeof(LHSShortString), LHS_TGCSTRING)
        );
        lhsassert_trueresult(str, 0);        
    }
    else
    {
        str = lhsvalue_caststring
        (
            lhsmem_newgcobject(vm, sizeof(LHSString) + l + 1, LHS_TGCSTRING)
        );
        lhsassert_trueresult(str, 0);
    }

    str->extra = extra;
    str->data[l] = 0;
    str->length = l;
    str->hash = 0;
    memcpy(str->data, data, l);

    if (is_short)
    {
        lhshash_insert(vm, &lhsvm_castvm(vm)->conststr, str, &str->hash);
    }

    return lhsgc_castgc(str);
}
