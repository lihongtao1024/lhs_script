#include "lhs_hash.h"
#include "lhs_vm.h"

#define LHS_HASHSIZE            4
#define lhshash_mod(h, l)       ((h) & ((l) - 1))
#define lhshash_castdata(o)                                             \
*(void**)(((char *)(o)) + offsetof(LHSHashNode, data))

static int lhshash_rehash(LHSHashNode** nodes, size_t osize, size_t nsize)
{
    for (size_t i = osize; i < nsize; ++i)
    {
        nodes[i] = 0;
    }

    for (size_t i = 0; i < osize; ++i)
    {
        LHSHashNode* chain = nodes[i];
        nodes[i] = 0;

        while (chain)
        {
            LHSHashNode* next = chain->next;
            size_t slot = lhshash_mod(chain->hash, nsize);
            chain->next = nodes[slot];
            nodes[slot] = chain;
            chain = next;
        }
    }
    return LHS_TRUE;
}

static int lhshash_grow(void* vm, LHSHashTable* hash, size_t nsize)
{
    if (hash->size == nsize)
    {
        return LHS_TRUE;
    }

    if (hash->size < nsize)
    {
        hash->nodes = lhsmem_renewobject
        (
            lhsvm_castvm(vm), 
            hash->nodes, 
            sizeof(LHSHashNode*) * hash->size, 
            sizeof(LHSHashNode*) * nsize
        );
        lhshash_rehash(hash->nodes, hash->size, nsize);
    }
    else
    {
        lhshash_rehash(hash->nodes, nsize, hash->size);
        hash->nodes = lhsmem_renewobject
        (
            lhsvm_castvm(vm), 
            hash->nodes, 
            sizeof(LHSHashNode*) * hash->size, 
            sizeof(LHSHashNode*) * nsize
        );
    }

    hash->size = nsize;
    return LHS_TRUE;
}

static LHSHashNode* lhshash_search(LHSHashTable* hash, void *userdata, 
    long long h, LHSHashNode*** output)
{
    LHSHashNode** list = &hash->nodes[lhshash_mod(h, hash->size)];
    for (LHSHashNode* chain = *list; chain; chain = chain->next)
    {
        void* data = lhshash_castdata(chain);
        if (chain->hash == h &&
            hash->equal(userdata, data))
        {
            *output = list;
            return chain;
        }
    }

    *output = list;
    return 0;
}

int lhshash_init(void* vm, LHSHashTable* hash, lhshash_calc calc, 
    lhshash_equal comp, size_t n)
{
    hash->calc = calc;
    hash->equal = comp;
    hash->usize = 0;
    hash->size = 0;
    hash->nodes = 0;
    return lhshash_grow(vm, hash, n ? max(n, LHS_HASHSIZE) : LHS_HASHSIZE);
}

int lhshash_insert(void* vm, LHSHashTable* hash, void* userdata, 
    long long* ohash)
{
    long long h = hash->calc(userdata);
    LHSHashNode** list = 0, * chain = 0;
    chain = lhshash_search(hash, userdata, h, &list);
    if (chain)
    {
        chain->data = userdata;
        if (ohash)
        {
            *ohash = h;
        }
        return LHS_TRUE;
    }

    if (hash->usize >= (hash->size >> 1))
    {
        if (!lhshash_grow(vm, hash, hash->size << 1))
        {
            return LHS_FALSE;
        }

        list = &hash->nodes[lhshash_mod(h, hash->size)];
    }

    chain = lhsmem_newobject(lhsvm_castvm(vm), sizeof(LHSHashNode));
    chain->hash = h;
    chain->data = userdata;

    if (ohash)
    {
        *ohash = h;
    }
    
    chain->next = *list;
    *list = chain;

    ++hash->usize;
    return LHS_TRUE;
}

void* lhshash_find(void* vm, LHSHashTable* hash, void* userdata)
{
    long long h = hash->calc(userdata);
    LHSHashNode** list = 0, * chain = 0;
    chain = lhshash_search(hash, userdata, h, &list);
    if (!chain)
    {
        return 0;
    }

    return chain->data;
}

void lhshash_remove(void* vm, LHSHashTable* hash, void* userdata)
{
    long long h = hash->calc(userdata);
    LHSHashNode** current = &hash->nodes[lhshash_mod(h, hash->size)];
    for (; *current; )
    {
        LHSHashNode* temp = *current;
        void* data = lhshash_castdata(temp);
        if (hash->equal(userdata, data))
        {
            *current = temp->next;
            lhsmem_freeobject(lhsvm_castvm(vm), temp, sizeof(LHSHashNode));
            --hash->usize;
        }
        else
        {
            current = &temp->next;
        }
    }
}

void lhshash_uninit(void* vm, LHSHashTable* hash)
{
    for (size_t i = 0; i < hash->size; ++i)
    {
        LHSHashNode* chain = hash->nodes[i];
        while (chain)
        {
            LHSHashNode* next = chain->next;
            lhsmem_freeobject(lhsvm_castvm(vm), chain, sizeof(LHSHashNode));
            chain = next;
        }
    }

    lhsmem_freeobject
    (
        lhsvm_castvm(vm), 
        hash->nodes, 
        sizeof(LHSHashNode*) * hash->size
    );

    hash->size = 0;
    hash->usize = 0;
}
