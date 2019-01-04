#ifndef _DICT_H_
# define _DICT_H_

# include "tp.h"

void _tp_dict_free(TP, _tp_dict *self);
tp_obj _tp_dict_copy(TP,tp_obj rr);
int _tp_dict_find(TP,_tp_dict *self,tp_obj k);
int _tp_dict_next(TP,_tp_dict *self);
tp_obj _tp_dict_get(TP,_tp_dict *self,tp_obj k, const char *error);
void _tp_dict_set(TP,_tp_dict *self,tp_obj k, tp_obj v);
void _tp_dict_del(TP,_tp_dict *self,tp_obj k, const char *error);
tp_obj tp_dict(TP);
tp_obj tp_dict_n(TP,int n, tp_obj* argv);
tp_obj tp_merge(TP);

#endif /*!_DICT_H_*/
