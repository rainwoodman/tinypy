tp_obj tpy_dict_merge(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i; for (i=0; i<v.dict.val->len; i++) {
        int n = tpd_dict_next(tp, v.dict.val);
        tpd_dict_set(tp,
                self.dict.val,
                v.dict.val->items[n].key,
                v.dict.val->items[n].val);
    }
    return tp_None;
}

/* Function: tp_dict
 *
 * Creates a new dictionary object.
 *
 * *Note* If you use <tp_setmeta> on the dictionary, you have to use <tp_getraw> to
 * access the "raw" dictionary again.
 *
 * Returns:
 * The newly created dictionary.
 */
tp_obj tpy_dict(TP) {
    tp_obj r = {TP_DICT};
    r.dict.val = tpd_dict_new(tp);
    r.dict.dtype = 1;
    return tp ? tp_track(tp,r) : r;
}

tp_obj tpy_dict_n(TP, int n, tp_obj* argv) {
    tp_obj r = tpy_dict(tp);
    int i; for (i=0; i<n; i++) {
        tp_set(tp,r,argv[i*2],argv[i*2+1]);
    }
    return r;
}

