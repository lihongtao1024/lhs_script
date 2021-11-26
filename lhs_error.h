#pragma once
#include "lhs_config.h"

/*error handler*/
#define lhs_errno(fmt, ...) \
printf(fmt##", description: %s.\n", __VA_ARGS__, strerror(errno))

#define lhs_errmsg(s) \
printf("%s\n", s)
