#pragma once
#include "lhs_config.h"
#include "lhs_frame.h"
#include "lhs_vm.h"

#define lhsexec_castcc(o) ((LHSCallContext*)o)

typedef const char* IPID;
typedef struct LHSCallContext
{
    struct LHSCallContext* parent;
    LHSFrame* frame;
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
} LHSCallContext;

int lhsexec_pcall(LHSVM* vm, int narg, int nret, StkID errfn);
