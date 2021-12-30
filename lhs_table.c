#include "lhs_table.h"
#include "lhs_vm.h"

#define LHS_TABLESIZE            4
#define lhstable_mod(h, l)       ((h) & ((l) - 1))

typedef union LHSHASH
{
    double n;
    long long i;
    void* p;
} LHSHASH;

static int lhstable_equal(const LHSValue* lkey, const LHSValue* rkey)
{
    if (lkey->type != rkey->type)
    {
        return LHS_FALSE;
    }

    if (lkey->type == LHS_TGC)
    {
        if (lkey->gc->type != rkey->gc->type)
        {
            return LHS_FALSE;
        }
    }

    int result = LHS_FALSE;
    switch (lkey->type)
    {
        case LHS_TINTEGER:
        {
            result = lkey->i == rkey->i;
            break;
        }
        case LHS_TNUMBER:
        {
            result = lkey->n == rkey->n;
            break;
        }
        case LHS_TBOOLEAN:
        {
            result = lkey->b == rkey->b;
            break;
        }
        case LHS_TDELEGATE:
        {
            result = lkey->dg == rkey->dg;
            break;
        }
        case LHS_TGC:
        {
            if (lkey->gc->type == LHS_TGCSTRING)
            {
                LHSString* lstr = lhsvalue_caststring(lkey->gc);
                if (lstr->length < LHS_SHORTSTRLEN)
                {
                    result = lkey->gc == rkey->gc;
                }
                else
                {
                    LHSString* rstr = lhsvalue_caststring(rkey->gc);
                    if (lstr->length == rstr->length)
                    {
                        result = !memcmp(lstr->data, rstr->data, lstr->length);
                    }
                }
            }
            else
            {
                result = lkey->gc == rkey->gc;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return result;
}

long long lhstable_calc(const LHSValue* key)
{
    long long hash = 0;

    switch (key->type)
    {
    case LHS_TINTEGER:
    {
        hash = key->i;
        break;
    }
    case LHS_TNUMBER:
    {
        LHSHASH lhsh;
        lhsh.n = key->n;
        hash = lhsh.i;
        break;
    }
    case LHS_TBOOLEAN:
    {
        hash = ((long long)1 << 31) | key->b;
        break;
    }
    case LHS_TDELEGATE:
    {
        LHSHASH lhsh;
        lhsh.p = key->gc;
        hash = lhsh.i;
        break;
    }
    case LHS_TGC:
    {
        if (key->gc->type == LHS_TGCSTRING)
        {
            LHSString* str = lhsvalue_caststring(key->gc);
            if (!str->hash)
            {
                str->hash = lhsvalue_hashstr(str);
            }
            hash = str->hash;
        }
        else
        {
            LHSHASH lhsh;
            lhsh.p = key->gc;
            hash = lhsh.i;
        }
        break;
    }
    default:
    {
        break;
    }
    }

    return hash;
}

static int lhstable_rehash(LHSTableNode** nodes, size_t osize, size_t nsize)
{
    for (size_t i = osize; i < nsize; ++i)
    {
        nodes[i] = 0;
    }

    for (size_t i = 0; i < osize; ++i)
    {
        LHSTableNode* node = nodes[i];
        nodes[i] = 0;

        while (node)
        {
            LHSTableNode* next = node->next;
            size_t slot = lhstable_mod(node->hash, nsize);
            node->next = nodes[slot];
            nodes[slot] = node;
            node = next;
        }
    }
    return LHS_TRUE;
}

static int lhstable_grow(LHSVM* vm, LHSTable* table, size_t nsize)
{
    if (table->size == nsize ||
        nsize < LHS_TABLESIZE)
    {
        return LHS_TRUE;
    }

    if (table->size < nsize)
    {
        table->nodes = lhsmem_renewobject
        (
            lhsvm_castvm(vm), 
            table->nodes, 
            sizeof(LHSTableNode*) * table->size, 
            sizeof(LHSTableNode*) * nsize
        );
        lhstable_rehash(table->nodes, table->size, nsize);
    }
    else
    {
        lhstable_rehash(table->nodes, table->size, nsize);
        table->nodes = lhsmem_renewobject
        (
            lhsvm_castvm(vm), 
            table->nodes, 
            sizeof(LHSTableNode*) * table->size, 
            sizeof(LHSTableNode*) * nsize
        );
    }

    table->size = nsize;
    return LHS_TRUE;
}

static LHSTableNode* lhstable_search(LHSTable* table, const LHSValue *key, 
    long long h, LHSTableNode*** output)
{
    LHSTableNode** list = &table->nodes[lhstable_mod(h, table->size)];
    for (LHSTableNode* node = *list; node; node = node->next)
    {
        if (node->hash == h &&
            lhstable_equal(&node->key, key))
        {
            *output = list;
            return node;
        }
    }

    *output = list;
    return 0;
}

int lhstable_init(LHSVM* vm, LHSTable* table)
{
    table->usize = 0;
    table->size = 0;
    table->nodes = 0;
    return lhstable_grow(vm, table, LHS_TABLESIZE);
}

int lhstable_setfield(LHSVM* vm, LHSTable* table)
{
    const LHSValue* key = lhsvm_getvalue(vm, -1),
        * value = lhsvm_getvalue(vm, -2);
    lhserr_check(vm, key->type != LHS_TNONE, "illegal table field.");

    long long h = lhstable_calc(key);
    LHSTableNode** list = 0, * node = 0;
    node = lhstable_search(table, key, h, &list);
    if (node)
    {
        memcpy(&node->value, value, sizeof(LHSValue));
        lhsvm_pop(vm, 2);
        return LHS_TRUE;
    }

    if (table->usize >= (table->size >> 1))
    {
        lhstable_grow(vm, table, table->size << 1);
        list = &table->nodes[lhstable_mod(h, table->size)];
    }

    node = (LHSTableNode*)lhsmem_newgcobject
    (
        vm, 
        sizeof(LHSTableNode), 
        LHS_TGCFULLDATA
    );
    node->hash = h;
    memcpy(&node->key, key, sizeof(LHSValue));
    memcpy(&node->value, value, sizeof(LHSValue));

    node->next = *list;
    *list = node;
    table->usize++;

    lhsvm_pop(vm, 2);
    return LHS_TRUE;
}

int lhstable_getfield(LHSVM* vm, LHSTable* table)
{
    LHSValue* key = lhsvalue_castvalue(lhsvm_getvalue(vm, -1));
    lhserr_check(vm, key->type != LHS_TNONE, "illegal table field.");

    long long h = lhstable_calc(key);
    LHSTableNode** list = 0, * node = 0;
    node = lhstable_search(table, key, h, &list);
    if (!node)
    {
        key->type = LHS_TNONE;
        return LHS_TRUE;
    }

    memcpy(key, &node->value, sizeof(LHSValue));
    return LHS_TRUE;
}

int lhstable_seti(LHSVM* vm, LHSTable* table, long long i)
{
    const LHSValue* value = lhsvm_getvalue(vm, -1);

    if (i < 0 || (size_t)i >= table->size)
    {
        lhsvm_pushinteger(vm, i);
        lhstable_setfield(vm, table);
        return LHS_TRUE;
    }

    LHSTableNode* node = table->nodes[i];
    if (node)
    {
        LHSValue key;
        key.type = LHS_TINTEGER;
        key.i = i;

        for (; node; node = node->next)
        {
            if (node->hash == i &&
                lhstable_equal(&node->key, &key))
            {
                memcpy(&node->value, value, sizeof(LHSValue));
                break;
            }
        }
    }

    if (!node)
    {
        node = (LHSTableNode*)lhsmem_newgcobject
        (
            vm, 
            sizeof(LHSTableNode), 
            LHS_TGCFULLDATA
        );
        node->hash = i;
        node->key.type = LHS_TINTEGER;
        node->key.i = i;
        memcpy(&node->value, value, sizeof(LHSValue));

        node->next = table->nodes[i];
        table->nodes[i] = node;
        table->usize++;
    }

    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

int lhstable_geti(LHSVM* vm, LHSTable* table, long long i)
{
    if (i < 0 || (size_t)i >= table->size)
    {
        lhsvm_pushinteger(vm, i);
        lhstable_getfield(vm, table);
        return LHS_TRUE;
    }

    LHSTableNode* node = table->nodes[i];
    if (!node)
    {
        lhsvm_pushnil(vm);
        return LHS_TRUE;
    }

    LHSValue key;
    key.type = LHS_TINTEGER;
    key.i = i;

    for (; node; node = node->next)
    {
        if (node->hash == i &&
            lhstable_equal(&node->key, &key))
        {
            break;
        }
    }

    if (node)
    {
        memcpy(lhsvm_incrementstack(vm), &node->value, sizeof(LHSValue));
    }
    else
    {
        lhsvm_pushnil(vm);
    }

    return LHS_TRUE;
}

int lhstable_remove(LHSVM* vm, LHSTable* table)
{
    const LHSValue* key = lhsvm_getvalue(vm, -1);
    lhserr_check(vm, key->type != LHS_TNONE, "illegal table field.");

    long long h = lhstable_calc(key);
    LHSTableNode** current = &table->nodes[lhstable_mod(h, table->size)];
    for (; *current; )
    {
        LHSTableNode* node = *current;
        if (node->hash == h &&
            lhstable_equal(&node->key, key))
        {
            *current = node->next;
            table->usize--;
            break;
        }
        else
        {
            current = &node->next;
        }
    }

    if (table->usize < (table->size >> 2))
    {
        lhstable_grow(vm, table, table->size >> 1);
    }

    lhsvm_pop(vm, 1);
    return LHS_TRUE;
}

void lhstable_foreach(LHSVM* vm, LHSTable* table, lhstable_iterator iterator,
    void* udata)
{
    for (size_t i = 0; i < table->size; ++i)
    {
        LHSTableNode* node = table->nodes[i];
        for (; node; node = node->next)
        {
            iterator(vm, table, &node->key, &node->value, udata);
        }
    }
}

long long lhstable_length(LHSVM* vm, LHSTable* table)
{
    long long i = 0;
    for (; i < (long long)table->size; ++i)
    {
        lhstable_geti(vm, table, i);
        if (lhsvm_getvalue(vm, -1)->type == LHS_TNONE)
        {
            lhsvm_pop(vm, 1);
            break;
        }

        lhsvm_pop(vm, 1);
    }

    return i;
}

int lhstable_uninit(LHSVM* vm, LHSTable* table)
{
    lhsmem_freeobject(vm, table->nodes, sizeof(LHSTableNode*) * table->size);
    table->size = 0;
    table->usize = 0;
    return LHS_TRUE;
}
