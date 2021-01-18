#include <stdint.h>

tp_obj tp_pack(TP, char *format, tp_obj v) {
    tp_obj r;
    if(*format != '=') {
        tp_raise_printf(tp_None, "Only = format is supported.");
    }
    format ++;
    switch(*format) {
        case 'd':
            r = tp_string_t(tp,sizeof(double));
            *(double*) tp_string_getptr(r) = TPN_AS_FLOAT(v);
            break;
        case 'i':
            r = tp_string_t(tp,sizeof(int32_t));
            *(int32_t*) tp_string_getptr(r) = TPN_AS_INT(v);
            break;
        case 'I':
            r = tp_string_t(tp,sizeof(uint32_t));
            *(uint32_t*) tp_string_getptr(r) = TPN_AS_INT(v);
            break;
        case 'q':
            r = tp_string_t(tp,sizeof(int64_t));
            *(int64_t*) tp_string_getptr(r) = TPN_AS_INT(v);
            break;
        case 'Q':
            r = tp_string_t(tp,sizeof(uint64_t));
            *(uint64_t*) tp_string_getptr(r) = TPN_AS_INT(v);
            break;
        default:
            abort();
    }
    return r;
}
tp_obj tp_unpack(TP, char * format, tp_obj v) {
    tp_obj r;
    if(*format != '=') {
        tp_raise_printf(tp_None, "Only = format is supported.");
    }
    format ++;
    switch(*format) {
        case 'd':
            if (tp_string_len(v) != sizeof(double)) goto ex_len;
            return tp_float(*((double*) tp_string_getptr(v)));
        case 'i':
            if (tp_string_len(v) != sizeof(int32_t)) goto ex_len;
            return tp_int(*((int32_t*) tp_string_getptr(v)));
        case 'I':
            if (tp_string_len(v) != sizeof(uint32_t)) goto ex_len;
            return tp_int(*((uint32_t*) tp_string_getptr(v)));
        case 'q':
            if (tp_string_len(v) != sizeof(int64_t)) goto ex_len;
            return tp_int(*((int64_t*) tp_string_getptr(v)));
        case 'Q':
            if (tp_string_len(v) != sizeof(uint64_t)) goto ex_len;
            return tp_int(*((uint64_t*) tp_string_getptr(v)));
        default:
            abort();
    }
    return r;

ex_len:
    tp_raise(tp_None, tp_string_atom(tp, "unpack ValueError: length of string is incorrect."));
}

