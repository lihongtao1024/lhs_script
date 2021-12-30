#pragma once
#include "lhs_config.h"
#include "lhs_hash.h"
#include "lhs_gc.h"
#include "lhs_vm.h"

#define lhsfunction_castfunc(o)              ((LHSFunction*)(o))

typedef struct LHSFunction
{
    LHSGC gc;               /*garbage collection handle*/
    int name;               /*function name index in values*/
    int narg;
    int nret;
    int nlocalvars;         /*number of local variables*/
    LHSBuf code;            /*executable byte codes*/
    LHSHash localvars;      /*hash table for local variables*/
    void* frame;
    struct LHSFunction* next;
} LHSFunction;

int lhsfunction_init(LHSVM* vm, LHSFunction* function);

const char* lhsfunction_getname(LHSVM* vm, LHSFunction* function);

void lhsfunction_uninit(LHSVM* vm, LHSFunction* function);
