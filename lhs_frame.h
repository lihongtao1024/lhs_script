#pragma once
#include "lhs_config.h"
#include "lhs_vector.h"
#include "lhs_value.h"
#include "lhs_variable.h"
#include "lhs_debug.h"
#include "lhs_gc.h"
#include "lhs_vm.h"

#define lhsframe_castmainframe(vm)         ((LHSFrame*)(vm)->mainframe)
#define lhsframe_castcurframe(vm)          ((LHSFrame*)(vm)->currentframe)
#define lhsframe_castframe(o)              ((LHSFrame*)(o))

#define lhsframe_insertvar(vm, f, l, c)    \
(lhsframe_insertvariable((vm), (f), (l), (c), LHS_FALSE))        

#define lhsframe_insertglobal(vm, f, l, c) \
(lhsframe_insertvariable((vm), (f), (l), (c), LHS_TRUE))

#define lhsframe_insertparam(vm, f, l, c)  \
lhsframe_insertvar(vm, f, l, c)

typedef struct LHSChunk
{
    int index;              /*chunk index*/
    struct LHSChunk* parent;
} LHSChunk;

typedef struct LHSFrame
{
    LHSGCObject gc;         /*garbage collection handle*/
    struct LHSFrame* parent;
    LHSVariables variables; /*hash table for variables*/     
    LHSVector values;       /*values for variables*/
    LHSDebug debugs;        /*debug info for variables*/
    LHSVector allchunks;    /*chain chunk*/
    LHSChunk* curchunk;     /*current chunk*/
    int name;               /*function name index in values*/
    int nret;
} LHSFrame;

int lhsframe_init(LHSVM* vm, LHSFrame* frame);

int lhsframe_enterchunk(LHSVM* vm, LHSFrame* frame, void* loadf);

int lhsframe_leavechunk(LHSVM* vm, LHSFrame* frame, void* loadf);

LHSVariable* lhsframe_insertconstant(LHSVM* vm);

LHSVariable* lhsframe_insertvariable(LHSVM* vm, LHSFrame* frame, 
    long long line, long long column, int global);

LHSVariable* lhsframe_getvariable(LHSVM* vm, LHSFrame* frame);

const char* lhsframe_name(LHSVM* vm, LHSFrame* frame);

void lhsframe_uninit(LHSVM* vm, LHSFrame* frame);
