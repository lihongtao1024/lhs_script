#include "lhs_vector.h"
#include "lhs_assert.h"
#include "lhs_vm.h"

#define vectorsize  64
#define lhsvector_castsize(v, x) ((x) * (v)->esize)
#define lhsvector_castat(v, x)   ((char*)((v)->nodes) + lhsvector_castsize(v, x))

static int lhsvector_grow(void* vm, LHSVector* vector, 
    size_t osize, size_t nsize)
{
    lhsassert_trueresult(vm && vector, false);

    if (nsize < vectorsize)
    {
        return true;
    }

    vector->nodes = lhsmem_renewobject
    (
        lhsvm_castvm(vm), 
        vector->nodes, 
        osize * vector->esize, 
        nsize * vector->esize
    );
    vector->size = nsize;
    lhsassert_trueresult(vector->nodes, false);
    return true;
}

int lhsvector_init(void* vm, LHSVector* vector, size_t esize)
{
    lhsassert_trueresult(vm && vector, false);

    vector->esize = esize;
    vector->nodes = 0;
    vector->size = 0;
    vector->usize = 0;
    return lhsvector_grow(vm, vector, 0, vectorsize);
}

int lhsvector_push(void* vm, LHSVector* vector, void* data, 
    size_t* index)
{
    lhsassert_trueresult(vm && vector, false);

    if (vector->usize >= vector->size &&
        !lhsvector_grow(vm, vector, vector->size, vector->size << 1))
    {
        return false;
    }

    memcpy(lhsvector_castat(vector, vector->usize), data, vector->esize);
    *index = vector->usize++;
    return true;
}

void lhsvector_pop(void* vm, LHSVector* vector, size_t n)
{
    lhsassert_truereturn(vm && vector && n > 0 && n <= vector->usize);

    vector->usize -= n;

    if (vector->usize <= vector->size >> 2)
    {
        lhsvector_grow(vm, vector, vector->size, vector->size >> 1);
    }
}

void lhsvector_remove(void* vm, LHSVector* vector, size_t index)
{
    lhsassert_truereturn(vm && vector);

    if (index >= vector->usize)
    {
        return;
    }

    memmove
    (
        lhsvector_castat(vector, index), 
        lhsvector_castat(vector, index + 1), 
        lhsvector_castsize(vector, vector->size - index - 1)
    );

    if (--vector->usize <= vector->size >> 2)
    {
        lhsvector_grow(vm, vector, vector->size, vector->size >> 1);
    }
}

void* lhsvector_at(void* vm, LHSVector* vector, size_t index)
{
    lhsassert_trueresult(vm && vector, 0);

    if (index >= vector->usize)
    {
        return 0;
    }

    return lhsvector_castat(vector, index);
}

void* lhsvector_increment(void* vm, LHSVector* vector)
{
    lhsassert_trueresult(vm && vector, 0);

    if (vector->usize >= vector->size)
    {
        lhsvector_grow(vm, vector, vector->size, vector->size << 1);
    }

    void* element = lhsvector_castat(vector, vector->usize);
    ++vector->usize;

    return element;
}

void* lhsvector_back(void* vm, LHSVector* vector)
{
    lhsassert_trueresult(vm && vector, 0);

    if (vector->usize <= 0)
    {
        return 0;
    }

    return lhsvector_castat(vector, vector->usize - 1);
}

size_t lhsvector_length(void* vm, LHSVector* vector)
{
    lhsassert_trueresult(vm && vector, 0);

    return vector->usize;
}

void lhsvector_uninit(void* vm, LHSVector* vector)
{
    lhsassert_truereturn(vm && vector);
    
    if (vector->size)
    {
        lhsmem_freeobject
        (
            lhsvm_castvm(vm), 
            vector->nodes, 
            lhsvector_castsize(vector, vector->size)
        );
    }

    vector->size = 0;
    vector->usize = 0;
}
