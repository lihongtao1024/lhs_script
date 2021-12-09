#pragma once
#include "lhs_config.h"
#include "lhs_vector.h"
#include "lhs_value.h"
#include "lhs_variable.h"
#include "lhs_debug.h"
#include "lhs_gc.h"
#include "lhs_vm.h"

#define LHS_FRAMENAME               0
#define lhsframe_castmainframe(vm)  ((LHSFrame*)(vm)->mainframe)
#define lhsframe_castcurframe(vm)   ((LHSFrame*)(vm)->currentframe)
#define lhsframe_castframe(o)       ((LHSFrame*)(o))

typedef struct LHSChunk
{
    int index;              /*chunk index*/
    struct LHSChunk* parent;
    struct LHSChunk* chain;
} LHSChunk;

typedef struct LHSFrame
{
    LHSGCObject gc;         /*garbage collection handle*/
    LHSChunk* curchunk;     /*current chunk*/
    LHSChunk* allchunks;     /*chain chunk*/
    LHSVariables variables; /*hash table for variables*/     
    LHSVector values;       /*values for variables*/
    LHSDebug debug;         /*debug info for variables*/
    int nchunk;
} LHSFrame;

int lhsframe_init(LHSVM* vm, LHSFrame* frame);

int lhsframe_enterchunk(LHSVM* vm, LHSFrame* frame, void* loadf);

int lhsframe_leavechunk(LHSVM* vm, LHSFrame* frame, void* loadf);

LHSVariable* lhsframe_insertvariable(LHSVM* vm, LHSFrame* frame, 
    long long line, long long column, int global);

LHSVariable* lhsframe_getvariable(LHSVM* vm, LHSFrame* frame);

const char* lhsframe_name(LHSVM* vm, LHSFrame* frame);

void lhsframe_uninit(LHSVM* vm, LHSFrame* frame);
