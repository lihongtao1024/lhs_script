#pragma once
#include "lhs_config.h"

#define lhsbuf_pushs(vm, b, s)                      \
lhsbuf_pushls(vm, b, s, strlen(s))

typedef struct LHSSTRBUF
{
    size_t usize;
    size_t size;
    char* data;
    char buf[128];
} LHSSTRBUF;

int lhsbuf_init(void* vm, LHSSTRBUF* buf);

int lhsbuf_reset(void* vm, LHSSTRBUF* buf);

int lhsbuf_pushc(void* vm, LHSSTRBUF* buf, char c);

int lhsbuf_pushi(void* vm, LHSSTRBUF* buf, int i);

int lhsbuf_pushl(void* vm, LHSSTRBUF* buf, long long i);

int lhsbuf_pushf(void* vm, LHSSTRBUF* buf, double n);

int lhsbuf_pushls(void* vm, LHSSTRBUF* buf, const char* str,
    size_t l);

int lhsbuf_topchar(void* vm, LHSSTRBUF* buf, char* c);

int lhsbuf_popchar(void* vm, LHSSTRBUF* buf, char *c);

int lhsbuf_popi(void* vm, LHSSTRBUF* buf, int i);

int lhsbuf_popl(void* vm, LHSSTRBUF* buf, long long i);

int lhsbuf_popf(void* vm, LHSSTRBUF* buf, double n);

int lhsbuf_isshort(void* vm, LHSSTRBUF* buf);

int lhsbuf_isempty(void* vm, LHSSTRBUF* buf);

void lhsbuf_uninit(void* vm, LHSSTRBUF* buf);
