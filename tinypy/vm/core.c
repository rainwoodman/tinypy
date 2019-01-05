#include "core.h"

#include "core/list.c"
#include "core/dict.c"
#include "core/misc.c"
#include "core/string.c"
#include "core/builtins.c"
#include "core/gc.c"
#include "core/ops.c"

#ifdef TP_SANDBOX
#include "core/sandbox.c"
#endif

tp_obj tp_None = {TP_NONE};

