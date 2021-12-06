#pragma once
#include "lhs_config.h"
#include "lhs_alloc.h"
#include "lhs_gc.h"
#include "lhs_value.h"
#include "lhs_hash.h"
#include "lhs_vector.h"
#include "lhs_variable.h"
#include "lhs_strbuf.h"
#include "lhs_error.h"

#define lhsvm_castvm(vm)        ((LHSVM*)(vm))
#define lhsvm_pushstring(vm, s) lhsvm_pushlstring(vm, s, strlen(s))

typedef struct LHSVM
{
    LHSGCObject gc;             /*garbage collect handle*/
    lhsmem_new falloc;          /*memory handler*/
    void* mainframe;            /*main function frame*/
    void* currentframe;         /*current function frame*/    
    LHSError* errorjmp;           /*error jump handler*/
    LHSHashTable shortstrhash;  /*hast table for short string*/
    LHSVariables conststrhash;  /*hash table for constant string*/
    LHSVector conststrvalue;    /*constant strings*/
    LHSVector stack;            /*execute stack*/
    LHSSTRBUF code;             /*executable byte code*/
    LHSGCObject* allgc;         /*all garbage collection*/
    size_t nalloc;              /*allocated memory size*/
} LHSVM;

LHSVM* lhsvm_create(lhsmem_new fn);

int lhsvm_pushnil(LHSVM* vm);

int lhsvm_pushboolean(LHSVM* vm, char b);

int lhsvm_pushvalue(LHSVM* vm, int index);

int lhsvm_pushlstring(LHSVM* vm, const char* str, size_t l);

int lhsvm_pushnumber(LHSVM* vm, double number);

int lhsvm_pushinteger(LHSVM* vm, long long number);

LHSValue* lhsvm_getvalue(LHSVM* vm, int index);

const char* lhsvm_tostring(LHSVM* vm, int index);

LHSString* lhsvm_findshort(LHSVM* vm, void* data, size_t l);

LHSValue* lhsvm_top(LHSVM* vm);

size_t lhsvm_gettop(LHSVM* vm);

int lhsvm_pop(LHSVM* vm, size_t n);

LHSVariable* lhsvm_insertconstant(LHSVM* vm);

void lhsvm_destroy(LHSVM* vm);
