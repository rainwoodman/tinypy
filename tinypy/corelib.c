#include "interp.h"

#include "corelib/builtins.c"

void _tp_import_corelib(TP) {
    tp->builtins = tp_import(tp, 0, "__builtins__", _tp_builtins_tpc,  sizeof(_tp_builtins_tpc));
}
