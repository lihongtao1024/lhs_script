#include "lhs_debug.h"
#include "lhs_assert.h"

int lhsdebug_init(void* vm, LHSDebug* debugs)
{
    lhsassert_trueresult(vm && debugs, false);

    lhsvector_init(vm, &debugs->symbols, sizeof(LHSSymbol));
    return true;
}

int lhsdebug_insert(void* vm, LHSDebug* debugs,
    LHSString* identifier, long long line, long long column)
{
    lhsassert_trueresult(vm && debugs && identifier, false);

    LHSSymbol* symbol = lhsvector_increment(vm, &debugs->symbols);
    lhsassert_trueresult(symbol, false);

    symbol->line = line;
    symbol->column = column;
    symbol->identifier = identifier;
    return true;
}

void* lhsdebug_at(void* vm, LHSDebug* debugs, size_t index)
{
    lhsassert_trueresult(vm && debugs, false);

    return lhsvector_at(vm, &debugs->symbols, index);
}

void lhsdebug_uninit(void* vm, LHSDebug* debugs)
{
    lhsassert_truereturn(vm && debugs);

    lhsvector_uninit(vm, &debugs->symbols);
}

