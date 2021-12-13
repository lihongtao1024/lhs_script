#pragma once
#include "lhs_config.h"
#include "lhs_vm.h"

int lhsexec_pcall(LHSVM* vm, StkID func, int narg, int nret, StkID errfn);