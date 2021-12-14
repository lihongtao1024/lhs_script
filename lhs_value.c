#include "lhs_value.h"

const char* lhsvalue_typename[] =
{
    "null", "integer", "number", "boolean", "delegate",
    "gc", "vm", "string", "string", "fdata"
};

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

long long lhsvalue_hashstr(void* data)
{
    LHSString* str = lhsvalue_caststring(data);
    return lhsvalue_hashformula(str->data, str->length, 0);
}

int lhsvalue_equalstr(void* data1, void* data2)
{
    LHSString* s1 = lhsvalue_caststring(data1),
        * s2 = lhsvalue_caststring(data2);
    if (s1->length != s2->length)
    {
        return LHS_FALSE;
    }

    return !memcmp(s1->data, s2->data, s1->length);
}
