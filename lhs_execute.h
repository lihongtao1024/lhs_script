#pragma once
#include "lhs_config.h"
#include "lhs_function.h"
#include "lhs_vm.h"

#define LHS_FCALL                       (1)
#define LHS_CCALL                       (2)
#define lhsexec_castcc(o)               ((LHSCallContext*)o)

typedef const char* IPID;
typedef struct LHSCallContext
{
    StkID base;         /*base for argument*/
    StkID errfn;        /*error handler*/
    StkID top;          /*stack top for this call*/
    IPID ip;            /*instruction point*/
    IPID rp;            /*return instruction point*/
    int narg;
    int nret;
    int line;
    int column;
    int refer;
    int type;
    LHSVector localvars;
    LHSFunction* frame;
    struct LHSCallContext* parent;
} LHSCallContext;

int lhsexec_pcall(LHSVM* vm, int narg, int nret, StkID errfn);
