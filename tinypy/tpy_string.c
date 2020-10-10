
/* FIXME: use StringBuilder. */
tp_obj tpy_str_join(TP) {
    tp_obj delim = TP_OBJ();
    tp_obj val = TP_OBJ();
    StringBuilder sb[1] = {0};

    int l=0,i;
    tp_obj r;
    char *s;
    for (i=0; i<val.list.val->len; i++) {
        if (i!=0) { l += tp_string_len(delim); }
        l += tp_string_len(tp_str(tp, val.list.val->items[i]));
    }
    r = tp_string_t(tp,l);
    s = tp_string_getptr(r);
    l = 0;
    for (i=0; i<val.list.val->len; i++) {
        tp_obj e;
        if (i!=0) {
            string_builder_write(tp, sb, tp_string_getptr(delim), tp_string_len(delim));
        }
        e = tp_str(tp, val.list.val->items[i]);
        tp_str_internal(tp, val.list.val->items[i], sb, 1);
    }
    return tp_string_steal_from_builder(tp, sb);
}

tp_obj tpy_str_split(TP) {
    tp_obj v = TP_OBJ();
    tp_obj d = TP_OBJ();
    tp_obj r = tp_list_t(tp);

    v = tp_string_view(tp, v, 0, tp_string_len(v));

    int i;
    while ((i = tp_str_index(v, d))!=-1) {
        tpd_list_append(tp, r.list.val, tp_string_view(tp, v, 0, i));
        v.string.info->s += i + tp_string_len(d);
        v.string.info->len -= i + tp_string_len(d);
    }
    tpd_list_append(tp, r.list.val, tp_string_view(tp, v, 0, tp_string_len(v)));
    return r;
}


tp_obj tpy_str_find(TP) {
    tp_obj s = TP_OBJ();
    tp_obj v = TP_OBJ();
    return tp_number(tp_str_index(s,v));
}

tp_obj tpy_str_index(TP) {
    tp_obj s = TP_OBJ();
    tp_obj v = TP_OBJ();
    int n = tp_str_index(s,v);
    if (n >= 0) { return tp_number(n); }
    tp_raise(tp_None,tp_string_atom(tp, "(tp_str_index) ValueError: substring not found"));
}

tp_obj tpy_chr(TP) {
    int v = TP_NUM();
    return tp_string_t_from_const(tp, tp->chars[(unsigned char)v], 1);
}
tp_obj tpy_ord(TP) {
    tp_obj s = TP_STR();
    if (tp_string_len(s) != 1) {
        tp_raise(tp_None,tp_string_atom(tp, "(tp_ord) TypeError: ord() expected a character"));
    }
    return tp_number((unsigned char)tp_string_getptr(s)[0]);
}

// NOTE: tpy strings are all byte strings, like py2. Thus encode is a noop.
tp_obj tpy_str_encode(TP) {
    tp_obj o = TP_TYPE(TP_STRING);
    return o;
}

tp_obj tpy_str_strip(TP) {
    tp_obj o = TP_TYPE(TP_STRING);
    char const *v = tp_string_getptr(o); int l = tp_string_len(o);
    int i; int a = l, b = 0;
    tp_obj r;
    char *s;
    for (i=0; i<l; i++) {
        if (v[i] != ' ' && v[i] != '\n' && v[i] != '\t' && v[i] != '\r') {
            a = _tp_min(a,i); b = _tp_max(b,i+1);
        }
    }
    if ((b-a) < 0) { return tp_string_atom(tp, ""); }
    r = tp_string_t(tp,b-a);
    s = tp_string_getptr(r);
    memcpy(s,v+a,b-a);
    return r;
}

tp_obj tpy_str_replace(TP) {
    tp_obj s = TP_OBJ();
    tp_obj k = TP_OBJ();
    tp_obj v = TP_OBJ();
    tp_obj p = tp_string_view(tp, s, 0, tp_string_len(s));
    int i,n = 0;
    int c;
    int l;
    tp_obj rr;
    char *r;
    char *d;
    tp_obj z;
    while ((i = tp_str_index(p,k)) != -1) {
        n += 1;
        p.string.info->s += i + tp_string_len(k);
        p.string.info->len -= i + tp_string_len(k);
    }
/*     fprintf(stderr,"ns: %d\n",n); */
    l = tp_string_len(s) + n * (tp_string_len(v)-tp_string_len(k));
    rr = tp_string_t(tp, l);
    r = tp_string_getptr(rr);
    d = r;
    z = p = s;
    while ((i = tp_str_index(p,k)) != -1) {
        p.string.info->s += i;
        p.string.info->len -= i;
        memcpy(d,tp_string_getptr(z), c=(tp_string_getptr(p) - tp_string_getptr(z))); d += c;
        p.string.info->s += tp_string_len(k);
        p.string.info->len -= tp_string_len(k);
        memcpy(d,tp_string_getptr(v), tp_string_len(v)); d += tp_string_len(v);
        z = p;
    }
    memcpy(d, tp_string_getptr(z), (tp_string_getptr(s) + tp_string_len(s)) - tp_string_getptr(z));

    return rr;
}
