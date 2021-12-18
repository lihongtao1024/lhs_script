#pragma once
#include "lhs_config.h"
#include "lhs_value.h"
#include "lhs_hash.h"
#include "lhs_gc.h"

#define LHS_MARKNONE              (0)
#define LHS_MARKLOCAL             (1)
#define LHS_MARKGLOBAL            (2)
#define LHS_MARKINTEGER           (3)
#define LHS_MARKNUMBER            (4)
#define LHS_MARKBOOLEAN           (5)
#define LHS_MARKSTRING            (6)
#define LHS_MARKSTACK             (7)
#define LHS_MARKMAX               (8)
#define lhsvar_castvardesc(ud)    ((LHSVarDesc*)(ud))

typedef struct LHSVarDesc
{
    LHSGCObject gc;
    char mark;
    int index;
    int chunk;
    int line;
    int column;
    LHSString *name;
} LHSVarDesc;

typedef struct LHSVar
{
    LHSValue value;
    const LHSVarDesc* desc;
} LHSVar;

long long lhsvar_hashvar(void* data);

int lhsvar_equalvar(void* data1, void* data2);
