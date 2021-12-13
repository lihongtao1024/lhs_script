#include "lhs_vector.h"
#include "lhs_vm.h"

#define LHS_VECTORSIZE           4
#define lhsvector_castsize(v, x) ((x) * (v)->esize)
#define lhsvector_castat(v, x)   ((char*)((v)->nodes) + lhsvector_castsize(v, x))

static int lhsvector_grow(void* vm, LHSVector* vector, 
    size_t osize, size_t nsize)
{
    if (nsize < LHS_VECTORSIZE)
    {
        return LHS_TRUE;
    }

    vector->nodes = lhsmem_renewobject
    (
        lhsvm_castvm(vm), 
        vector->nodes, 
        osize * vector->esize, 
        nsize * vector->esize
    );
    vector->size = nsize;
    return LHS_TRUE;
}

int lhsvector_init(void* vm, LHSVector* vector, size_t esize, size_t n)
{
    vector->esize = esize;
    vector->nodes = 0;
    vector->size = 0;
    vector->usize = 0;
    return lhsvector_grow
    (
        vm, 
        vector, 
        0, 
        n ? max(n, LHS_VECTORSIZE) : LHS_VECTORSIZE
    );
}

int lhsvector_push(void* vm, LHSVector* vector, void* data, 
    size_t* index)
{
    if (vector->usize >= vector->size &&
        !lhsvector_grow(vm, vector, vector->size, vector->size << 1))
    {
        return LHS_FALSE;
    }

    memcpy(lhsvector_castat(vector, vector->usize), data, vector->esize);
    
    if (index)
    {
        *index = vector->usize;
    }
    
    ++vector->usize;
    return LHS_TRUE;
}

void lhsvector_pop(void* vm, LHSVector* vector, size_t n)
{
    vector->usize -= n;

    if (vector->usize <= vector->size >> 2)
    {
        lhsvector_grow(vm, vector, vector->size, vector->size >> 1);
    }
}

void lhsvector_remove(void* vm, LHSVector* vector, size_t index)
{
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
    if (index >= vector->usize)
    {
        return 0;
    }

    return lhsvector_castat(vector, index);
}

void* lhsvector_increment(void* vm, LHSVector* vector)
{
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
    if (vector->usize <= 0)
    {
        return 0;
    }

    return lhsvector_castat(vector, vector->usize - 1);
}

size_t lhsvector_length(void* vm, LHSVector* vector)
{
    return vector->usize;
}

void lhsvector_foreach(void* vm, LHSVector* vector, 
    lhsvector_iterator iterator)
{
    for (size_t i = 0; i < vector->usize; ++i)
    {
        iterator(vm, vector, lhsvector_castat(vector, i));
    }
}

void lhsvector_uninit(void* vm, LHSVector* vector)
{    
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
