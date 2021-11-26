#pragma once
#include "lhs_config.h"
#include "lhs_alloc.h"
#include "lhs_gc.h"
#include "lhs_value.h"
#include "lhs_hash.h"
#include "lhs_vector.h"
#include "lhs_execute.h"

#define lhsvm_castvm(vm)        ((LHSVM*)(vm))
#define lhsvm_pushstring(vm, s) lhsvm_pushlstring(vm, s, strlen(s))

typedef struct LHSVM
{
    LHSGCObject gc;         /*garbage collect handle*/
    long long rax;          /*rax register simulating for win64*/
    long long rip;          /*rip register simulating for win64*/    
    long long rbp;          /*rbp register simulating for win64*/
    long long rsp;          /*rsp register simulating for win64*/
    lhsmem_new falloc;      /*memory handler*/
    void* mainframe;        /*main function frame*/
    void* currentframe;     /*current function frame*/
    LHSGCObject* allgc;     /*all garbage collection*/
    LHSJmp* errorjmp;       /*error jump handler*/
    LHSHashTable conststr;  /*constant table for string*/
    LHSVector stack;        /*execute stack*/
} LHSVM;

LHSVM* lhsvm_create(lhsmem_new fn);

int lhsvm_pushnil(LHSVM* vm);

int lhsvm_pushboolean(LHSVM* vm, char b);

int lhsvm_pushvalue(LHSVM* vm, LHSValue* value);

int lhsvm_pushlstring(LHSVM* vm, const char* str, size_t l);

int lhsvm_pushnumber(LHSVM* vm, double number);

int lhsvm_pushinteger(LHSVM* vm, long long number);

LHSValue* lhsvm_getvalue(LHSVM* vm, int index);

const char* lhsvm_tostring(LHSVM* vm, int index);

LHSString* lhsvm_findshort(LHSVM* vm, void* data, size_t l);

LHSValue* lhsvm_top(LHSVM* vm);

size_t lhsvm_gettop(LHSVM* vm);

int lhsvm_pop(LHSVM* vm, size_t n);

int lhsvm_insertvariable(LHSVM* vm);

void lhsvm_destroy(LHSVM* vm);
