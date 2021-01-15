#define MINI_PRINTF_ENABLE_OBJECTS
#include "printf/mini-printf.c"

typedef struct {
    TP;
    char * buffer;
    int size;
    int len;
} StringBuilder;

void string_builder_write(StringBuilder * sb, const char * s, int len)
{
    TP = sb->tp;
    if(sb->buffer == NULL) {
        sb->buffer = tp_malloc(tp, 128);
        sb->len = 0;
        sb->size = 128;
    }
    if(len < 0) len = strlen(s);
    if(sb->len + len + 1 >= sb->size) {
        sb->size = (sb->len + len + 1) + sb->len / 2;
        sb->buffer = tp_realloc(tp, sb->buffer, sb->size);
    }
    memcpy(sb->buffer + sb->len, s, len);
    sb->len += len;
    sb->buffer[sb->len] = 0;
}

void string_builder_echo(StringBuilder * sb, tp_obj o)
{
    TP = sb->tp;
    o = tp_str(tp, o);
    string_builder_write(sb, tp_string_getptr(o), tp_string_len(o));
}


/* File: String
 * String handling functions.
 */
 
/*
 * Create a new empty string of a certain size.
 */ 
tp_obj tp_string_t(TP, int n) {
    tp_obj r;
    r.type.typeid = TP_STRING;
    r.type.magic = TP_STRING_NORMAL;
    r.info = tp_malloc(tp, sizeof(tpd_string));
    TPD_STRING(r)->len = n;
    TPD_STRING(r)->s = tp_malloc(tp, n);
    return tp_track(tp, r);
}

tp_obj tp_string_atom(TP, const char * v) {
    /* FIXME: use a hash table for the atoms to avoid the leak */
    static tpd_string info = {0};
    tp_obj r;
    r.type.typeid = TP_STRING;
    r.type.magic = TP_STRING_ATOM;
    r.info = &info;
    r.ptr = (char*) v;
    return r;
}

/*
 * Return a untracked string object from external memory.
 */
tp_obj tp_string_from_const_nt(TP, const char *s, int n) {
    tp_obj r;
    if(n < 0) n = strlen(s);
    r.type.typeid = TP_STRING;
    r.type.magic = TP_STRING_EXTERN;
    r.info = tp_malloc(tp, sizeof(tpd_string));
    TPD_STRING(r)->base = tp_None;
    TPD_STRING(r)->s = (char*) s;
    TPD_STRING(r)->len = n;
    return r;
}

/* return a tracked string object from external memory */
tp_obj tp_string_from_const(TP, const char *s, int n) {
    return tp_track(tp, tp_string_from_const_nt(tp, s, n));
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

tp_obj tp_string_steal_from_builder(TP, StringBuilder * sb)
{
    tp_obj r;
    r.type.typeid = TP_STRING;
    r.type.magic = TP_STRING_NORMAL;
    r.info = tp_malloc(tp, sizeof(tpd_string));
    TPD_STRING(r)->len = sb->len;
    TPD_STRING(r)->s = sb->buffer;
    sb->buffer = NULL;
    sb->len = 0;
    return tp_track(tp, r);
}

char * tp_string_getptr(tp_obj s) {
    if(s.type.magic == TP_STRING_ATOM) {
        return (char*) s.ptr;
    }
    return TPD_STRING(s)->s;
}

int tp_string_len(tp_obj s) {
    if(s.type.magic == TP_STRING_ATOM) {
        return strlen(s.ptr);
    }
    return TPD_STRING(s)->len;
}

/*
 * Create a new string which is a substring slice (view) of another STRING.
 * the returned object does not allocate new memory. It refers to the same
 * memory object to the original string.
 */
tp_obj tp_string_view(TP, tp_obj s, int a, int b) {
    int l = tp_string_len(s);
    a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
    tp_obj r;
    r.type.typeid = TP_STRING;
    r.type.magic = TP_STRING_VIEW;
    r.info = tp_malloc(tp, sizeof(tpd_string));
    TPD_STRING(r)->base = s;
    TPD_STRING(r)->s = tp_string_getptr(s) + a;
    TPD_STRING(r)->len = b - a;
    return tp_track(tp, r);
}

static int _tp_printf_handler(void* tp, void* obj_ptr, int ch, int lenhint, char **buf)
{
    tp_obj * obj = obj_ptr;
    tp_obj str = (ch == 'O')?tp_str(tp, *obj):tp_repr(tp, *obj);
    char * ptr = tp_string_getptr(str);
    int len = tp_string_len(str);
    if (lenhint > 0 && len > lenhint) len = lenhint;
    *buf = malloc(len);
    memcpy(*buf, ptr, len);
    return len;
}

static int _tp_printf_puts(char* s, unsigned int len, void * buf)
{
    StringBuilder * sb = buf;
    string_builder_write(sb, s, len);
    return len;
}
static void _tp_printf_freeor(void* tp, void* buf)
{
    free(buf);
}

tp_obj tp_printf(TP, char const *fmt,...) {
    va_list arg;

    mini_printf_set_handler(tp, _tp_printf_handler, _tp_printf_freeor);
    StringBuilder sb[1] = {tp};
    va_start(arg, fmt);
    mini_vpprintf(_tp_printf_puts, sb, fmt, arg);
    va_end(arg);
    return tp_string_steal_from_builder(tp, sb);
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

int tp_string_cmp_const(tp_obj a, const char * b, int n)
{
    int l = _tp_min(tp_string_len(a), n);
    int v = memcmp(tp_string_getptr(a), b, l);
    if (v == 0) {
        v = tp_string_len(a) - n;
    }
    return v;
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
