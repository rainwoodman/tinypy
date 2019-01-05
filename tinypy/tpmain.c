#include "interp.h"

/* INCLUDE */

int main(int argc, char *argv[]) {
    tp_vm *tp = tp_init(argc,argv);
    /* INIT */
    tp_ez_call(tp,"__builtins__", "_entry_point", tp_None);
    tp_deinit(tp);
    return(0);
}

/**/
