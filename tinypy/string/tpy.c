
tp_obj tpy_str_join(TP) {
    tp_obj delim = TP_OBJ();
    tp_obj val = TP_OBJ();
    int l=0,i;
    tp_obj r;
    char *s;
    for (i=0; i<val.list.val->len; i++) {
        if (i!=0) { l += delim.string.len; }
        l += tp_str(tp,val.list.val->items[i]).string.len;
    }
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    l = 0;
    for (i=0; i<val.list.val->len; i++) {
        tp_obj e;
        if (i!=0) {
            memcpy(s+l,delim.string.val,delim.string.len); l += delim.string.len;
        }
        e = tp_str(tp,val.list.val->items[i]);
        memcpy(s+l,e.string.val,e.string.len); l += e.string.len;
    }
    return tp_track(tp,r);
}

tp_obj tpy_str_split(TP) {
    tp_obj v = TP_OBJ();
    tp_obj d = TP_OBJ();
    tp_obj r = tpy_list(tp);

    int i;
    while ((i=_tp_str_index(v, d))!=-1) {
        tpd_list_append(tp, r.list.val, tp_string_sub(tp, v, 0, i));
        v.string.val += i + d.string.len; v.string.len -= i + d.string.len;
    }
    tpd_list_append(tp, r.list.val, tp_string_sub(tp, v, 0, v.string.len));
    return r;
}


tp_obj tpy_find(TP) {
    tp_obj s = TP_OBJ();
    tp_obj v = TP_OBJ();
    return tp_number(_tp_str_index(s,v));
}

tp_obj tpy_str_index(TP) {
    tp_obj s = TP_OBJ();
    tp_obj v = TP_OBJ();
    int n = _tp_str_index(s,v);
    if (n >= 0) { return tp_number(n); }
    tp_raise(tp_None,tp_string("(tp_str_index) ValueError: substring not found"));
}

tp_obj tpy_str2(TP) {
    tp_obj v = TP_OBJ();
    return tp_str(tp, v);
}

tp_obj tp_repr(TP, tp_obj self);

tp_obj tpy_repr(TP) {
    tp_obj v = TP_OBJ();
    return tp_repr(tp, v);
}

tp_obj tpy_chr(TP) {
    int v = TP_NUM();
    /* returns an untracked string, since we do not own the memory. */
    return tp_string_n(tp->chars[(unsigned char)v],1);
}
tp_obj tpy_ord(TP) {
    tp_obj s = TP_STR();
    if (s.string.len != 1) {
        tp_raise(tp_None,tp_string("(tp_ord) TypeError: ord() expected a character"));
    }
    return tp_number((unsigned char)s.string.val[0]);
}

tp_obj tpy_str_strip(TP) {
    tp_obj o = TP_TYPE(TP_STRING);
    char const *v = o.string.val; int l = o.string.len;
    int i; int a = l, b = 0;
    tp_obj r;
    char *s;
    for (i=0; i<l; i++) {
        if (v[i] != ' ' && v[i] != '\n' && v[i] != '\t' && v[i] != '\r') {
            a = _tp_min(a,i); b = _tp_max(b,i+1);
        }
    }
    if ((b-a) < 0) { return tp_string(""); }
    r = tp_string_t(tp,b-a);
    s = r.string.info->s;
    memcpy(s,v+a,b-a);
    return tp_track(tp,r);
}

tp_obj tpy_str_replace(TP) {
    tp_obj s = TP_OBJ();
    tp_obj k = TP_OBJ();
    tp_obj v = TP_OBJ();
    tp_obj p = s;
    int i,n = 0;
    int c;
    int l;
    tp_obj rr;
    char *r;
    char *d;
    tp_obj z;
    while ((i = _tp_str_index(p,k)) != -1) {
        n += 1;
        p.string.val += i + k.string.len; p.string.len -= i + k.string.len;
    }
/*     fprintf(stderr,"ns: %d\n",n); */
    l = s.string.len + n * (v.string.len-k.string.len);
    rr = tp_string_t(tp,l);
    r = rr.string.info->s;
    d = r;
    z = p = s;
    while ((i = _tp_str_index(p,k)) != -1) {
        p.string.val += i; p.string.len -= i;
        memcpy(d,z.string.val,c=(p.string.val-z.string.val)); d += c;
        p.string.val += k.string.len; p.string.len -= k.string.len;
        memcpy(d,v.string.val,v.string.len); d += v.string.len;
        z = p;
    }
    memcpy(d,z.string.val,(s.string.val + s.string.len) - z.string.val);

    return tp_track(tp,rr);
}
