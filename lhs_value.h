#pragma once
#include "lhs_config.h"
#include "lhs_gc.h"

#define LHS_TNONE               (0)
#define LHS_TINTEGER            (1)
#define LHS_TNUMBER             (2)
#define LHS_TBOOLEAN            (3)
#define LHS_TDELEGATE           (4)
#define LHS_TGC                 (5)
#define LHS_SHORTSTRLEN         (48)
#define lhsvalue_castvalue(ud)  ((LHSValue*)(ud))
#define lhsvalue_caststring(ud) ((LHSString*)(ud))

typedef struct LHSString
{
    LHSGCObject gc;
    long long hash;
    int length;
    int reserved;
    char data[0];
} LHSString;

typedef struct LHSShortString
{
    LHSGCObject gc;
    long long hash;
    int length;
    int reserved;
    char data[LHS_SHORTSTRLEN];
} LHSShortString;

typedef struct LHSValue
{
    int type;
    union
    {
        char b;
        long long i;
        double n;
        LHSGCObject* gc;
        lhsvm_delegate dg;
    };
} LHSValue;

extern const char* lhsvalue_typename[];

long long lhsvalue_hashformula(const char* str, size_t l, size_t seed);

long long lhsvalue_hashstr(void* data);

int lhsvalue_equalstr(void* data1, void* data2);
