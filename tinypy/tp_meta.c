tp_obj tp_get_meta(TP, tp_obj self) {
    if(self.type.typeid > TP_HAS_META) {
        return self.obj.info->meta;
    }
    return tp_None;
}

int _tp_lookup_(TP, tp_obj self, int hash, tp_obj k, tp_obj *r, int depth) {
    /* first do a dict look up from the object itself, but never look for values from a raw dict. */
    if(self.type.typeid == TP_DICT && self.type.magic != TP_DICT_RAW) {
        int n = tpd_dict_hashfind(tp, self.dict.val, hash, k);
        if (n != -1) {
            *r= self.dict.val->items[n].val;
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

    tp_obj meta = tp_get_meta(tp, self);

    if (tp_none(meta)) {
        return 0;
    }

    if (meta.type.typeid == TP_DICT &&
        _tp_lookup_(tp, meta, hash, k, r, depth)) {
        if ( r->type.typeid == TP_FUNC && 0 == (r->type.magic & TP_FUNC_MASK_STATIC)) {
            /* object dict or string, or list */
            if ((self.type.typeid == TP_DICT && self.type.magic != TP_DICT_CLASS)
                || self.type.typeid == TP_LIST
                || self.type.typeid == TP_STRING
            ) {
                *r = tp_bind(tp, *r, self);
            }
        }
        return 1;
    }
    return 0;
}

int _tp_lookup(TP, tp_obj self, tp_obj k, tp_obj *r) {
    return _tp_lookup_(tp, self, tp_hash(tp, k), k, r, 8);
}

#define TP_META_BEGIN(self, name) \
    if ((self.type.typeid == TP_DICT && self.type.magic == TP_DICT_OBJECT) || \
        (self.type.typeid == TP_STRING || self.type.typeid == TP_LIST ) \
        ) { \
        tp_obj name; if (_tp_lookup(tp, self, tp_string_atom(tp, #name), &name)) {
#define TP_META_END \
        } \
    }

#define TP_META_BEGIN_CLASS(self, name) \
    if ((self.type.typeid == TP_DICT && self.type.magic == TP_DICT_CLASS) \
        ) { \
        tp_obj name; if (_tp_lookup(tp, self, tp_string_atom(tp, #name), &name)) {
#define TP_META_END_CLASS \
        } \
    }

