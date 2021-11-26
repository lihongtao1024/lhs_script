#pragma once
#include "lhs_config.h"
#include "lhs_gc.h"

/*memory allocation function declaration*/
typedef void* (*lhsmem_new)(void*, void*, size_t, size_t);

#define lhsmem_newobject(vm, n)         (vm)->falloc((vm), 0, 0, (n))
#define lhsmem_renewobject(vm, p, o, n) (vm)->falloc((vm), (p), (o), (n))
#define lhsmem_freeobject(vm, p, l)     (vm)->falloc((vm), p, l, 0)
#define lhsmem_initgc(p, t)             (p)->type = (t); p->marked = 0; p->next = 0
#define lhsmem_newgcstring(vm, p, e)    lhsmem_newgclstring(vm, p, strlen(p), e)

void* lhsmem_realloc(void* vm, void* original, size_t osize, 
    size_t nsize);

LHSGCObject* lhsmem_newgcobject(void* vm, size_t size, int type);

LHSGCObject* lhsmem_newgclstring(void* vm, void* data, size_t l, int extra);
