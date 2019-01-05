
tp_vm *tp_init(int argc, char *argv[]);
void tp_deinit(TP);
tp_obj tp_import(TP, const char * fname, const char * name, void *codes, int len);
tp_obj tp_ez_call(TP, const char *mod, const char *fnc, tp_obj params);
void tp_run(TP,int cur);
