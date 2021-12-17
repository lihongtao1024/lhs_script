#include "lhs_frame.h"
#include "lhs_link.h"
#include "lhs_load.h"
#include "lhs_code.h"

int lhsframe_init(LHSVM* vm, LHSFrame* frame)
{
    lhshash_init(vm, &frame->localvars, lhsvar_hashvar, lhsvar_equalvar, 0);
    lhsvector_init(vm, &frame->localvalues, sizeof(LHSVar), 0);
    frame->parent = 0;
    frame->entry = vm->code.usize;
    frame->name = -1;
    frame->narg = 0;
    frame->nret = LHS_UNCERTAIN;
    return LHS_TRUE;
}

const char* lhsframe_name(LHSVM* vm, LHSFrame* frame)
{
    LHSVar* var = lhsvector_at(vm, &frame->localvalues, frame->name);
    return lhsvalue_caststring(var->value.gc)->data;
}

int lhsframe_setframe(LHSVM* vm, LHSFrame* frame)
{
    frame->parent = vm->currentframe;
    vm->currentframe = frame;
    return LHS_TRUE;
}

int lhsframe_resetframe(LHSVM* vm)
{
    vm->currentframe = lhsframe_castframe(vm->currentframe)->parent;
    return LHS_TRUE;
}

void lhsframe_uninit(LHSVM* vm, LHSFrame* frame)
{
    lhshash_uninit(vm, &frame->localvars);
    lhsvector_uninit(vm, &frame->localvalues);
}
