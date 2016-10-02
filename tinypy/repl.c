#define CPYTHON_MOD

#include "tp.c"

tp_obj run_protected(TP, char* source, tp_obj globals) {
    if(setjmp(tp->nextexpr)) {
        --(tp->cur);
        printf("\nException:\n"); tp_echo(tp,tp->ex); printf("\n");
        return tp_None;
    }
    return tp_eval(tp, source, globals);
}

int main(int argc, char *argv[]) {
    char source[1024];
    tp_vm *tp = tp_init(argc,argv);
    tp_obj globals = tp_dict(tp);
    printf("Tinypy REPL.\n> ");
    while(fgets(source, 1024, stdin)) {
        tp_obj result = run_protected(tp, source, globals);
        tp_echo(tp, result);
        printf("\n> ");
    }
    tp_deinit(tp);
    return(0);
}

/**/
