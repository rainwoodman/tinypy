int _tp_lookup_(TP, tp_obj self, int hash, tp_obj k, tp_obj *meta, int depth) {
    int n = tpd_dict_hashfind(tp, self.dict.val, hash, k);
    if (n != -1) {
        *meta = self.dict.val->items[n].val;
        return 1;
    }
    depth--;
    if (!depth) {
        tp_raise(0,tp_string_atom(tp, "(tp_lookup) RuntimeError: maximum lookup depth exceeded"));
    }
    if (self.type.magic != TP_DICT_RAW
    && self.dict.val->meta.type.typeid == TP_DICT
    && _tp_lookup_(tp, self.dict.val->meta, hash, k, meta, depth)) {
        if (self.type.magic == TP_DICT_OBJECT && meta->type.typeid == TP_FUNC) {
            /* make a copy of the function that is bound to the instance;
 *               FIXME: what does dtype == 2 mean? */
            *meta = tp_bind(tp, *meta, self);
        }
        return 1;
    }
    return 0;
}

int _tp_lookup(TP, tp_obj self, tp_obj k, tp_obj *meta) {
    return _tp_lookup_(tp, self, tp_hash(tp, k), k, meta, 8);
}

#define TP_META_BEGIN(self,name) \
    if (self.type.typeid == TP_DICT && self.type.magic == TP_DICT_OBJECT) { \
        tp_obj meta; if (_tp_lookup(tp,self,tp_string_atom(tp, name),&meta)) {

#define TP_META_END \
        } \
    }

