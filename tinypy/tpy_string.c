
tp_obj tpy_str_join(TP) {
    tp_obj delim = TP_OBJ();
    tp_obj val = TP_OBJ();
    int l=0,i;
    tp_obj r;
    char *s;
    for (i=0; i<val.list.val->len; i++) {
        if (i!=0) { l += delim.string.val->len; }
        l += tp_str(tp, val.list.val->items[i]).string.val->len;
    }
    r = tp_string_t(tp,l);
    s = r.string.val->s;
    l = 0;
    for (i=0; i<val.list.val->len; i++) {
        tp_obj e;
        if (i!=0) {
            memcpy(s+l,delim.string.val->s, delim.string.val->len); l += delim.string.val->len;
        }
        e = tp_str(tp, val.list.val->items[i]);
        memcpy(s+l,e.string.val->s,e.string.val->len); l += e.string.val->len;
    }
    return r;
}

tp_obj tpy_str_split(TP) {
    tp_obj v = TP_OBJ();
    tp_obj d = TP_OBJ();
    tp_obj r = tp_list_t(tp);

    v = tp_string_sub(tp, v, 0, v.string.val->len);

    int i;
    while ((i = tp_str_index(v, d))!=-1) {
        tpd_list_append(tp, r.list.val, tp_string_sub(tp, v, 0, i));
        v.string.val->s += i + d.string.val->len;
        v.string.val->len -= i + d.string.val->len;
    }
    tpd_list_append(tp, r.list.val, tp_string_sub(tp, v, 0, v.string.val->len));
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
    if (s.string.val->len != 1) {
        tp_raise(tp_None,tp_string_atom(tp, "(tp_ord) TypeError: ord() expected a character"));
    }
    return tp_number((unsigned char)s.string.val->s[0]);
}

tp_obj tpy_str_strip(TP) {
    tp_obj o = TP_TYPE(TP_STRING);
    char const *v = o.string.val->s; int l = o.string.val->len;
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
    s = r.string.val->s;
    memcpy(s,v+a,b-a);
    return r;
}

tp_obj tpy_str_replace(TP) {
    tp_obj s = TP_OBJ();
    tp_obj k = TP_OBJ();
    tp_obj v = TP_OBJ();
    tp_obj p = tp_string_sub(tp, s, 0, s.string.val->len);
    int i,n = 0;
    int c;
    int l;
    tp_obj rr;
    char *r;
    char *d;
    tp_obj z;
    while ((i = tp_str_index(p,k)) != -1) {
        n += 1;
        p.string.val->s += i + k.string.val->len;
        p.string.val->len -= i + k.string.val->len;
    }
/*     fprintf(stderr,"ns: %d\n",n); */
    l = s.string.val->len + n * (v.string.val->len-k.string.val->len);
    rr = tp_string_t(tp, l);
    r = rr.string.val->s;
    d = r;
    z = p = s;
    while ((i = tp_str_index(p,k)) != -1) {
        p.string.val->s += i; p.string.val->len -= i;
        memcpy(d,z.string.val->s,c=(p.string.val->s-z.string.val->s)); d += c;
        p.string.val->s += k.string.val->len; p.string.val->len -= k.string.val->len;
        memcpy(d,v.string.val->s,v.string.val->len); d += v.string.val->len;
        z = p;
    }
    memcpy(d,z.string.val->s,(s.string.val->s + s.string.val->len) - z.string.val->s);

    return rr;
}
