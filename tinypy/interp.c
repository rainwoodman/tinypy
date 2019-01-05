#include "interp.h"

#include "interp/list.c"
#include "interp/dict.c"
#include "interp/misc.c"
#include "interp/string.c"
#include "interp/builtins.c"
#include "interp/gc.c"
#include "interp/ops.c"

#ifdef TP_SANDBOX
#include "interp/sandbox.c"
#endif

tp_obj tp_None = {TP_NONE};

#include "interp/vm.c"

