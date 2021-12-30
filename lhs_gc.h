#pragma once
#include "lhs_config.h"

#define LHS_TGCVM       (1)
#define LHS_TGCFRAME    (2)
#define LHS_TGCSTRING   (3)
#define LHS_TGCFUNCTION (4)
#define LHS_TGCFULLDATA (5)
#define LHS_TGCTABLE    (6)
#define lhsgc_castgc(o) ((LHSGC*)(o))

typedef struct LHSGC
{
    char type;
    int marked;
    size_t size;
    struct LHSGC* next;
} LHSGC;
