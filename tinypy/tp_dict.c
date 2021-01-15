tp_obj tp_dict_nt(TP) {
    tp_obj r = {TP_DICT};
    r.type.magic = TP_DICT_RAW;
    r.info = tpd_dict_new(tp);
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

/* Function: tp_object
 * Creates a new object.
 *
 * Returns:
 * The newly created object. The object initially has no parent class, use
 * <tp_setmeta> to set a class. Also see <tp_object_new>.
 */
tp_obj tp_object(TP) {
    tp_obj self = tp_dict_t(tp);
    self.type.magic = TP_DICT_OBJECT;
    tp_set_meta(tp, self, tp->object_class);
    return self;
}

/* Function: tp_class
 * Creates a new base class.
 *
 * Parameters:
 * none
 *
 * Returns:
 * A new, empty class (derived from tinypy's builtin "object" class).
 */
tp_obj tp_class(TP) {
    tp_obj klass = tp_dict_t(tp);
    klass.type.magic = TP_DICT_CLASS;
    tp_set_meta(tp, klass, tp->object_class);
    return klass;
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
    int n = tpd_dict_hashfind(tp, TPD_DICT(self), hash, k);

    if (n < 0) {
        tp_raise_printf(tp_None, "(tpd_dict_get) KeyError: %O (%d)", &k, hash);
    }

    return tpd_dict_get(tp, TPD_DICT(self), n);
}

int tp_dict_has(TP, tp_obj self, tp_obj k) {
    int hash = tp_hash(tp, k);
    int n = tpd_dict_hashfind(tp, TPD_DICT(self), hash, k);

    if (n < 0) {
        return 0;
    }
    return 1;
}

void tp_dict_del(TP, tp_obj self, tp_obj k) {
    int n = tpd_dict_hashfind(tp, TPD_DICT(self), tp_hash(tp, k), k);
    if (n < 0) {
        tp_raise_printf(, "(tpd_dict_del) KeyError: %O", &k);
    }
    tpd_dict_del(tp, TPD_DICT(self), n);
}

void tp_dict_set(TP, tp_obj self, tp_obj k, tp_obj v) {
    tpd_dict_hashsetx(tp, TPD_DICT(self), tp_hash(tp, k), k, v);
    tp_grey(tp, k);
    tp_grey(tp, v);
}

tp_obj tp_dict_copy(TP, tp_obj rr) {
    tp_obj obj = tp_dict_nt(tp);
    tpd_dict *o = TPD_DICT(rr);
    tpd_dict *r = TPD_DICT(obj);
    obj.type = rr.type;
    *r = *o;
    r->items = (tpd_item*) tp_malloc(tp, sizeof(tpd_item)*o->alloc);
    memcpy(r->items, o->items, sizeof(tpd_item)*o->alloc);
    return tp_track(tp, obj);
}

int tp_dict_equal(TP, tp_obj a, tp_obj b) {
    int i;
    if(TPD_DICT(a)->len != TPD_DICT(b)->len) {
        return 0;
    }
    for (i=0; i<TPD_DICT(a)->len; i++) {
        int na = tpd_dict_next(tp, TPD_DICT(a));
        tp_obj key = TPD_DICT(a)->items[na].key;
        tp_obj value = TPD_DICT(b)->items[na].val;
        int hash = tp_hash(tp, key);

        int nb = tpd_dict_hashfind(tp, TPD_DICT(b), hash, key);
        if (nb < 0) return 0;
        tp_obj bv = tpd_dict_get(tp, TPD_DICT(b), nb);
        if(!tp_equal(tp, value, bv)) return 0;
    }
    return 1;
}

void tp_dict_update(TP, tp_obj self, tp_obj v)
{
    int i;
    for (i=0; i<TPD_DICT(v)->len; i++) {
        int n = tpd_dict_next(tp, TPD_DICT(v));
        tp_dict_set(tp,
                self,
                TPD_DICT(v)->items[n].key,
                TPD_DICT(v)->items[n].val);
    }
}
