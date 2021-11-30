#include "lhs_frame.h"
#include "lhs_link.h"

static void lhsframe_uninitnext(LHSFrame* maimframe, LHSFrame* frame)
{
    lhsframe_uninit(maimframe->vm, frame);
}

int lhsframe_init(LHSVM* vm, LHSFrame* frame, size_t level)
{
    frame->vm = vm;
    frame->level = level;
    frame->base = lhsvector_length(vm, &vm->stack);
    frame->ret = 0;
    lhshash_init(vm, &frame->localvars, lhsvariable_hashvar, lhsvariable_equalvar);
    lhsvector_init(vm, &frame->localvalues, sizeof(LHSVariable));
    lhsvector_init(vm, &frame->codes, sizeof(long long));
    lhsdebug_init(vm, &frame->debug);
    lhsslink_init(frame, next);
    return true;
}

LHSVariable* lhsframe_insertvariable(LHSVM* vm, LHSFrame* frame, 
    long long line, long long column)
{    
    LHSValue* key = lhsvm_getvalue(vm, -1);
    LHSVariable* variable = lhsvariable_castvar
    (
        lhsmem_newgcobject
        (
            vm, 
            sizeof(LHSVariable), 
            LHS_TGCFULLDATA
        )
    );

    LHSValue* local = lhsvector_increment(vm, &frame->localvalues);
    local->type = LHS_TNONE;

    variable->index = (int)lhsvector_length(vm, &frame->localvalues) - 1;
    variable->mark = LHS_MARKLOCAL;
    variable->name = lhsvalue_caststring(key->gc);

    lhshash_insert(vm, &frame->localvars, variable, 0);
    lhsdebug_insert(vm, &frame->debug, variable->name, line, column);
    lhsvm_pop(vm, 1);
    return variable;
}

LHSVariable* lhsframe_getvariable(LHSVM* vm, LHSFrame* frame)
{
    LHSValue* key = lhsvm_getvalue(vm, -1);
    LHSVariable* nvariable = lhsvariable_castvar
    (
        lhsmem_newobject(vm, sizeof(LHSVariable))
    );
    nvariable->name = lhsvalue_caststring(key->gc);

    LHSVariable* ovariable = lhshash_find(vm, &frame->localvars, nvariable);
    lhsmem_freeobject(vm, nvariable, sizeof(LHSVariable));
    lhsvm_pop(vm, 1);
    return ovariable;
}

const char* lhsframe_name(LHSVM* vm, LHSFrame* frame)
{
    LHSSymbol* symbol = lhsdebug_at(vm, &frame->debug, LHS_FRAMENAME);
    return symbol->identifier->data;
}

void lhsframe_uninit(LHSVM* vm, LHSFrame* frame)
{
    lhshash_uninit(vm, &frame->localvars);
    lhsvector_uninit(vm, &frame->localvalues);
    lhsvector_uninit(vm, &frame->codes);
    lhsdebug_uninit(vm, &frame->debug);
    lhsslink_foreach(LHSFrame, frame, next, next, lhsframe_uninitnext);
}
