#define CPYTHON_MOD

#include "tp.c"

#include <readline/readline.h>
#include <readline/history.h>

/* Compile with: "gcc -o repl tinypy/repl.c -lm -lreadline -Wall"
   Supports basic one-line instructions. No module included.
*/

tp_obj run_protected(TP, char* source, tp_obj globals) {
    if(setjmp(tp->nextexpr)) {
        --(tp->cur);
        printf("\nException:\n"); tp_echo(tp,tp->ex); printf("\n");
        return tp_None;
    }
    return tp_eval(tp, source, globals);
}

int main(int argc, char *argv[]) {
    char* line;

    using_history();
    stifle_history(100);
    read_history(".tinypy_history");

    tp_vm *tp = tp_init(argc,argv);
    tp_obj globals = tp_dict(tp);
    printf("Tinypy REPL.\n");
    while(NULL != (line = readline("> "))) {
        if(!strcmp(line, "quit")) {
            break;
        } else if(!strcmp(line, "globals")) {
            tp_echo(tp, tp_str(tp, globals));
            printf("\n");
            continue;
        }
        add_history (line);
        write_history(".tinypy_history");
        tp_obj result = run_protected(tp, line, globals);
        tp_echo(tp, result);
        printf("\n");
        free(line);
    }
    tp_deinit(tp);
    return(0);
}

/**/
