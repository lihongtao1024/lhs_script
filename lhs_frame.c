#include "lhs_frame.h"
#include "lhs_link.h"
#include "lhs_code.h"

static void lhsframe_uninitnext(LHSFrame* main, LHSFrame* frame, LHSVM* vm)
{
    lhsframe_uninit(vm, frame);
}

int lhsframe_init(LHSVM* vm, LHSFrame* frame)
{
    lhsvector_init(vm, &frame->chunks, sizeof(LHSChunk));
    lhsvector_init(vm, &frame->values, sizeof(LHSValue));
    lhshash_init(vm, &frame->variables, lhsvariable_hashvar, lhsvariable_equalvar);
    lhsdebug_init(vm, &frame->debug);

    frame->curchunk = 0;
    lhsslink_init(frame, next);
    lhsframe_enterchunk(vm, frame);
    return LHS_TRUE;
}

int lhsframe_enterchunk(LHSVM* vm, LHSFrame* frame)
{
    LHSChunk* chunk = lhsvector_increment(vm, &frame->chunks);
    chunk->index = (int)lhsvector_length(vm, &frame->chunks) - 1;
    chunk->parent = frame->curchunk;
    frame->curchunk = chunk;
    lhscode_unaryl(vm, &vm->codes, OP_PUSHC, chunk->index);
    return LHS_TRUE;
}

int lhsframe_leavechunk(LHSVM* vm, LHSFrame* frame)
{
    frame->curchunk = frame->curchunk->parent;
    lhscode_unary(vm, &vm->codes, OP_POPC);
    return LHS_TRUE;
}

LHSVariable* lhsframe_insertvariable(LHSVM* vm, LHSFrame* frame, 
    long long line, long long column, int global)
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

    LHSFrame* curframe = frame; 
    int chunk = frame->curchunk->index;
    if (global)
    {
        curframe = lhsframe_castmainframe(vm);
        chunk = 0;
    }

    LHSValue *value = lhsvector_increment(vm, &curframe->values);
    value->type = LHS_TNONE;

    variable->chunk = chunk;
    variable->index = (int)lhsvector_length(vm, &curframe->values) - 1;
    variable->mark = global ? LHS_MARKGLOBAL : LHS_MARKLOCAL;
    variable->name = lhsvalue_caststring(key->gc);

    lhshash_insert(vm, &curframe->variables, variable, 0);
    lhsdebug_insert(vm, &curframe->debug, variable->name, line, column);
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
    nvariable->chunk = frame->curchunk->index;
    nvariable->name = lhsvalue_caststring(key->gc);

    LHSVariable* ovariable = lhshash_find
    (
        vm, 
        &frame->variables, 
        nvariable
    );
    if (!ovariable)
    {
        for (LHSChunk* chunk = frame->curchunk->parent; 
            chunk; 
            chunk = chunk->parent)
        {
            nvariable->chunk = chunk->index;
            ovariable = lhshash_find(vm, &frame->variables, nvariable);
            if (ovariable)
            {
                break;
            }
        }

        nvariable->chunk = 0;
        ovariable = lhshash_find
        (
            vm, 
            &lhsframe_castmainframe(vm)->variables, 
            nvariable
        );
        if (ovariable && ovariable->mark != LHS_MARKGLOBAL)
        {
            ovariable = 0;
        }
    }

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
    lhsframe_leavechunk(vm, frame);
    lhsdebug_uninit(vm, &frame->debug);
    lhshash_uninit(vm, &frame->variables);
    lhsvector_uninit(vm, &frame->values);
    lhsvector_uninit(vm, &frame->chunks);
    lhsslink_foreach(LHSFrame, frame, next, next, lhsframe_uninitnext, vm);
}
