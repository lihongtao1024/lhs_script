#pragma once
#include "lhs_config.h"
#include "lhs_vector.h"
#include "lhs_value.h"
#include "lhs_variable.h"
#include "lhs_gc.h"
#include "lhs_vm.h"

#define lhsframe_castmainframe(vm)         ((LHSFunction*)(vm)->mainframe)
#define lhsframe_castcurframe(vm)          ((LHSFunction*)(vm)->currentframe)
#define lhsframe_castframe(o)              ((LHSFunction*)(o))

typedef struct LHSFunction
{
    LHSGCObject gc;         /*garbage collection handle*/
    int name;               /*function name index in values*/
    int narg;
    int nret;
    int nlocalvars;         /*number of local variables*/
    size_t entry;           /*ip entry*/
    LHSBuf code;            /*executable byte codes*/
    LHSHash localvars;      /*hash table for local variables*/
} LHSFunction;

int lhsfunction_init(LHSVM* vm, LHSFunction* function);

const char* lhsfunction_getname(LHSVM* vm, LHSFunction* function);

void lhsfunction_uninit(LHSVM* vm, LHSFunction* function);
