#include "interp.h"

#include "interp/hash.c"

#include "tpd_list.c"
#include "tpd_dict.c"

#include "tp_list.c"
#include "tp_dict.c"
#include "tp_string.c"

#include "tpy_dict.c"
#include "tpy_list.c"
#include "tpy_string.c"

#include "tpy_func.c"
#include "tpy_data.c"

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
