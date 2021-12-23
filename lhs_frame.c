#include "lhs_frame.h"
#include "lhs_link.h"

int lhsframe_init(LHSVM* vm, LHSFrame* frame)
{
    frame->name = 0;
    frame->mainfunc = 0;
    lhslink_init(frame, allfunc);
    lhslink_init(frame, next);
    return LHS_TRUE;
}

int lhsframe_uninit(LHSVM* vm, LHSFrame* frame)
{
    return LHS_TRUE;
}
