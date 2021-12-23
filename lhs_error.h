#pragma once
#include "lhs_config.h"

/*error handler*/
#define lhserr_no(fmt, ...)                                         \
printf(fmt##", description: %s.\n", __VA_ARGS__, strerror(errno))

#define lhserr_msg(s)                                               \
printf("%s\n", s)

#define lhserr_syntax(vm, lf, fmt, ...)                             \
lhserr_throw                                                        \
(                                                                   \
    vm,                                                             \
    "syntax error at: [%s:%d:%d], "##fmt,                           \
    lhsfunction_getname(vm, lhsparser_castlex(lf)->curfunction),    \
    (lf)->line,                                                     \
    (lf)->column,                                                   \
    __VA_ARGS__                                                     \
)

#define lhserr_check(vm, exp, fmt, ...)                             \
(exp) || lhserr_throw                                               \
(                                                                   \
    vm,                                                             \
    "check '%s', runtime error: "##fmt,                             \
    #exp,                                                           \
    __VA_ARGS__                                                     \
);

typedef void (*protectedf)(void*, void*);
typedef void (*protectedfex)(void*, void*, void*);

typedef struct LHSError
{
    int errcode;
    jmp_buf buf;
    struct LHSError* prev;
} LHSError;

int lhserr_protectedcall(void* vm, protectedf fn, void* udata);

int lhserr_protectedcallex(void* vm, protectedfex fn, void* ud1, void* ud2);

int lhserr_throw(void* vm, const char* fmt, ...);

int lhserr_runtime(void* vm, const void* desc, const char* fmt, ...);
