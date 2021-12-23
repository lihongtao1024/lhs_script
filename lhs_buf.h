#pragma once
#include "lhs_config.h"

#define lhsbuf_pushs(vm, b, s)                      \
lhsbuf_pushls(vm, b, s, strlen(s))

typedef struct LHSBuf
{
    size_t usize;
    size_t size;
    char* data;
    char buf[32];
} LHSBuf;

int lhsbuf_init(void* vm, LHSBuf* buf);

int lhsbuf_reset(void* vm, LHSBuf* buf);

int lhsbuf_pushc(void* vm, LHSBuf* buf, char c);

int lhsbuf_pushi(void* vm, LHSBuf* buf, int i);

int lhsbuf_pushl(void* vm, LHSBuf* buf, long long i);

int lhsbuf_pushf(void* vm, LHSBuf* buf, double n);

int lhsbuf_pushls(void* vm, LHSBuf* buf, const char* str,
    size_t l);

void lhsbuf_uninit(void* vm, LHSBuf* buf);
