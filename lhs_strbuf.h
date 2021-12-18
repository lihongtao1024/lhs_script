#pragma once
#include "lhs_config.h"

#define lhsbuf_pushs(vm, b, s)                      \
lhsbuf_pushls(vm, b, s, strlen(s))

typedef struct LHSSTRBUF
{
    size_t usize;
    size_t size;
    char* data;
    char buf[32];
} LHSSTRBUF;

int lhsbuf_init(void* vm, LHSSTRBUF* buf);

int lhsbuf_reset(void* vm, LHSSTRBUF* buf);

int lhsbuf_pushc(void* vm, LHSSTRBUF* buf, char c);

int lhsbuf_pushi(void* vm, LHSSTRBUF* buf, int i);

int lhsbuf_pushl(void* vm, LHSSTRBUF* buf, long long i);

int lhsbuf_pushf(void* vm, LHSSTRBUF* buf, double n);

int lhsbuf_pushls(void* vm, LHSSTRBUF* buf, const char* str,
    size_t l);

void lhsbuf_uninit(void* vm, LHSSTRBUF* buf);
