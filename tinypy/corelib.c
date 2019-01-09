#include "tp.h"

#include "corelib/builtins.c"

void tp_module_corelib_init(TP) {
    tp->builtins = tp_import_from_buffer(tp, 0, "__builtins__", _tp_builtins_tpc,  sizeof(_tp_builtins_tpc));
}
