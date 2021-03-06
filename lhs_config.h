#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <limits.h>
#include <setjmp.h>

#define LHS_TRUE         (1)
#define LHS_FALSE        (0)
#define LHS_MAXCALLLAYER (65536)

typedef int (*lhsvm_delegate)(void*);
