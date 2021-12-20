#include "lhs_function.h"

static int lhsfunction_initlocalvar(LHSVM* vm, LHSHash* hash, LHSVarDesc* desc, 
    LHSFunction* function)
{
    lhserr_check
    (
        vm,
        desc->mark == LHS_MARKLOCAL &&
        desc->index < function->localvalues.usize,
        "system error."
    );

    LHSVar* localvar = lhsvector_at(vm, &function->localvalues, 
        desc->index);
    localvar->desc = desc;
    localvar->value.type = LHS_TNONE;
    return LHS_TRUE;
}

int lhsfunction_init(LHSVM* vm, LHSFunction* function, LHSFrame* frame)
{
    function->frame = frame;

    lhsvector_init
    (
        vm, 
        &function->localvalues, 
        sizeof(LHSVar), 
        frame->localvalues.usize
    );
    function->localvalues.usize = frame->localvalues.usize;

    lhshash_foreach(vm, &frame->localvars, lhsfunction_initlocalvar, function);
    return LHS_TRUE;
}

int lhsfunction_uninit(LHSVM* vm, LHSFunction* function)
{
    lhsvector_uninit(vm, &function->localvalues);
    return LHS_TRUE;
}
