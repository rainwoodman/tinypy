#include "tp.h"

/* INCLUDE */

void * _tp_import_modules(TP);

/* from runtime */
tp_obj tp_load(TP, const char*);

int main(int argc, char *argv[]) {
    tp_vm *tp = tp_init(argc, argv, 1);
    /* INIT */
    _tp_import_modules(tp);
    
    tp_obj fname = tp_string_atom(tp, argv[1]);
    tp_obj source = tp_load(tp, argv[1]);
    tp_obj code = tp_compile(tp, source, fname);
    tp_obj module = tp_import(tp, tp_string_atom(tp, "__main__"), code, fname);

    tp_deinit(tp);
    return(0);
}

/**/
