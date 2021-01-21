tp_obj tp_call_extern(TP, tp_obj func) {
}

tp_obj tp_func_nt(TP, int t, void *ptr) {
    tp_obj r = {TP_FUNC};
    tpd_func *info = (tpd_func*)tp_malloc(tp, sizeof(tpd_func));
    r.type.mask = t;
    r.info = info;
    r.ptr = ptr;
    return r;
}

tp_obj tp_func_t(TP, int t, void *ptr) {
    return tp_track(tp, tp_func_nt(tp, t, ptr));
}

tp_obj tp_bind(TP, tp_obj function, tp_obj self) {
    tp_obj r = tp_func_nt(tp,
                function.type.mask | TP_FUNC_MASK_METHOD,
                function.ptr);
    memcpy(r.info, function.info, sizeof(*TPD_FUNC(function)));
    TPD_FUNC(r)->instance = self;
    return tp_track(tp, r);
}

tp_obj tp_staticmethod(TP, tp_obj function) {
    tp_obj r = tp_func_nt(tp,
                function.type.mask | TP_FUNC_MASK_STATIC,
                function.ptr);
    memcpy(r.info, function.info, sizeof(*TPD_FUNC(function)));
    TPD_FUNC(r)->instance = tp_None;
    return tp_track(tp, r);
}

tp_obj tp_def(TP, tp_obj code, tp_obj g,
    tp_obj args,
    tp_obj defaults,
    tp_obj varargs,
    tp_obj varkw) {
    tp_obj r = tp_func_t(tp, 0, NULL);
    TPD_FUNC(r)->code = code;
    TPD_FUNC(r)->globals = g;
    TPD_FUNC(r)->instance = tp_None;
    TPD_FUNC(r)->args = args;
    TPD_FUNC(r)->defaults = defaults;
    TPD_FUNC(r)->varargs = varargs;
    TPD_FUNC(r)->varkw = varkw;
    return r;
}

/* Function: tp_func
 * Creates a new tinypy function object.
 * 
 * This is how you can create a tinypy function object which, when called in
 * the script, calls the provided C function.
 */
tp_obj tp_function(TP, tp_obj v(TP)) {
    tp_obj r = tp_func_t(tp, 0, v);
    TPD_FUNC(r)->code = tp_None;
    TPD_FUNC(r)->globals = tp_None;
    TPD_FUNC(r)->instance = tp_None;
    TPD_FUNC(r)->args = tp_None;
    TPD_FUNC(r)->defaults = tp_None;
    TPD_FUNC(r)->varargs = tp_None;
    TPD_FUNC(r)->varkw = tp_None;
    return r;
}

/* Function: tp_method
 * Creates a method for an instance. Use this in a C module to create objects by
 * proto-typing.
 */
tp_obj tp_method(TP, tp_obj self, tp_obj v(TP)) {
    return tp_bind(tp, tp_function(tp, v), self);
}


