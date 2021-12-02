#pragma once
#include "lhs_config.h"
#include "lhs_vm.h"

#define lhsbuf_pushstr(vm, b, s)                                        \
lhsbuf_pushlstr(vm, b, s, strlen(s))

typedef struct LHSSTRBUF
{
    size_t usize;
    size_t size;
    char* data;
    char buf[128];
} LHSSTRBUF;

int lhsbuf_init(LHSVM* vm, LHSSTRBUF* buf);

int lhsbuf_reset(LHSVM* vm, LHSSTRBUF* buf);

int lhsbuf_pushchar(LHSVM* vm, LHSSTRBUF* buf, char c);

int lhsbuf_pushlstr(LHSVM* vm, LHSSTRBUF* buf, const char* str, size_t l);

int lhsbuf_topchar(LHSVM* vm, LHSSTRBUF* buf, char* c);

int lhsbuf_popchar(LHSVM* vm, LHSSTRBUF* buf, char *c);

int lhsbuf_isshort(LHSVM* vm, LHSSTRBUF* buf);

int lhsbuf_isempty(LHSVM* vm, LHSSTRBUF* buf);

void lhsbuf_uninit(LHSVM* vm, LHSSTRBUF* buf);
