#pragma once
#include "lhs_config.h"
#include "lhs_value.h"
#include "lhs_vm.h"

int lhscode_unarayexpression(char symbol, int index);

int lhscode_binaryexpression(char symbol, int index1, int index2);
