#include "lhs_frame.h"
#include "lhs_assert.h"
#include "lhs_link.h"

static void lhsframe_uninitnext(LHSFrame* maimframe, LHSFrame* frame)
{
    lhsassert_truereturn(maimframe && frame);
    lhsframe_uninit(maimframe->vm, frame);
}

int lhsframe_init(LHSVM* vm, LHSFrame* frame, size_t level)
{
    lhsassert_trueresult(vm && frame, false);

    frame->vm = vm;
    frame->level = level;
    frame->base = lhsvector_length(vm, &vm->stack);
    frame->ret = 0;
    lhshash_init(vm, &frame->variables, lhsvariable_hashvar, lhsvariable_equalvar);
    lhsvector_init(vm, &frame->values, sizeof(LHSVariable));
    lhsvector_init(vm, &frame->codes, sizeof(long long));
    lhsdebug_init(vm, &frame->debug);
    lhsslink_init(frame, next);
    return true;
}

int lhsframe_insertvariable(LHSVM* vm, LHSFrame* frame, 
    long long line, long long column)
{
    lhsassert_trueresult(vm && frame && lhsvm_gettop(vm) >= 2, false);

    LHSVariable* variable = lhsvariable_castvar
    (
        lhsmem_newgcobject
        (
            vm, 
            sizeof(LHSVariable), 
            LHS_TGCFULLDATA
        )
    );
    lhsassert_trueresult(variable, false);

    LHSValue* value = lhsvector_increment(vm, &frame->values);
    lhsassert_trueresult(value, false);
    memcpy(value, lhsvm_getvalue(vm, -1), sizeof(LHSValue));

    variable->index = lhsvector_length(vm, &frame->values) - 1;
    variable->marked = LHS_MARKLOCAL;
    variable->name = lhsvalue_caststring(lhsvm_getvalue(vm, -2)->gc);

    lhshash_insert(vm, &frame->variables, variable, 0);
    lhsdebug_insert(vm, &frame->debug, variable->name, line, column);
    lhsvm_pop(vm, 2);
    return true;
}

int lhsframe_getvariable(LHSVM* vm, LHSFrame* frame)
{
    lhsassert_trueresult(vm && frame && lhsvm_gettop(vm) >= 1, false);

    return true;
}

const char* lhsframe_name(LHSVM* vm, LHSFrame* frame)
{
    lhsassert_trueresult(vm && frame, 0);

    LHSSymbol* symbol = lhsdebug_at(vm, &frame->debug, LHS_FRAMENAME);
    lhsassert_trueresult(symbol && symbol->identifier, 0);

    return symbol->identifier->data;
}

void lhsframe_uninit(LHSVM* vm, LHSFrame* frame)
{
    lhsassert_truereturn(vm && frame);

    lhshash_uninit(vm, &frame->variables);
    lhsvector_uninit(vm, &frame->values);
    lhsvector_uninit(vm, &frame->codes);
    lhsdebug_uninit(vm, &frame->debug);
    lhsslink_foreach(LHSFrame, frame, next, next, lhsframe_uninitnext);
}
