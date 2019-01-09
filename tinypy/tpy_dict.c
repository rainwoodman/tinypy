tp_obj tpy_dict_merge(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i;
    for (i=0; i<v.dict.val->len; i++) {
        int n = tpd_dict_next(tp, v.dict.val);
        tp_dict_set(tp,
                self,
                v.dict.val->items[n].key,
                v.dict.val->items[n].val);
    }
    return tp_None;
}

