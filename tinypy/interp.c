#include "interp.h"

#include "interp/hash.c"

#include "list/tpd.c"
#include "list/tp.c"
#include "list/tpy.c"

#include "dict/tpd.c"
#include "dict/tp.c"
#include "dict/tpy.c"

#include "string/tp.c"
#include "string/tpy.c"

#include "interp/misc.c"
#include "interp/meta.c"
#include "interp/frame.c"
#include "interp/import.c"
#include "interp/gc.c"
#include "interp/ops.c"

#ifdef TP_SANDBOX
#include "interp/sandbox.c"
#endif

tp_obj tp_None = {TP_NONE};

#include "interp/vm.c"

#include "interp/builtins.c"
