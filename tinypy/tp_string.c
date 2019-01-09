/* File: String
 * String handling functions.
 */
 
/*
 * Create a new empty string of a certain size.
 */ 
tp_obj tp_string_t(TP, int n) {
    tp_obj r;
    r.string.type.typeid = TP_STRING;
    r.string.type.magic = TP_STRING_NONE;
    r.string.val = (tpd_string*)tp_malloc(tp, sizeof(tpd_string));
    r.string.val->len = n;
    r.string.val->s = tp_malloc(tp, n);
    return tp_track(tp, r);
}

tp_obj tp_string_atom(TP, const char * v) {
    /* FIXME: use a hash table for the atoms to avoid the leak */
    return tp_string_from_const(tp, v, -1);
}

/*
 */
tp_obj tp_string_from_const(TP, const char *s, int n) {
    tp_obj r;
    if(n < 0) n = strlen(s);
    r.string.type.typeid = TP_STRING;
    r.string.type.magic = TP_STRING_ATOM;
    r.string.val = (tpd_string*) tp_malloc(tp, sizeof(tpd_string));
    r.string.val->base = tp_None;
    r.string.val->s = (char*) s;
    r.string.val->len = n;
    return tp_track(tp, r);
}
/*
 * Create a new string which is a copy of some memory.
 */
tp_obj tp_string_from_buffer(TP, const char *s, int n) {
    if(n < 0) n = strlen(s);
    tp_obj r = tp_string_t(tp, n);
    memcpy(r.string.val->s, s, n);
    return r;
}

/*
 * Create a new string which is a substring slice (view) of another STRING.
 * the returned object does not allocate new memory. It refers to the same
 * memory object to the original string.
 */
tp_obj tp_string_sub(TP, tp_obj s, int a, int b) {
    int l = s.string.val->len;
    a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
    tp_obj r = tp_string_from_const(tp, s.string.val->s + a, b - a);
    r.string.val->base = s;
    r.type.magic = TP_STRING_VIEW;
    return r;
}

tp_obj tp_printf(TP, char const *fmt,...) {
    int l;
    tp_obj r;
    char *s;
    va_list arg;
    va_start(arg, fmt);
    l = vsnprintf(NULL, 0, fmt,arg);
    r = tp_string_t(tp, l + 1);
    s = r.string.val->s;
    va_end(arg);
    va_start(arg, fmt);
    vsnprintf(s, l + 1, fmt, arg);
    va_end(arg);
    return r;
}

int tp_str_index (tp_obj s, tp_obj k) {
    int i=0;
    while ((s.string.val->len - i) >= k.string.val->len) {
        if (memcmp(s.string.val->s+i,k.string.val->s,k.string.val->len) == 0) {
            return i;
        }
        i += 1;
    }
    return -1;
}


int tp_string_cmp(tp_obj * a, tp_obj * b)
{
    int l = _tp_min(a->string.val->len, b->string.val->len);
    int v = memcmp(a->string.val->s, b->string.val->s, l);
    if (v == 0) {
        v = a->string.val->len - b->string.val->len;
    }
    return v;
}

tp_obj tp_string_add(TP, tp_obj a, tp_obj b)
{
    int al = a.string.val->len, bl = b.string.val->len;
    tp_obj r = tp_string_t(tp, al+bl);
    char *s = r.string.val->s;
    memcpy(s,a.string.val->s,al);
    memcpy(s+al,b.string.val->s,bl);
    return r;
}

tp_obj tp_string_mul(TP, tp_obj a, int n)
{
    int al = a.string.val->len;
    if(n <= 0) {
        tp_obj r = tp_string_t(tp, 0);
        return r;
    }
    tp_obj r = tp_string_t(tp, al*n);
    char *s = r.string.val->s;
    int i;
    for (i=0; i<n; i++) {
        memcpy(s+al*i, a.string.val->s, al);
    }
    return r;
}
