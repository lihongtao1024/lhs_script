#include "lhs_hash.h"
#include "lhs_assert.h"
#include "lhs_vm.h"

#define hashsize            64
#define lhshash_mod(h, l)   ((h) & ((l) - 1))
#define lhshash_castdata(o) *(void**)(((char *)(o)) + offsetof(LHSHashNode, data))

static int lhshash_rehash(LHSHashNode** nodes, size_t osize, size_t nsize)
{
    lhsassert_trueresult(nodes, false);

    for (size_t i = osize; i < nsize; ++i)
    {
        nodes[i] = 0;
    }

    for (size_t i = 0; i < osize; ++i)
    {
        LHSHashNode* node = nodes[i];
        nodes[i] = 0;

        while (node)
        {
            LHSHashNode* next = node->next;
            size_t slot = lhshash_mod(node->hash, nsize);
            node->next = nodes[slot];
            nodes[slot] = node;
            node = next;
        }
    }
    return true;
}

static int lhshash_grow(void* vm, LHSHashTable* hash, size_t nsize)
{
    lhsassert_trueresult(vm && hash, false);

    if (hash->size == nsize)
    {
        return true;
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
        lhsassert_trueresult(hash->nodes, false);
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
        lhsassert_trueresult(hash->nodes, false);
    }

    hash->size = nsize;
    return true;
}

static LHSHashNode* lhshash_search(LHSHashTable* hash, void *userdata, 
    long long h, LHSHashNode*** output)
{
    lhsassert_trueresult(hash && hash->equal && userdata, 0);

    LHSHashNode** list = &hash->nodes[lhshash_mod(h, hash->size)];
    for (LHSHashNode* node = *list; node; node = node->next)
    {
        void* data = lhshash_castdata(node);
        if (node->hash == h &&
            hash->equal(userdata, data))
        {
            *output = list;
            return node;
        }
    }

    *output = list;
    return 0;
}

int lhshash_init(void* vm, LHSHashTable* hash, lhshash_calc calc, 
    lhshash_equal comp)
{
    lhsassert_trueresult(vm && hash && calc && comp, false);

    hash->calc = calc;
    hash->equal = comp;
    hash->usize = 0;
    hash->size = 0;
    hash->nodes = 0;
    return lhshash_grow(vm, hash, hashsize);
}

int lhshash_insert(void* vm, LHSHashTable* hash, void* userdata, 
    long long* ohash)
{
    lhsassert_trueresult
    (
        vm && hash && hash->calc && hash->equal && userdata, 
        false
    );

    long long h = hash->calc(userdata);
    LHSHashNode** list = 0, * node = 0;
    node = lhshash_search(hash, userdata, h, &list);
    if (node)
    {
        node->data = userdata;
        if (ohash)
        {
            *ohash = h;
        }
        return true;
    }

    if (hash->usize >= (hash->size >> 1))
    {
        if (!lhshash_grow(vm, hash, hash->size << 1))
        {
            return false;
        }

        list = &hash->nodes[lhshash_mod(h, hash->size)];
    }

    node = lhsmem_newobject(lhsvm_castvm(vm), sizeof(LHSHashNode));
    lhsassert_trueresult(node, false);

    node->hash = h;
    node->data = userdata;

    if (ohash)
    {
        *ohash = h;
    }
    
    node->next = *list;
    *list = node;

    ++hash->usize;
    return true;
}

void* lhshash_find(void* vm, LHSHashTable* hash, void* userdata)
{
    lhsassert_trueresult
    (
        vm && hash && hash->calc && userdata, 
        0
    );

    long long h = hash->calc(userdata);
    LHSHashNode** list = 0, * node = 0;
    node = lhshash_search(hash, userdata, h, &list);
    if (!node)
    {
        return 0;
    }

    return node->data;
}

void lhshash_remove(void* vm, LHSHashTable* hash, void* userdata)
{
    lhsassert_truereturn
    (
        vm && hash && hash->calc && userdata
    );

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
    lhsassert_truereturn(vm && hash);

    for (size_t i = 0; i < hash->size; ++i)
    {
        LHSHashNode* node = hash->nodes[i];
        while (node)
        {
            LHSHashNode* next = node->next;
            lhsmem_freeobject(lhsvm_castvm(vm), node, sizeof(LHSHashNode));
            node = next;
        }
    }

    lhsmem_freeobject
    (
        lhsvm_castvm(vm), 
        hash->nodes, sizeof(LHSHashNode*)* hash->size
    );

    hash->size = 0;
    hash->usize = 0;
}
