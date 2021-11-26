#include "lhs_value.h"
#include "lhs_assert.h"

long long lhsvalue_hashformula(const char* str, size_t l, size_t seed)
{
    if (!seed)
    {
        seed = ((l << 2) + (l >> 1)) & 0xffff;
    }

    long long h = seed ^ l;
    for (; l > 0; l--)
    {
        h ^= ((h << 5) + (h >> 2) + (char)(str[l - 1]));
    }

    return h;
}

long long lhsvalue_hashstring(void* data)
{
    lhsassert_trueresult(data, 0);

    LHSString* str = lhsvalue_caststring(data);
    return lhsvalue_hashformula(str->data, str->length, 0);
}

int lhsvalue_equalstring(void* data1, void* data2)
{
    lhsassert_trueresult(data1 && data2, 0);

    LHSString* s1 = lhsvalue_caststring(data1),
        * s2 = lhsvalue_caststring(data2);
    if (s1->length != s2->length)
    {
        return false;
    }

    return !memcmp(s1->data, s2->data, s1->length);
}
