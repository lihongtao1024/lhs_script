#pragma once
#include "lhs_config.h"

#define LHS_TGCVM       (1)
#define LHS_TGCFRAME    (2)
#define LHS_TGCSTRING   (3)
#define LHS_TGCFUNCTION (4)
#define LHS_TGCFULLDATA (5)
#define lhsgc_castgc(o) ((LHSGCObject*)(o))

typedef struct LHSGCObject
{
    char type;
    int marked;
    size_t size;
    struct LHSGCObject* next;
} LHSGCObject;
