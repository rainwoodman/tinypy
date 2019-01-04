#ifndef _VM_H_
# define _VM_H_

# include "tp.h"

void tp_deinit(TP);
tp_obj tp_call(TP,tp_obj self, tp_obj params);
tp_obj tp_ez_call(TP, const char *mod, const char *fnc, tp_obj params);
tp_obj tp_import(TP, const char * fname, const char * name, void *codes, int len);
tp_obj tp_compile(TP, tp_obj text, tp_obj fname);
tp_vm *tp_init(int argc, char *argv[]);

#endif /*!_VM_H_*/
