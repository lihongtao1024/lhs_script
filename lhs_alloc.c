#include "lhs_alloc.h"
#include "lhs_vm.h"
#include "lhs_hash.h"
#include "lhs_value.h"
#include "lhs_link.h"

void* lhsmem_alloc(void* vm, void* original, size_t osize, 
    size_t nsize)
{
    if (!nsize)
    {
        free(original);
        return 0;
    }

    return realloc(original, nsize);
}

void* lhsmem_newobject(void* vm, size_t size)
{
    void* data = lhsvm_castvm(vm)->alloc(vm, 0, 0, size);
    if (data)
    {
        lhsvm_castvm(vm)->nalloc += size;
    }

    return data;
}

void* lhsmem_renewobject(void* vm, void* original, size_t osize, size_t nsize)
{
    void* data = lhsvm_castvm(vm)->alloc(vm, original, osize, nsize);
    if (data)
    {
        lhsvm_castvm(vm)->nalloc = lhsvm_castvm(vm)->nalloc - osize + nsize;
    }

    return data;
}

void lhsmem_freeobject(void* vm, void* data, size_t size)
{
    if (!lhsvm_castvm(vm)->alloc(vm, data, size, 0))
    {
        lhsvm_castvm(vm)->nalloc -= size;
    }
}

LHSGC* lhsmem_newgcobject(void* vm, size_t size, int type)
{
    LHSGC* o = lhsgc_castgc(lhsmem_newobject(lhsvm_castvm(vm), size));
    lhsmem_initgc(o, type, size); 
    lhslink_forward(lhsvm_castvm(vm), allgc, o, next);
    return o;
}
