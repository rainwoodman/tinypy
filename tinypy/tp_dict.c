tp_obj tp_dict_get(TP, tp_obj self, tp_obj k) {
    int hash = tp_hash(tp, k);
    int n = tpd_dict_hashfind(tp, self.dict.val, hash, k);

    if (n < 0) {
        char * str = tp_cstr(tp, k);
        tp_obj message = tp_printf_tracked(tp, "(tpd_dict_del) KeyError: %s", str);
        tp_free(tp, str);
        tp_raise(tp_None, message);
    }

    return tpd_dict_get(tp, self.dict.val, n);
}

void tp_dict_del(TP, tp_obj self, tp_obj k) {
    int n = tpd_dict_hashfind(tp, self.dict.val, tp_hash(tp, k), k);
    if (n < 0) {
        char * str = tp_cstr(tp, k);
        tp_obj message = tp_printf_tracked(tp, "(tpd_dict_del) KeyError: %s", str);
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
    obj.dict.dtype = 1;
    return obj;
}

