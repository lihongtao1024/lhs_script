#pragma once
#include "lhs_config.h"
#include "lhs_gc.h"

#define LHS_TNONE        (0)
#define LHS_TINTEGER     (1)
#define LHS_TDOUBLE      (2)
#define LHS_TGC          (3)
#define LHS_TBOOLEAN     (4)
#define LHS_SHORTSTRLEN  (64)

#define lhsvalue_castvalue(ud)  ((LHSValue*)(ud))
#define lhsvalue_caststring(ud) ((LHSString*)lhsgc_castgc((ud)))
#define lhsvalue_caststack(ud)  ((LHSStack)(ud))
#define lhsvalue_isshortstr(s)  ((s)->length < LHS_SHORTSTRLEN)

typedef struct LHSString
{
    LHSGCObject gc;
    long long hash;
    size_t length;
    int extra;
    char data[0];
} LHSString;

typedef struct LHSShortString
{
    LHSGCObject gc;
    long long hash;
    size_t length;
    int extra;
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
    };
} LHSValue;

typedef LHSValue* LHSStack;

long long lhsvalue_hashformula(const char* str, size_t l, size_t seed);

long long lhsvalue_hashstring(void* data);

int lhsvalue_equalstring(void* data1, void* data2);
