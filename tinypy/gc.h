#ifndef _GC_H_
# define _GC_H_

# include "tp.h"

void tp_grey(TP,tp_obj v);
void tp_gc_init(TP);
void tp_gc_deinit(TP);
void tp_delete(TP,tp_obj v);
void tp_full(TP);
tp_obj tp_track(TP,tp_obj v);

#endif /*!_GC_H_*/
