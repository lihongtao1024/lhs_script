#pragma once
#include "lhs_config.h"
#include "lhs_value.h"
#include "lhs_vm.h"

#define lhstable_casttable(o) ((LHSTable*)(o))

typedef struct LHSTableNode
{
    LHSGC gc;
    long long hash;
    LHSValue key;
    LHSValue value;
    struct LHSTableNode* next;
} LHSTableNode;

typedef struct LHSTable
{
    LHSGC gc;
    size_t size;
    size_t usize;
    LHSTableNode** nodes;
} LHSTable;

typedef int (*lhstable_iterator)(LHSVM*, const LHSTable*, const LHSValue*, 
    LHSValue*, void* userdata);

int lhstable_init(LHSVM* vm, LHSTable* table);

int lhstable_setfield(LHSVM* vm, LHSTable* table);

int lhstable_getfield(LHSVM* vm, LHSTable* table);

int lhstable_seti(LHSVM* vm, LHSTable* table, long long i);

int lhstable_geti(LHSVM* vm, LHSTable* table, long long i);

int lhstable_remove(LHSVM* vm, LHSTable* table);

void lhstable_foreach(LHSVM* vm, LHSTable* table, lhstable_iterator iterator,
    void* udata);

int lhstable_uninit(LHSVM* vm, LHSTable* table);
