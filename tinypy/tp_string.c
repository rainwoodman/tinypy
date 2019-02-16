/* File: String
 * String handling functions.
 */
 
/*
 * Create a new empty string of a certain size.
 */ 
tp_obj tp_string_t(TP, int n) {
    tp_obj r;
    r.type.typeid = TP_STRING;
    r.type.magic = TP_STRING_NONE;
    r.string.info = (tpd_string*)tp_malloc(tp, sizeof(tpd_string));
    r.string.info->len = n;
    r.string.info->s = tp_malloc(tp, n);
    r.obj.info->meta = tp->_string_meta;
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
    r.type.typeid = TP_STRING;
    r.type.magic = TP_STRING_ATOM;
    r.string.info = (tpd_string*) tp_malloc(tp, sizeof(tpd_string));
    r.string.info->base = tp_None;
    r.string.info->s = (char*) s;
    r.string.info->len = n;
    r.obj.info->meta = tp->_string_meta;
    return tp_track(tp, r);
}
/*
 * Create a new string which is a copy of some memory.
 */
tp_obj tp_string_from_buffer(TP, const char *s, int n) {
    if(n < 0) n = strlen(s);
    tp_obj r = tp_string_t(tp, n);
    memcpy(tp_string_getptr(r), s, n);
    return r;
}

char * tp_string_getptr(tp_obj s) {
    return s.string.info->s;
}

int tp_string_len(tp_obj s) {
    /* will skip info for atoms */
    return s.string.info->len;
}

/*
 * Create a new string which is a substring slice (view) of another STRING.
 * the returned object does not allocate new memory. It refers to the same
 * memory object to the original string.
 */
tp_obj tp_string_view(TP, tp_obj s, int a, int b) {
    int l = tp_string_len(s);
    a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
    tp_obj r = tp_string_from_const(tp, tp_string_getptr(s) + a, b - a);
    r.string.info->base = s;
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
    s = tp_string_getptr(r);
    va_end(arg);
    va_start(arg, fmt);
    vsnprintf(s, l + 1, fmt, arg);
    va_end(arg);
    return r;
}

int tp_str_index (tp_obj s, tp_obj k) {
    int i=0;
    while ((tp_string_len(s) - i) >= tp_string_len(k)) {
        if (memcmp(tp_string_getptr(s) + i,
                   tp_string_getptr(k),
                   tp_string_len(k)) == 0) {
            return i;
        }
        i += 1;
    }
    return -1;
}


int tp_string_cmp(tp_obj a, tp_obj b)
{
    int l = _tp_min(tp_string_len(a), tp_string_len(b));
    int v = memcmp(tp_string_getptr(a), tp_string_getptr(b), l);
    if (v == 0) {
        v = tp_string_len(a) - tp_string_len(b);
    }
    return v;
}

tp_obj tp_string_add(TP, tp_obj a, tp_obj b)
{
    int al = tp_string_len(a), bl = tp_string_len(b);
    tp_obj r = tp_string_t(tp, al+bl);
    char *s = tp_string_getptr(r);
    memcpy(s, tp_string_getptr(a), al);
    memcpy(s + al, tp_string_getptr(b), bl);
    return r;
}

tp_obj tp_string_mul(TP, tp_obj a, int n)
{
    int al = tp_string_len(a);
    if(n <= 0) {
        tp_obj r = tp_string_t(tp, 0);
        return r;
    }
    tp_obj r = tp_string_t(tp, al*n);
    char *s = tp_string_getptr(r);
    int i;
    for (i=0; i<n; i++) {
        memcpy(s+al*i, tp_string_getptr(a), al);
    }
    return r;
}
