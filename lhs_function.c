#include "lhs_function.h"
#include "lhs_link.h"
#include "lhs_load.h"
#include "lhs_code.h"
#include "lhs_variable.h"

int lhsfunction_init(LHSVM* vm, LHSFunction* function)
{
    function->frame = 0;
    function->name = -1;
    function->narg = 0;
    function->nlocalvars = 0;
    function->nret = LHS_UNCERTAIN;

    lhslink_init(function, next);
    lhsbuf_init(vm, &function->code);
    lhshash_init(vm, &function->localvars, lhsvar_hashvar, lhsvar_equalvar, 0);
    return LHS_TRUE;
}

const char* lhsfunction_getname(LHSVM* vm, LHSFunction* function)
{
    LHSVar* var = lhsvector_at(vm, &vm->conststrs, function->name);
    return lhsvalue_caststring(var->value.gc)->data;
}

void lhsfunction_uninit(LHSVM* vm, LHSFunction* function)
{
    lhsbuf_uninit(vm, &function->code);
    lhshash_uninit(vm, &function->localvars);
}
