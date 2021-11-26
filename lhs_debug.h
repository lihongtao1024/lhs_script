#pragma once
#include "lhs_config.h"
#include "lhs_value.h"
#include "lhs_vector.h"

typedef struct LHSSymbol
{
    long long line;
    long long column;
    LHSString* identifier;
} LHSSymbol;

typedef struct LHSDebug
{
    LHSVector symbols;
} LHSDebug;

int lhsdebug_init(void* vm, LHSDebug* debugs);

int lhsdebug_insert(void* vm, LHSDebug* debugs, 
    LHSString* identifier, long long line, long long column);

void* lhsdebug_at(void* vm, LHSDebug* debugs, size_t index);

void lhsdebug_uninit(void* vm, LHSDebug* debugs);
