#pragma once
#include "lhs_config.h"
#include "lhs_vm.h"

#define lhsexec_castcc(o) ((LHSCallContext*)o)

typedef const char* IPID;
typedef struct LHSCallContext
{
    struct LHSCallContext* prev;
    StkID base;
    StkID errfn;
    StkID top;
    IPID ip;
    IPID rp;
    int narg;
    int nret;
} LHSCallContext;

int lhsexec_pcall(LHSVM* vm, int narg, int nret, StkID errfn);
