#pragma once
#include "lhs_config.h"
#include "lhs_vm.h"

#define lhsexec_castcc(o) ((LHSCallContext*)o)

typedef const char* IPID;
typedef struct LHSCallContext
{
    struct LHSCallContext* prev;
    StkID base;         /*base for argument*/
    StkID errfn;        /*error handler*/
    StkID top;          /*stack top for this call*/
    IPID ip;            /*instruction point*/
    IPID rp;            /*return instruction point*/
    int narg;
    int nret;
} LHSCallContext;

int lhsexec_pcall(LHSVM* vm, int narg, int nret, StkID errfn);
