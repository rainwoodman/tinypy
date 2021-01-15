
tp_obj tpy_str_join(TP) {
    tp_obj delim = TP_PARAMS_OBJ();
    tp_obj val = TP_PARAMS_OBJ();
    StringBuilder sb[1] = {tp};

    int l=0,i;
    tp_obj r;
    char *s;
    for (i=0; i<TPD_LIST(val)->len; i++) {
        if (i!=0) { l += tp_string_len(delim); }
        l += tp_string_len(tp_str(tp, TPD_LIST(val)->items[i]));
    }
    r = tp_string_t(tp,l);
    s = tp_string_getptr(r);
    l = 0;
    for (i=0; i<TPD_LIST(val)->len; i++) {
        tp_obj e;
        if (i!=0) {
            string_builder_write(sb, tp_string_getptr(delim), tp_string_len(delim));
        }
        e = tp_str(tp, TPD_LIST(val)->items[i]);
        tp_str_internal(tp, TPD_LIST(val)->items[i], sb, 1);
    }
    return tp_string_steal_from_builder(tp, sb);
}

tp_obj tpy_str_split(TP) {
    tp_obj v = TP_PARAMS_OBJ();
    tp_obj d = TP_PARAMS_OBJ();
    tp_obj r = tp_list_t(tp);

    v = tp_string_view(tp, v, 0, tp_string_len(v));

    int i;
    while ((i = tp_str_index(v, d))!=-1) {
        tpd_list_append(tp, TPD_LIST(r), tp_string_view(tp, v, 0, i));
        TPD_STRING(v)->s += i + tp_string_len(d);
        TPD_STRING(v)->len -= i + tp_string_len(d);
    }
    tpd_list_append(tp, TPD_LIST(r), tp_string_view(tp, v, 0, tp_string_len(v)));
    return r;
}


tp_obj tpy_str_find(TP) {
    tp_obj s = TP_PARAMS_OBJ();
    tp_obj v = TP_PARAMS_OBJ();
    return tp_int(tp_str_index(s,v));
}

tp_obj tpy_str_index(TP) {
    tp_obj s = TP_PARAMS_OBJ();
    tp_obj v = TP_PARAMS_OBJ();
    int n = tp_str_index(s,v);
    if (n >= 0) { return tp_int(n); }
    tp_raise(tp_None,tp_string_atom(tp, "(tp_str_index) ValueError: substring not found"));
}

tp_obj tpy_chr(TP) {
    int v = TP_PARAMS_INT();
    return tp->chars[(unsigned char)v];
}

tp_obj tpy_ord(TP) {
    tp_obj s = TP_PARAMS_STR();
    if (tp_string_len(s) != 1) {
        tp_raise(tp_None,tp_string_atom(tp, "(tp_ord) TypeError: ord() expected a character"));
    }
    return tp_int((unsigned char)tp_string_getptr(s)[0]);
}

// NOTE: tpy strings are all byte strings, like py2. Thus encode is a noop.
tp_obj tpy_str_encode(TP) {
    tp_obj o = TP_PARAMS_TYPE(TP_STRING);
    return o;
}

tp_obj tpy_str_strip(TP) {
    tp_obj o = TP_PARAMS_TYPE(TP_STRING);
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
    tp_obj s = TP_PARAMS_OBJ();
    tp_obj k = TP_PARAMS_OBJ();
    tp_obj v = TP_PARAMS_OBJ();
    StringBuilder sb[1] = {tp};

    char * ss = tp_string_getptr(s);
    char * se = ss + tp_string_len(s);
    char * ks = tp_string_getptr(k);
    char * ke = ks + tp_string_len(k);
    char * vs = tp_string_getptr(v);
    char * ve = vs + tp_string_len(v);

    char * sp, * kp;
    kp = ks;
    for(sp = ss; sp < se; sp++) {
        if(*sp == *kp) {
            kp++;
            if(kp == ke) {
                /* found the pattern, write the replacement */
                string_builder_write(sb, vs, ve - vs);
                kp = ks;
            }
            continue;
        }
        kp = ks;
        string_builder_write(sb, sp, 1);
    }
    return tp_string_steal_from_builder(tp, sb);
}
