#include "tp.h"
#include "tp_internal.h"

tp_obj tp_None = {TP_NONE};
tp_obj tp_True = {TP_NUMBER, .num = 1};
tp_obj tp_False = {TP_NUMBER, .num = 0};

#include "tp_number.c"
#include "tpd_list.c"
#include "tpd_dict.c"

#include "tp_echo.c"
#include "tp_param.c"

#include "tp_gc.c"
#include "tp_hash.c"
#include "tp_list.c"
#include "tp_dict.c"
#include "tp_string.c"

#include "tp_meta.c"
#include "tp_data.c"
#include "tp_func.c"
#include "tp_frame.c"

#include "tp_repr.c"

#include "tpy_string.c"
#include "tpy_dict.c"
#include "tpy_list.c"

#include "tp_vm.c"

/* FIXME: after string / dict gets a meta, register these methods
 * to the meta in tpy_builtin
 * and tp_ops above tpy **/
#include "tp_ops.c" 
#include "tp_import.c"
#include "tp_interp.c"

#ifdef TP_SANDBOX
#include "interp/sandbox.c"
#endif


#include "tpy_builtins.c"
