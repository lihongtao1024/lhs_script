#include "lhs_frame.h"
#include "lhs_link.h"
#include "lhs_load.h"
#include "lhs_code.h"

static void lhsframe_freechunk(LHSVM* vm, LHSVector* vector, LHSChunk** chunk)
{
    lhsmem_freeobject(vm, *chunk, sizeof(LHSChunk));
}

int lhsframe_init(LHSVM* vm, LHSFrame* frame)
{
    lhsvector_init(vm, &frame->values, sizeof(LHSValue), 0);
    lhshash_init(vm, &frame->variables, lhsvariable_hashvar, lhsvariable_equalvar, 0);
    lhsdebug_init(vm, &frame->debugs);
    lhsvector_init(vm, &frame->allchunks, sizeof(LHSChunk*), 0);

    frame->parent = 0;
    frame->curchunk = 0;
    frame->entry = vm->code.usize;
    frame->name = -1;
    frame->narg = 0;
    frame->nret = LHS_MULTRET;
    return LHS_TRUE;
}

int lhsframe_enterchunk(LHSVM* vm, LHSFrame* frame, void* loadf)
{
    LHSChunk* chunk = lhsmem_newobject(vm, sizeof(LHSChunk));
    *(LHSChunk**)lhsvector_increment(vm, &frame->allchunks) = chunk;

    chunk->index = (int)lhsvector_length(vm, &frame->allchunks) - 1;
    chunk->parent = frame->curchunk;
    frame->curchunk = chunk;
    return LHS_TRUE;
}

int lhsframe_setchunk(LHSVM* vm, LHSFrame* frame, LHSChunk* chunk)
{
    chunk->parent = frame->curchunk;
    frame->curchunk = chunk;
    return LHS_TRUE;
}

int lhsframe_resetchunk(LHSVM* vm, LHSFrame* frame)
{
    frame->curchunk = frame->curchunk->parent;
    return LHS_TRUE;
}

int lhsframe_leavechunk(LHSVM* vm, LHSFrame* frame, void* loadf)
{
    frame->curchunk = frame->curchunk->parent;
    return LHS_TRUE;
}

LHSVariable* lhsframe_insertconstant(LHSVM* vm)
{
    LHSValue* key = lhsvm_getvalue(vm, -1);
    size_t size = sizeof(LHSVariable);
    LHSVariable* nvariable = lhsmem_newobject(vm, size);
    nvariable->name = lhsvalue_caststring(key->gc);

    LHSVariable* ovariable = lhshash_find(vm, &vm->conststrhash, nvariable);
    if (ovariable)
    {
        lhsmem_freeobject(vm, nvariable, sizeof(LHSVariable));
        lhsvm_pop(vm, 1);
        return ovariable;
    }

    lhsmem_initgc(lhsgc_castgc(&nvariable->gc), LHS_TGCFULLDATA, size);
    lhsslink_push(lhsvm_castvm(vm), allgc, lhsgc_castgc(&nvariable->gc), next);
    lhshash_insert(vm, &vm->conststrhash, nvariable, 0);

    LHSValue* value = lhsvector_increment(vm, &vm->conststrvalue);
    memcpy(value, key, size);

    nvariable->index = (int)lhsvector_length(vm, &vm->conststrvalue) - 1;
    nvariable->mark = LHS_MARKSTRING;

    lhsvm_pop(vm, 1);
    return nvariable;
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
    int chunk = 0;
    if (global)
    {
        curframe = lhsframe_castmainframe(vm);
    }
    else
    {
        chunk = frame->curchunk->index;
    }

    LHSValue *value = lhsvector_increment(vm, &curframe->values);
    value->type = LHS_TNONE;

    variable->index = (int)lhsvector_length(vm, &curframe->values) - 1;
    variable->chunk = chunk;    
    variable->mark = global ? LHS_MARKGLOBAL : LHS_MARKLOCAL;
    variable->name = lhsvalue_caststring(key->gc);

    lhshash_insert(vm, &curframe->variables, variable, 0);
    lhsdebug_insert(vm, &curframe->debugs, variable->name, line, column);
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
    nvariable->chunk = frame->curchunk ? frame->curchunk->index : 0;
    nvariable->name = lhsvalue_caststring(key->gc);

    LHSVariable* ovariable = lhshash_find
    (
        vm, 
        &frame->variables, 
        nvariable
    );
    if (!ovariable && 
        frame->curchunk)
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

        if (!ovariable)
        {
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
    }

    lhsmem_freeobject(vm, nvariable, sizeof(LHSVariable));
    lhsvm_pop(vm, 1);
    return ovariable;
}

const char* lhsframe_name(LHSVM* vm, LHSFrame* frame)
{
    LHSSymbol* symbol = lhsdebug_at(vm, &frame->debugs, frame->name);
    return symbol->identifier->data;
}

int lhsframe_setframe(LHSVM* vm, LHSFrame* frame)
{
    frame->parent = vm->currentframe;
    vm->currentframe = frame;
    return LHS_TRUE;
}

int lhsframe_resetframe(LHSVM* vm)
{
    lhsframe_castframe(vm->currentframe)->curchunk = 0;
    vm->currentframe = lhsframe_castframe(vm->currentframe)->parent;
    return LHS_TRUE;
}

void lhsframe_uninit(LHSVM* vm, LHSFrame* frame)
{
    lhsdebug_uninit(vm, &frame->debugs);
    lhshash_uninit(vm, &frame->variables);
    lhsvector_uninit(vm, &frame->values);
    lhsvector_foreach(vm, &frame->allchunks, lhsframe_freechunk);
    lhsvector_uninit(vm, &frame->allchunks);
}
