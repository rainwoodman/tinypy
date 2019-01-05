#include <vm/core.h>
#include <vm/vm.h>

/* INCLUDE */

int main(int argc, char *argv[]) {
    tp_vm *tp = tp_init(argc,argv);
    /* INIT */
    tp_ez_call(tp,"tinypy.compiler.py2bc","tinypy",tp_None);
    tp_deinit(tp);
    return(0);
}

/**/
