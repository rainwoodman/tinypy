
tp_obj tpy_str_join(TP) {
    tp_obj delim = TP_OBJ();
    tp_obj val = TP_OBJ();
    int l=0,i;
    tp_obj r;
    char *s;
    for (i=0; i<val.list.val->len; i++) {
        if (i!=0) { l += delim.string.info->len; }
        l += tp_str(tp, val.list.val->items[i]).string.info->len;
    }
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    l = 0;
    for (i=0; i<val.list.val->len; i++) {
        tp_obj e;
        if (i!=0) {
            memcpy(s+l,delim.string.info->s, delim.string.info->len); l += delim.string.info->len;
        }
        e = tp_str(tp, val.list.val->items[i]);
        memcpy(s+l,e.string.info->s,e.string.info->len); l += e.string.info->len;
    }
    return r;
}

tp_obj tpy_str_split(TP) {
    tp_obj v = TP_OBJ();
    tp_obj d = TP_OBJ();
    tp_obj r = tp_list_t(tp);

    v = tp_string_view(tp, v, 0, v.string.info->len);

    int i;
    while ((i = tp_str_index(v, d))!=-1) {
        tpd_list_append(tp, r.list.val, tp_string_view(tp, v, 0, i));
        v.string.info->s += i + d.string.info->len;
        v.string.info->len -= i + d.string.info->len;
    }
    tpd_list_append(tp, r.list.val, tp_string_view(tp, v, 0, v.string.info->len));
    return r;
}


tp_obj tpy_find(TP) {
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
    /* returns an untracked string, since we do not own the memory. */
    return tp_string_from_const(tp, tp->chars[(unsigned char)v], 1);
}
tp_obj tpy_ord(TP) {
    tp_obj s = TP_STR();
    if (s.string.info->len != 1) {
        tp_raise(tp_None,tp_string_atom(tp, "(tp_ord) TypeError: ord() expected a character"));
    }
    return tp_number((unsigned char)s.string.info->s[0]);
}

tp_obj tpy_str_strip(TP) {
    tp_obj o = TP_TYPE(TP_STRING);
    char const *v = o.string.info->s; int l = o.string.info->len;
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
    s = r.string.info->s;
    memcpy(s,v+a,b-a);
    return r;
}

tp_obj tpy_str_replace(TP) {
    tp_obj s = TP_OBJ();
    tp_obj k = TP_OBJ();
    tp_obj v = TP_OBJ();
    tp_obj p = tp_string_view(tp, s, 0, s.string.info->len);
    int i,n = 0;
    int c;
    int l;
    tp_obj rr;
    char *r;
    char *d;
    tp_obj z;
    while ((i = tp_str_index(p,k)) != -1) {
        n += 1;
        p.string.info->s += i + k.string.info->len;
        p.string.info->len -= i + k.string.info->len;
    }
/*     fprintf(stderr,"ns: %d\n",n); */
    l = s.string.info->len + n * (v.string.info->len-k.string.info->len);
    rr = tp_string_t(tp, l);
    r = rr.string.info->s;
    d = r;
    z = p = s;
    while ((i = tp_str_index(p,k)) != -1) {
        p.string.info->s += i; p.string.info->len -= i;
        memcpy(d,z.string.info->s,c=(p.string.info->s-z.string.info->s)); d += c;
        p.string.info->s += k.string.info->len; p.string.info->len -= k.string.info->len;
        memcpy(d,v.string.info->s,v.string.info->len); d += v.string.info->len;
        z = p;
    }
    memcpy(d,z.string.info->s,(s.string.info->s + s.string.info->len) - z.string.info->s);

    return rr;
}
