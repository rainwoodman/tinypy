int _tp_lookup_(TP, tp_obj self, int hash, tp_obj k, tp_obj *meta, int depth) {
    if(self.type.typeid == TP_DICT) {
        /* first do a dict look up from the object itself */
        int n = tpd_dict_hashfind(tp, self.dict.val, hash, k);
        if (n != -1) {
            *meta = self.dict.val->items[n].val;
            return 1;
        }
        /* raw dict, no meta chain up. we are done. */
        if(self.type.magic == TP_DICT_RAW)
            return 0;
    }

    depth--;
    if (!depth) {
        tp_raise(0,tp_string_atom(tp, "(tp_lookup) RuntimeError: maximum lookup depth exceeded"));
    }
    if (self.type.typeid < TP_HAS_META) {
        return 0;
    }

    if (self.obj.info->meta.type.typeid == TP_DICT &&
        _tp_lookup_(tp, self.obj.info->meta, hash, k, meta, depth)) {
        if ( meta->type.typeid == TP_FUNC) {
            /* object dict or string, or list */
            if ((self.type.typeid == TP_DICT && self.type.magic == TP_DICT_OBJECT)
                || self.type.typeid == TP_LIST
                || self.type.typeid == TP_STRING
            ) {
                /* an object is seen, bind the meta method to the instance FIXME: for list and string */
                *meta = tp_bind(tp, *meta, self);
            }
        }
        return 1;
    }
    return 0;
}

int _tp_lookup(TP, tp_obj self, tp_obj k, tp_obj *meta) {
    return _tp_lookup_(tp, self, tp_hash(tp, k), k, meta, 8);
}

#define TP_META_BEGIN(self,name) \
    if ((self.type.typeid == TP_DICT && self.type.magic == TP_DICT_OBJECT) || \
        (self.type.typeid == TP_STRING || self.type.typeid == TP_LIST ) \
        ) { \
        tp_obj meta; if (_tp_lookup(tp, self, tp_string_atom(tp, name),&meta)) {
#define TP_META_END \
        } \
    }

