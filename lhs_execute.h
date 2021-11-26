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

int lhsexecute_protectedrun(void* vm, protectedf fn, void* userdata);

int lhsexecute_protectederr(void* vm, const char* fmt, ...);
