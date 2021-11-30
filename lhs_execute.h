#pragma once
#include "lhs_config.h"

typedef void (*protectedf)(void*, void*);

typedef struct LHSJmp
{
    struct LHSJmp* prev;
    jmp_buf buf;
    int errcode;
} LHSJmp;

#define lhsexecute_symbolerr(vm, fmt, ...) \
lhsexecute_protectederr(vm, "syntax error at:[%s:%lld:%lld], desc:"##fmt, __VA_ARGS__)

#define lhsexecute_typeerr(vm, ud, t) \
if ((ud)->type != (t)) \
{\
    lhsexecute_protectederr \
    ( \
        vm, \
        "the type should be:%d, but it is:%d, expr:%s", \
        t, \
        (ud)->type, \
        #ud \
    ); \
}

int lhsexecute_protectedrun(void* vm, protectedf fn, void* userdata);

int lhsexecute_protectederr(void* vm, const char* fmt, ...);
