tp_obj tp_dict_nt(TP) {
    tp_obj r = {TP_DICT};
    r.dict.val = tpd_dict_new(tp);
    r.type.magic = TP_DICT_CLASS;
    r.obj.info->meta = tp->_dict_meta;
    return r;
}

/* Function: tp_dict_t
 *
 * Creates a new dictionary object.
 *
 * *Note* If you use <tp_setmeta> on the dictionary, you have to use <tp_getraw> to
 * access the "raw" dictionary again.
 *
 * Returns:
 * The newly created dictionary.
 */

tp_obj tp_dict_t(TP) {
    return tp_track(tp, tp_dict_nt(tp));
}

tp_obj tp_rawdict_t(TP) {
    tp_obj r = {TP_DICT};
    r.dict.val = tpd_dict_new(tp);
    r.type.magic = TP_DICT_RAW;
    return r;
}


tp_obj tp_dict_from_items (TP, int n, tp_obj * argv) {
    tp_obj r = tp_dict_t(tp);
    int i; for (i=0; i<n; i++) {
        tp_set(tp, r, argv[i*2], argv[i*2+1]);
    }
    return r;
}

tp_obj tp_dict_get(TP, tp_obj self, tp_obj k) {
    int hash = tp_hash(tp, k);
    int n = tpd_dict_hashfind(tp, self.dict.val, hash, k);

    if (n < 0) {
        char * str = tp_cstr(tp, k);
        tp_obj message = tp_printf(tp, "(tpd_dict_get) KeyError: %s (%d)", str, hash);
        tp_free(tp, str);
        tp_raise(tp_None, message);
    }

    return tpd_dict_get(tp, self.dict.val, n);
}

int tp_dict_has(TP, tp_obj self, tp_obj k) {
    int hash = tp_hash(tp, k);
    int n = tpd_dict_hashfind(tp, self.dict.val, hash, k);

    if (n < 0) {
        return 0;
    }
    return 1;
}

void tp_dict_del(TP, tp_obj self, tp_obj k) {
    int n = tpd_dict_hashfind(tp, self.dict.val, tp_hash(tp, k), k);
    if (n < 0) {
        char * str = tp_cstr(tp, k);
        tp_obj message = tp_printf(tp, "(tpd_dict_del) KeyError: %s", str);
        tp_free(tp, str);
        tp_raise(, message);
    }
    tpd_dict_del(tp, self.dict.val, n);
}

void tp_dict_set(TP, tp_obj self, tp_obj k, tp_obj v) {
    tpd_dict_hashsetx(tp, self.dict.val, tp_hash(tp, k), k, v);
    tp_grey(tp, k);
    tp_grey(tp, v);
}

tp_obj tp_dict_copy(TP, tp_obj rr) {
    tp_obj obj = {TP_DICT};
    tpd_dict *o = rr.dict.val;
    tpd_dict *r = tpd_dict_new(tp);
    *r = *o;
    r->gci = 0;
    r->items = (tpd_item*) tp_malloc(tp, sizeof(tpd_item)*o->alloc);
    memcpy(r->items, o->items, sizeof(tpd_item)*o->alloc);
    obj.dict.val = r;
    obj.type.magic = TP_DICT_CLASS;
    return obj;
}

