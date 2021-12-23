#pragma once
#include "lhs_config.h"
#include "lhs_function.h"
#include "lhs_gc.h"
#include "lhs_vm.h"

#define lhsframe_castframe(o)           ((LHSFrame*)(o))

typedef struct LHSFrame
{
    LHSGCObject gc;
    int name;
    LHSFunction* mainfunc;
    LHSFunction* allfunc;
    struct LHSFrame* next;
} LHSFrame;

int lhsframe_init(LHSVM* vm, LHSFrame* frame);

int lhsframe_uninit(LHSVM* vm, LHSFrame* frame);
