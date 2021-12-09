#pragma once
#include "lhs_config.h"

typedef int (*lhshash_equal)(void*, void*);
typedef long long (*lhshash_calc)(void*);

typedef struct LHSHashNode
{
    struct LHSHashNode* next;
    long long hash;
    void* data;
} LHSHashNode;

typedef struct LHSHashTable
{
    LHSHashNode** nodes;
    lhshash_calc calc;
    lhshash_equal equal;
    size_t size;
    size_t usize;    
} LHSHashTable;

int lhshash_init(void* vm, LHSHashTable* hash, lhshash_calc calc, 
    lhshash_equal comp, size_t n);

int lhshash_insert(void* vm, LHSHashTable* hash, void* userdata, 
    long long* ohash);

void* lhshash_find(void* vm, LHSHashTable* hash, void* userdata);

void lhshash_remove(void* vm, LHSHashTable* hash, void* userdata);

void lhshash_uninit(void* vm, LHSHashTable* hash);
