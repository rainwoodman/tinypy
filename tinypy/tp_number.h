tp_inline static tp_obj tp_int(long v) {
    tp_obj r = {TP_NUMBER};
    r.type.magic = TP_NUMBER_INT;
    r.nint = v;
    return r;
}

tp_inline static tp_obj tp_float(double v) {
    tp_obj r = {TP_NUMBER};
    r.type.magic = TP_NUMBER_FLOAT;
    r.nfloat = v;
    return r;
}

tp_inline static tp_obj tp_number_cast(TP, tp_obj v, enum TPTypeMagic kind) {
    tp_obj r = {TP_NUMBER};
    r.type.magic = kind;
    switch(v.type.magic) {
        case TP_NUMBER_INT: switch (kind) {
            case TP_NUMBER_FLOAT: r.nfloat = v.nint; break;
            case TP_NUMBER_INT: r.nint = v.nint; break;
            default: abort();
        } break;
        case TP_NUMBER_FLOAT: switch (kind) {
            case TP_NUMBER_FLOAT: r.nfloat = v.nfloat; break;
            case TP_NUMBER_INT: r.nint = v.nfloat; break;
            default: abort();
        } break;
        default: abort();
    }
    return r;
}

tp_inline static enum TPTypeMagic tp_number_upcast(TP, tp_obj *a, tp_obj *b) {
    enum TPTypeMagic kind = a->type.magic;
    if(b->type.magic > kind) {
        kind = b->type.magic;
    }
    *a = tp_number_cast(tp, *a, kind);
    *b = tp_number_cast(tp, *b, kind);
    return kind;
}

tp_inline static long tp_number_as_int(TP, tp_obj v) {
    if(v.type.magic != TP_NUMBER_INT) {
        abort();
        tp_raise_printf(0, "tp_number_as_int: expecting an integer");
    }
    return v.nint;
}
#define TPN_AS_INT(v) tp_number_as_int(tp, v)
tp_inline static double tp_number_as_float(TP, tp_obj v) {
    if(v.type.magic != TP_NUMBER_FLOAT) {
        abort();
        tp_raise_printf(0, "tp_number_as_int: expecting a float");
    }
    return v.nfloat;
}
#define TPN_AS_FLOAT(v) tp_number_as_float(tp, v)

tp_inline static tp_obj tp_bool(int v) {
    return v?tp_True:tp_False;
}
