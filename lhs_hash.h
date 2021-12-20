#pragma once
#include "lhs_config.h"

typedef int (*lhshash_equal)(void*, void*);
typedef long long (*lhshash_calc)(void*);
typedef int (*lhshash_iterator)(void*, void*, void*, void*);

typedef struct LHSHashNode
{
    long long hash;
    void* data;
    struct LHSHashNode* next;
} LHSHashNode;

typedef struct LHSHash
{
    size_t size;
    size_t usize;
    lhshash_calc calc;
    lhshash_equal equal;
    LHSHashNode** nodes;
} LHSHash;

int lhshash_init(void* vm, LHSHash* hash, lhshash_calc calc, 
    lhshash_equal comp, size_t n);

int lhshash_insert(void* vm, LHSHash* hash, void* userdata, 
    long long* ohash);

void* lhshash_find(void* vm, LHSHash* hash, void* userdata);

void lhshash_remove(void* vm, LHSHash* hash, void* userdata);

void lhshash_foreach(void* vm, LHSHash* hash, lhshash_iterator iterator, 
    void* udata);

void lhshash_uninit(void* vm, LHSHash* hash);
