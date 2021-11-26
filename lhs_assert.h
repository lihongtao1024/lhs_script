#pragma once
#include "lhs_config.h"

#define lhsassert_true(e) \
assert((e))

#define lhsassert_truereturn(e) \
if (!(e))\
{\
    assert((e));\
    return;\
}

#define lhsassert_trueresult(e, r) \
if (!(e))\
{\
    assert((e));\
    return (r);\
}
