#pragma once
#include "lhs_config.h"
#include "lhs_gc.h"

#define lhsmem_initgc(p, t, l)                                  \
(p)->type = (t);(p)->marked = 0;(p)->size = l;(p)->next = 0

typedef void* (*lhsmem_new)(void*, void*, size_t, size_t);

void* lhsmem_default(void* vm, void* original, size_t osize, 
    size_t nsize);

void* lhsmem_newobject(void* vm, size_t size);

void* lhsmem_renewobject(void* vm, void* original, size_t osize, size_t nsize);

void lhsmem_freeobject(void* vm, void* data, size_t size);

LHSGCObject* lhsmem_newgcobject(void* vm, size_t size, int type);
