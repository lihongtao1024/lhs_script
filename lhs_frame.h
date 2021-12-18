#pragma once
#include "lhs_config.h"
#include "lhs_vector.h"
#include "lhs_value.h"
#include "lhs_variable.h"
#include "lhs_gc.h"
#include "lhs_vm.h"

#define lhsframe_castmainframe(vm)         ((LHSFrame*)(vm)->mainframe)
#define lhsframe_castcurframe(vm)          ((LHSFrame*)(vm)->currentframe)
#define lhsframe_castframe(o)              ((LHSFrame*)(o))

typedef struct LHSFrame
{
    LHSGCObject gc;         /*garbage collection handle*/
    int name;               /*function name index in values*/
    int narg;
    int nret;
    size_t entry;           /*ip entry*/
    LHSHashTable localvars; /*hash table for local variables*/     
    LHSVector localvalues;  /*value array for local variables*/
} LHSFrame;

int lhsframe_init(LHSVM* vm, LHSFrame* frame);

const char* lhsframe_getname(LHSVM* vm, LHSFrame* frame);

int lhsframe_setframe(LHSVM* vm, LHSFrame* frame);

void lhsframe_uninit(LHSVM* vm, LHSFrame* frame);
