#pragma once
#include "lhs_config.h"
#include "lhs_vector.h"
#include "lhs_frame.h"
#include "lhs_vm.h"

typedef struct LHSFunction
{
    LHSFrame* frame;        /*attach to frame*/
    LHSVector localvalues;  /*value array for local variables*/
} LHSFunction;

int lhsfunction_init(LHSVM* vm, LHSFunction* function, 
    LHSFrame* frame);

int lhsfunction_uninit(LHSVM* vm, LHSFunction* function);
