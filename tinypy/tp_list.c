/* C API for lists */
tp_obj tp_list_copy(TP, tp_obj rr) {
    tp_check_type(tp, TP_LIST, rr);

    tp_obj val = {TP_LIST};
    tpd_list *o = rr.list.val;
    tpd_list *r = tpd_list_new(tp);
    *r = *o; r->gci = 0;
    r->alloc = o->len;
    r->items = (tp_obj*)tp_malloc(tp, sizeof(tp_obj)*o->len);
    memcpy(r->items,o->items,sizeof(tp_obj)*o->len);
    val.list.val = r;
    return val;
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

tp_obj tp_list_nt(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = tpd_list_new(tp);
    return r;
}

int tp_list_cmp(TP, tp_obj a, tp_obj b)
{
    int n, v;
    for(n=0; n<_tp_min(a.list.val->len, b.list.val->len); n++) {
        tp_obj aa = a.list.val->items[n];
        tp_obj bb = b.list.val->items[n];
        v = tp_cmp(tp, aa, bb);
        if (v) { return v; }
    }
    return a.list.val->len - b.list.val->len;
}
