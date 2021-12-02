#include "lhs_variable.h"
#include "lhs_execute.h"

long long lhsvariable_hashvar(void* data)
{
    LHSString* name = ((LHSVariable*)data)->name;
    if (!name->hash)
    {
        name->hash = lhsvalue_hashformula(name->data, name->length, 0);
    }
    return name->hash;
}

int lhsvariable_equalvar(void* data1, void* data2)
{
    LHSVariable* variable1 = (LHSVariable*)data1,
        * variable2 = (LHSVariable*)data2;
    if (variable1->chunk != variable2->chunk)
    {
        return LHS_FALSE;
    }

    LHSString* s1 = variable1->name,
        * s2 = variable2->name;
    if (s1->length != s2->length)
    {
        return LHS_FALSE;
    }

    return !memcmp(s1->data, s2->data, s1->length);
}
