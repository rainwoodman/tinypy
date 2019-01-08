
int tpd_dict_find(TP, tpd_dict * self, tp_obj k) {
    return tpd_dict_hashfind(tp, self, tp_hash(tp, k), k);
}

void tpd_dict_setx(TP, tpd_dict * self, tp_obj k, tp_obj v) {
    int hash = tp_hash(tp, k);
    int n = tpd_dict_hashfind(tp, self, hash, k);
    if (n == -1) {
        if (self->len >= (self->alloc/2)) {
            tpd_dict_realloc(tp, self, self->alloc*2);
        } else if (self->used >= (self->alloc*3/4)) {
            tpd_dict_realloc(tp, self, self->alloc);
        }
        tpd_dict_hashset(tp, self, hash, k, v);
    } else {
        self->items[n].val = v;
    }
}


void tpd_dict_set(TP, tpd_dict *self, tp_obj k, tp_obj v) {
    tpd_dict_setx(tp, self, k, v);
    tp_grey(tp,k);
    tp_grey(tp, v);
}

tp_obj tpd_dict_get(TP, tpd_dict *self, tp_obj k, const char *error) {
    int n = tpd_dict_find(tp, self, k);
    if (n < 0) {
        tp_raise(tp_None,
            tp_add(tp, tp_string("(tpd_dict_get) KeyError: "), tp_str(tp,k))
        );
    }
    return self->items[n].val;
}

void tpd_dict_del(TP, tpd_dict * self, tp_obj k, const char *error) {
    int n = tpd_dict_find(tp, self, k);
    if (n < 0) {
        tp_raise(, tp_add(tp, tp_string("(tpd_dict_del) KeyError: "), tp_str(tp, k)));
    }
    self->items[n].used = -1;
    self->len -= 1;
}

tp_obj tp_dict_copy(TP, tp_obj rr) {
    tp_obj obj = {TP_DICT};
    tpd_dict *o = rr.dict.val;
    tpd_dict *r = tpd_dict_new(tp);
    *r = *o;
    r->gci = 0;
    r->items = (tp_item*) tp_malloc(tp, sizeof(tp_item)*o->alloc);
    memcpy(r->items, o->items, sizeof(tp_item)*o->alloc);
    obj.dict.val = r;
    obj.dict.dtype = 1;
    return obj;
}

