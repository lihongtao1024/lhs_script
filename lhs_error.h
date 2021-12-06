#pragma once
#include "lhs_config.h"

/*error handler*/
#define lhserr_no(fmt, ...)                                         \
printf(fmt##", description: %s.\n", __VA_ARGS__, strerror(errno))

#define lhserr_msg(s)                                               \
printf("%s\n", s)

#define lhserr_syntaxerr(vm, lf, fmt, ...)                          \
lhserr_throw                                                        \
(                                                                   \
    vm,                                                             \
    "syntax error at:[%s:%lld:%lld], "##fmt,                        \
    lhsframe_name(vm, lhsframe_castmainframe(vm)),                  \
    (lf)->line,                                                     \
    (lf)->column,                                                   \
    __VA_ARGS__                                                     \
)

typedef void (*protectedf)(void*, void*);
typedef struct LHSError
{
    struct LHSError* prev;
    jmp_buf buf;
    int errcode;
} LHSError;

int lhserr_protectedcall(void* vm, protectedf fn, void* udata);

int lhserr_throw(void* vm, const char* fmt, ...);
