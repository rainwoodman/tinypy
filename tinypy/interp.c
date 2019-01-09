#include "interp.h"

#include "tpd_list.c"
#include "tpd_dict.c"

#include "tp_gc.c"

#include "tp_hash.c"
#include "tp_list.c"
#include "tp_dict.c"
#include "tp_string.c"

#include "tp_func.c"
#include "tp_data.c"

#include "tp_meta.c"
#include "tp_repr.c"

#include "tpy_string.c"

#include "tp_misc.c"
#include "tp_frame.c"

#include "tpy_dict.c"
#include "tpy_list.c"

#include "tp_import.c"
#include "tp_ops.c"

#ifdef TP_SANDBOX
#include "interp/sandbox.c"
#endif

tp_obj tp_None = {TP_NONE};

#include "tp_vm.c"

#include "tpy_builtins.c"
