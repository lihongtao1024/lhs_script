#include "lhs_variable.h"
#include "lhs_assert.h"

long long lhsvariable_hashvar(void* data)
{
    lhsassert_trueresult(data, false);

    LHSString* name = ((LHSVariable*)data)->name;
    if (!name->hash)
    {
        name->hash = lhsvalue_hashformula(name->data, name->length, 0);
    }
    return name->hash;
}

int lhsvariable_equalvar(void* data1, void* data2)
{
    lhsassert_trueresult(data1 && data2, 0);

    LHSString* s1 = ((LHSVariable*)data1)->name,
        * s2 = ((LHSVariable*)data2)->name;
    if (s1->length != s2->length)
    {
        return false;
    }

    return !memcmp(s1->data, s2->data, s1->length);
}
