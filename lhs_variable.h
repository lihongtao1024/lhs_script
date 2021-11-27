#pragma once
#include "lhs_config.h"
#include "lhs_value.h"
#include "lhs_hash.h"
#include "lhs_gc.h"

#define LHS_MARKNONE            (0)
#define LHS_MARKLOCAL           (1)
#define LHS_MARKGLOBAL          (2)
#define lhsvariable_castvar(ud) ((LHSVariable*)(ud))

typedef struct LHSVariable
{
    LHSGCObject gc;
    int index;
    int marked;    
    LHSString *name;
} LHSVariable;

typedef LHSHashTable LHSVariables;

long long lhsvariable_hashvar(void* data);

int lhsvariable_equalvar(void* data1, void* data2);
