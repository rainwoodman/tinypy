#ifndef _LIST_H_
# define _LIST_H_

# include "tp.h"

void _tp_list_realloc(TP, _tp_list *self, int len);
void _tp_list_set(TP,_tp_list *self,int k, tp_obj v, const char *error);
void _tp_list_free(TP, _tp_list *self);
tp_obj _tp_list_get(TP,_tp_list *self,int k,const char *error);
void _tp_list_insertx(TP,_tp_list *self, int n, tp_obj v);
void _tp_list_appendx(TP,_tp_list *self, tp_obj v);
void _tp_list_insert(TP,_tp_list *self, int n, tp_obj v);
void _tp_list_append(TP,_tp_list *self, tp_obj v);
tp_obj _tp_list_pop(TP,_tp_list *self, int n, const char *error);
int _tp_list_find(TP,_tp_list *self, tp_obj v);
tp_obj tp_index(TP);
_tp_list *_tp_list_new(TP);
tp_obj _tp_list_copy(TP, tp_obj rr);
tp_obj tp_append(TP);
tp_obj tp_pop(TP);
tp_obj tp_insert(TP);
tp_obj tp_extend(TP);
tp_obj tp_list_nt(TP);
tp_obj tp_list(TP);
tp_obj tp_list_n(TP,int n,tp_obj *argv);
int _tp_sort_cmp(tp_obj *a,tp_obj *b);
tp_obj tp_sort(TP);

#endif /*!_LIST_H_*/
