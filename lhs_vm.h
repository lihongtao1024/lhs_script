#pragma once
#include "lhs_config.h"
#include "lhs_alloc.h"
#include "lhs_gc.h"
#include "lhs_hash.h"
#include "lhs_vector.h"
#include "lhs_value.h"
#include "lhs_buf.h"
#include "lhs_error.h"

#define lhsvm_castvm(vm)        ((LHSVM*)(vm))
#define lhsvm_pushstring(vm, s) lhsvm_pushlstring(vm, s, strlen(s))

typedef size_t StkID;
typedef struct LHSVM
{
    LHSGC gc;                   /*garbage collect handle*/
    StkID top;                  /*position of runtime stack top*/
    size_t nalloc;              /*allocated memory size*/
    size_t ncallcontext;        /*for call layers*/
    LHSHash shortstrhash;       /*hast table for short string*/
    LHSHash conststrhash;       /*hash table for constant string*/
    LHSHash globalvars;         /*hash table for global variable*/
    LHSVector conststrs;        /*constant string array*/
    LHSVector globalvalues;     /*value array for global variable*/
    LHSVector stack;            /*execute stack*/
    void* mainframe;            /*main function frame*/
    void* callcontext;          /*runtime context*/
    LHSError* errorjmp;         /*error jump handler*/
    lhsmem_new alloc;           /*memory handler*/
    LHSGC* allgc;               /*all garbage collection*/
} LHSVM;

LHSVM* lhsvm_create(lhsmem_new fn);

int lhsvm_dofile(LHSVM* vm, const char* name);

int lhsvm_pushnil(LHSVM* vm);

int lhsvm_pushboolean(LHSVM* vm, char b);

int lhsvm_pushvalue(LHSVM* vm, int index);

int lhsvm_pushlstring(LHSVM* vm, const char* str, size_t l);

int lhsvm_pushnumber(LHSVM* vm, double number);

int lhsvm_pushinteger(LHSVM* vm, long long number);

int lhsvm_pushdelegate(LHSVM* vm, lhsvm_delegate delegate);

int lhsvm_setglobal(LHSVM* vm, const char* name);

const LHSValue* lhsvm_getvalue(LHSVM* vm, int index);

int lhsvm_gettop(LHSVM* vm);

int lhsvm_pop(LHSVM* vm, size_t n);

LHSValue* lhsvm_incrementstack(LHSVM* vm);

const char* lhsvm_tostring(LHSVM* vm, int index);

double lhsvm_tonumber(LHSVM* vm, int index);

long long lhsvm_tointeger(LHSVM* vm, int inex);

void lhsvm_destroy(LHSVM* vm);

const LHSString* lhsvm_insertshortstr(LHSVM* vm, const char* str,
    size_t l, int reserved);