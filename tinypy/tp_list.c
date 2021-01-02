tp_obj tp_list_nt(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = tpd_list_new(tp);
    return r;
}

tp_obj tp_list_t(TP) {
    return tp_track(tp, tp_list_nt(tp));
}

tp_obj tp_list_from_items(TP, int n, tp_obj *argv) {
    int i;
    tp_obj r = tp_list_t(tp);
    tpd_list_realloc(tp, r.list.val,n);
    for (i=0; i<n; i++) {
        tpd_list_append(tp, r.list.val, argv[i]);
    }
    return r;
}

tp_obj tp_list_v(TP,int n,...) {
    int i;
    tp_obj r = tp_list_t(tp);
    va_list a; va_start(a,n);
    for (i=0; i<n; i++) {
        tpd_list_append(tp, r.list.val, va_arg(a, tp_obj));
    }
    va_end(a);
    return r;
}

/* C API for lists */
tp_obj tp_list_copy(TP, tp_obj rr) {
    tp_check_type(tp, TP_LIST, rr);

    tp_obj val = tp_list_nt(tp);
    tpd_list *o = rr.list.val;
    tpd_list *r = val.list.val;
    *r = *o;
    r->alloc = o->len;
    r->items = (tp_obj*)tp_malloc(tp, sizeof(tp_obj)*o->len);
    memcpy(r->items,o->items,sizeof(tp_obj)*o->len);
    return tp_track(tp, val);
}

tp_obj tp_list_add(TP, tp_obj a, tp_obj b)
{
    tp_obj r;
    r = tp_list_copy(tp, a);
    tpd_list_extend(tp, r.list.val, b.list.val);
    return r;
}

tp_obj tp_list_mul(TP, tp_obj a, int n)
{
    tp_obj r;
    r = tp_list_copy(tp, a);
    int i;
    for (i = 1; i < n; i ++) {
        tpd_list_extend(tp, r.list.val, a.list.val);
    }
    return r;
}

int tp_list_equal(TP, tp_obj a, tp_obj b)
{
    int n, v;
    if(a.list.val->len != b.list.val->len) {
        return 0;
    }
    for(n=0; n<a.list.val->len; n++) {
        tp_obj aa = a.list.val->items[n];
        tp_obj bb = b.list.val->items[n];
        if(!tp_equal(tp, aa, bb)) return 0;
    }
    return 1;
}

int tp_list_lessthan(TP, tp_obj a, tp_obj b)
{
    int n, v;
    for(n=0; n<_tp_min(a.list.val->len, b.list.val->len); n++) {
        tp_obj aa = a.list.val->items[n];
        tp_obj bb = b.list.val->items[n];
        if(tp_equal(tp, aa, bb)) continue;
        if(tp_lessthan(tp, aa, bb)) {
            return 1;
        } else {
            return 0;
        }
    }
    return a.list.val->len < b.list.val->len;
}
