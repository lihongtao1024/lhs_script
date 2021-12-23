#include "lhs_function.h"
#include "lhs_link.h"
#include "lhs_load.h"
#include "lhs_code.h"

int lhsfunction_init(LHSVM* vm, LHSFunction* function)
{
    lhshash_init(vm, &function->localvars, lhsvar_hashvar, lhsvar_equalvar, 0);
    function->entry = vm->code.usize;
    function->name = -1;
    function->narg = 0;
    function->nlocalvars = 0;
    function->nret = LHS_UNCERTAIN;
    return LHS_TRUE;
}

const char* lhsfunction_getname(LHSVM* vm, LHSFunction* function)
{
    LHSVar* var = lhsvector_at(vm, &vm->conststrs, function->name);
    return lhsvalue_caststring(var->value.gc)->data;
}

void lhsfunction_uninit(LHSVM* vm, LHSFunction* function)
{
    lhshash_uninit(vm, &function->localvars);
}
