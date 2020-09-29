
tp_obj tp_main(TP,char *fname, void *code, int len) {
    return tp_import_from_buffer(tp,fname, "__main__", code, len);
}

tp_obj tp_ez_call(TP, const char *mod, const char *func, tp_obj params) {
    tp_obj tmp;
    tmp = tp_get(tp,tp->modules,tp_string_atom(tp, mod));
    tmp = tp_get(tp,tmp,tp_string_atom(tp, func));
    return tp_call(tp,tmp,params);
}

/* Function: tp_compile
 * Compile some tinypy code.
 *
 */
tp_obj tp_compile(TP, tp_obj text, tp_obj fname) {
    return tp_ez_call(tp, "tinypy.compiler.py2bc", "compile", tp_params_v(tp, 2, text, fname));
}

tp_obj tp_args(TP, int argc, char *argv[]) {
    tp_obj self = tp_list_t(tp);
    int i;
    for (i=1; i<argc; i++) {
        tpd_list_append(tp, self.list.val, tp_string_atom(tp, argv[i]));
    }
    return self;
}

void tp_module_sys_init (TP, int argc, char * argv[]) {
    tp_obj sys = tp_dict_t(tp);
    tp_obj args = tp_args(tp,argc,argv);
    tp_set(tp, sys, tp_string_atom(tp, "version"), tp_string_atom(tp, "tinypy 1.2+SVN"));
    tp_set(tp, sys, tp_string_atom(tp, "modules"), tp->modules);
    tp_set(tp, sys, tp_string_atom(tp, "argv"), args);
    tp_set(tp, tp->modules, tp_string_atom(tp, "sys"), sys);
}

/* Function: tp_init
 * Initializes a new virtual machine.
 *
 * The given parameters have the same format as the parameters to main, and
 * allow passing arguments to your tinypy scripts.
 *
 * Returns:
 * The newly created tinypy instance.
 */
tp_vm * tp_init(int argc, char *argv[]) {
    tp_vm *tp = tp_create_vm();
    tp_module_sys_init(tp, argc, argv);
    tp_module_builtins_init(tp);

    tp_module_corelib_init(tp);
    tp_module_compiler_init(tp);
    return tp;
}


/* Function: tp_deinit
 * Destroys a VM instance.
 * 
 * When you no longer need an instance of tinypy, you can use this to free all
 * memory used by it. Even when you are using only a single tinypy instance, it
 * may be good practice to call this function on shutdown.
 */
void tp_deinit(TP) {
    while (tp->root.list.val->len) {
        tpd_list_pop(tp, tp->root.list.val, 0, "tp_deinit");
    }
    tp_full(tp); tp_full(tp);
    tp_delete(tp,tp->root);
    tp_gc_deinit(tp);
    tp->mem_used -= sizeof(tp_vm); 
    free(tp);
}
