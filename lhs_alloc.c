#include "lhs_alloc.h"
#include "lhs_vm.h"
#include "lhs_hash.h"
#include "lhs_value.h"
#include "lhs_link.h"

void* lhsmem_default(void* vm, void* original, size_t osize, 
    size_t nsize)
{
    lhs_unused(vm);
    lhs_unused(osize);

    if (!nsize)
    {
        free(original);
        return 0;
    }

    return realloc(original, nsize);
}

void* lhsmem_newobject(void* vm, size_t size)
{
    void* data = lhsvm_castvm(vm)->falloc(vm, 0, 0, size);
    if (data)
    {
        lhsvm_castvm(vm)->nalloc += size;
    }

    return data;
}

void* lhsmem_renewobject(void* vm, void* original, size_t osize, size_t nsize)
{
    void* data = lhsvm_castvm(vm)->falloc(vm, original, osize, nsize);
    if (data)
    {
        lhsvm_castvm(vm)->nalloc = lhsvm_castvm(vm)->nalloc - osize + nsize;
    }

    return data;
}

void lhsmem_freeobject(void* vm, void* data, size_t size)
{
    if (!lhsvm_castvm(vm)->falloc(vm, data, size, 0))
    {
        lhsvm_castvm(vm)->nalloc -= size;
    }
}

LHSGCObject* lhsmem_newgcobject(void* vm, size_t size, int type)
{
    LHSGCObject* o = lhsgc_castgc(lhsmem_newobject(lhsvm_castvm(vm), size));
    lhsmem_initgc(o, type, size);   

    lhsslink_push(lhsvm_castvm(vm), allgc, o, next);
    return o;
}

LHSGCObject* lhsmem_newgclstring(void* vm, void* data, size_t l, int extra)
{
    int is_short = (l < LHS_SHORTSTRLEN);
    LHSString* str = 0;
    if (is_short)
    {
        str = lhsvalue_caststring
        (
            lhsmem_newgcobject(vm, sizeof(LHSShortString), LHS_TGCSTRING)
        );     
    }
    else
    {
        str = lhsvalue_caststring
        (
            lhsmem_newgcobject(vm, sizeof(LHSString) + l + 1, LHS_TGCSTRING)
        );
    }

    str->extra = extra;
    str->data[l] = 0;
    str->length = l;
    str->hash = 0;
    memcpy(str->data, data, l);

    if (is_short)
    {
        lhshash_insert(vm, &lhsvm_castvm(vm)->shortstrhash, str, &str->hash);
    }

    return lhsgc_castgc(str);
}
