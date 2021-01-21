#include <stdlib.h>
#include "tp.h"

void * _tp_import_modules(TP);
/* from runtime */
tp_obj tp_load(TP, const char*);

int main(int argc,  char *argv[]) {
    /* fixme: make this argc */
    char * p = getenv("TP_DISABLE_PY_RUNTIME");
    int enable_py_runtime = (p == NULL) || (*p == '0');

    tp_vm *tp = tp_init(argc, argv, enable_py_runtime);

    tp_obj fname = tp_string_atom(tp, argv[1]);
    tp_obj code = tp_load(tp, argv[1]);

    tp_obj module = tp_import(tp, tp_string_atom(tp, "__main__"), code, fname);

    tp_deinit(tp);
    return(0);
}
