int _tp_lookup_(TP, tp_obj self, tp_obj k, tp_obj *meta, int depth) {
    int n = tpd_dict_find(tp, self.dict.val, k);
    if (n != -1) {
        *meta = self.dict.val->items[n].val;
        return 1;
    }
    depth--;
    if (!depth) {
        tp_raise(0,tp_string("(tp_lookup) RuntimeError: maximum lookup depth exceeded"));
    }
    if (self.dict.dtype != 0
    && self.dict.val->meta.type == TP_DICT
    && _tp_lookup_(tp, self.dict.val->meta, k, meta, depth)) {
        if (self.dict.dtype == 2 && meta->type == TP_FNC) {
            /* make a copy of the function; FIXME: what does dtype == 2 mean? */
            *meta = tp_fnc_new(tp,
                meta->fnc.ftype|2,
                meta->fnc.cfnc,
                meta->fnc.info->code,
                self,
                meta->fnc.info->globals);
        }
        return 1;
    }
    return 0;
}

int _tp_lookup(TP,tp_obj self, tp_obj k, tp_obj *meta) {
    return _tp_lookup_(tp, self, k, meta, 8);
}

#define TP_META_BEGIN(self,name) \
    if (self.type == TP_DICT && self.dict.dtype == 2) { \
        tp_obj meta; if (_tp_lookup(tp,self,tp_string(name),&meta)) {

#define TP_META_END \
        } \
    }

