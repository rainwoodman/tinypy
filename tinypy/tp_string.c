/* File: String
 * String handling functions.
 */
 
/*
 * Create a new empty string of a certain size.
 * Does not put it in for GC tracking, since contents should be
 * filled after returning.
 */ 
tp_obj tp_string_t(TP, int n) {
    tp_obj r = tp_string_n(0,n);
    r.string.info = (tpd_string*)tp_malloc(tp, sizeof(tpd_string)+n);
    r.string.info->len = n;
    r.string.val = r.string.info->s;
    return r;
}

/*
 * Create a new string which is a copy of some memory.
 * This does not put into GC tracking for you.
 */
tp_obj tp_string_copy(TP, const char *s, int n) {
    tp_obj r = tp_string_t(tp,n);
    memcpy(r.string.info->s,s,n);
    return r;
}

/*
 * Create a new string which is a substring slice (view) of another STRING.
 * the returned object does not allocate new memory, and thus the object does not
 * need to be tracked by gc.
 */
tp_obj tp_string_sub(TP, tp_obj s, int a, int b) {
    int l = s.string.len;
    a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
    tp_obj r = s;
    r.string.val += a;
    r.string.len = b-a;
    return r;
}

tp_obj tp_printf_tracked(TP, char const *fmt,...) {
    int l;
    tp_obj r;
    char *s;
    va_list arg;
    va_start(arg, fmt);
    l = vsnprintf(NULL, 0, fmt,arg);
    r = tp_string_t(tp, l + 1);
    s = r.string.info->s;
    va_end(arg);
    va_start(arg, fmt);
    vsnprintf(s, l + 1, fmt, arg);
    va_end(arg);
    return tp_track(tp, r);
}

int _tp_str_index(tp_obj s, tp_obj k) {
    int i=0;
    while ((s.string.len - i) >= k.string.len) {
        if (memcmp(s.string.val+i,k.string.val,k.string.len) == 0) {
            return i;
        }
        i += 1;
    }
    return -1;
}


int _tp_string_cmp(tp_obj * a, tp_obj * b)
{
    int l = _tp_min(a->string.len, b->string.len);
    int v = memcmp(a->string.val, b->string.val, l);
    if (v == 0) {
        v = a->string.len - b->string.len;
    }
    return v;
}

tp_obj tp_string_add(TP, tp_obj a, tp_obj b)
{
    int al = a.string.len, bl = b.string.len;
    tp_obj r = tp_string_t(tp,al+bl);
    char *s = r.string.info->s;
    memcpy(s,a.string.val,al); memcpy(s+al,b.string.val,bl);
    return r;
}

tp_obj tp_string_mul(TP, tp_obj a, int n)
{
    int al = a.string.len;
    if(n <= 0) {
        tp_obj r = tp_string_t(tp, 0);
        return tp_track(tp, r);
    }
    tp_obj r = tp_string_t(tp, al*n);
    char *s = r.string.info->s;
    int i;
    for (i=0; i<n; i++) {
        memcpy(s+al*i, a.string.val, al);
    }
    return r;
}
