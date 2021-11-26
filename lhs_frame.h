#pragma once
#include "lhs_config.h"
#include "lhs_vector.h"
#include "lhs_value.h"
#include "lhs_variable.h"
#include "lhs_debug.h"
#include "lhs_gc.h"
#include "lhs_vm.h"

#define LHS_FRAMENAME 0
#define lhsframe_castmainframe(vm)  ((LHSFrame*)(vm)->mainframe)
#define lhsframe_castcurframe(vm)   ((LHSFrame*)(vm)->currentframe)

typedef struct LHSFrame
{
    LHSGCObject gc;         /*garbage collection handle*/
    size_t level;           /*valid scope*/
    LHSVariables variables; /*hash table for variables*/     
    LHSVector values;       /*values for variables*/
    LHSVector codes;        /*execute codes*/
    LHSDebug debug;         /*debug info for frame*/
    LHSVM* vm;              /*virtual machine*/
    size_t base;            /*base of frame stack*/
    size_t ret;             /*return of frame stack*/
    struct LHSFrame* next;  /*sub frames*/
} LHSFrame;

int lhsframe_init(LHSVM* vm, LHSFrame* frame, size_t level);

int lhsframe_insertvariable(LHSVM* vm, LHSFrame* frame, 
    long long line, long long column);

int lhsframe_getvariable(LHSVM* vm, LHSFrame* frame);

const char* lhsframe_name(LHSVM* vm, LHSFrame* frame);

void lhsframe_uninit(LHSVM* vm, LHSFrame* frame);
