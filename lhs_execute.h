#pragma once
#include "lhs_config.h"
#include "lhs_function.h"
#include "lhs_vm.h"

#define LHS_SCALL                       (1)
#define LHS_CCALL                       (2)
#define lhsexec_castcc(o)               ((LHSCallContext*)o)

typedef const char* IPID;
typedef struct LHSCallContext
{
    StkID base;         /*base for argument*/
    StkID errfn;        /*error handler*/
    IPID ip;            /*instruction pointer*/
    IPID rp;            /*return instruction pointer*/
    int nwant;          /*number of used results*/
    int line;
    int column;
    int refer;
    int type;
    LHSVector localvars;
    LHSFunction* func;
    struct LHSCallContext* parent;
} LHSCallContext;

int lhsexec_pcall(LHSVM* vm, LHSFunction* function, int nwant, StkID errfn);
