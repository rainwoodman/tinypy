#ifndef _BUILTINS_H_
# define _BUILTINS_H_

# include "tp.h"
# include "list.h"
# include "misc.h"

tp_obj tp_print(TP);
tp_obj tp_bind(TP);
tp_obj tp_min(TP);
tp_obj tp_max(TP);
tp_obj tp_copy(TP);
tp_obj tp_len_(TP);
tp_obj tp_assert(TP);
tp_obj tp_range(TP);
tp_obj tp_system(TP);
tp_obj tp_istype(TP);
tp_obj tp_float(TP);
tp_obj tp_save(TP);
tp_obj tp_load(TP);
tp_obj tp_fpack(TP);
tp_obj tp_abs(TP);
tp_obj tp_int(TP);
tp_obj tp_round(TP);
tp_obj tp_exists(TP);
tp_obj tp_mtime(TP);
int _tp_lookup(TP,tp_obj self, tp_obj k, tp_obj *meta);

#define TP_META_BEGIN(self,name) \
    if (self.dict.dtype == 2) { \
        tp_obj meta; if (_tp_lookup(tp,self,tp_string(name),&meta)) {

#define TP_META_END \
        } \
    }

tp_obj tp_setmeta(TP);
tp_obj tp_getmeta(TP);
tp_obj tp_object(TP);
tp_obj tp_object_new(TP);
tp_obj tp_object_call(TP);
tp_obj tp_getraw(TP);
tp_obj tp_class(TP);
tp_obj tp_builtins_bool(TP);

#endif /*!_BUILTINS_H_*/
