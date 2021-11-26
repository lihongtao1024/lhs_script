#pragma once
#include "lhs_config.h"

#define LHS_TGCVM       1
#define LHS_TGCFRAME    2
#define LHS_TGCSTRING   3 
#define LHS_TGCFULLDATA 4

#define lhsgc_castgc(o) ((LHSGCObject*)(o))

extern const char* lhsgc_type[];

typedef struct LHSGCObject
{
    int type;
    int marked;
    struct LHSGCObject* next;
} LHSGCObject;
