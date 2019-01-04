#ifndef _MISC_H_
# define _MISC_H_

# include "tp.h"
# include "list.h"


tp_obj _tp_tcall(TP,tp_obj fnc);
tp_obj tp_fnc_new(TP,int t, void *v, tp_obj c,tp_obj s, tp_obj g);
tp_obj tp_def(TP,tp_obj code, tp_obj g);
tp_obj tp_fnc(TP,tp_obj v(TP));
tp_obj tp_method(TP,tp_obj self,tp_obj v(TP));
tp_obj tp_data(TP,int magic,void *v);
tp_obj tp_params(TP);
tp_obj tp_params_n(TP,int n, tp_obj argv[]);
tp_obj tp_params_v(TP,int n,...);

#endif /*!_MISC_H_*/
