#include "lhs_debug.h"

int lhsdebug_init(void* vm, LHSDebug* debugs)
{
    lhsvector_init(vm, &debugs->symbols, sizeof(LHSSymbol), 0);
    return LHS_TRUE;
}

int lhsdebug_insert(void* vm, LHSDebug* debugs,
    LHSString* identifier, long long line, long long column)
{
    LHSSymbol* symbol = lhsvector_increment(vm, &debugs->symbols);
    symbol->line = line;
    symbol->column = column;
    symbol->identifier = identifier;
    return LHS_TRUE;
}

void* lhsdebug_at(void* vm, LHSDebug* debugs, size_t index)
{
    return lhsvector_at(vm, &debugs->symbols, index);
}

void lhsdebug_uninit(void* vm, LHSDebug* debugs)
{
    lhsvector_uninit(vm, &debugs->symbols);
}

