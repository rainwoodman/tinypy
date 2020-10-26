tp_obj tp_call_extern(TP, tp_obj func) {
}

tp_obj tp_func_nt(TP, int t, void *v, tp_obj c, tp_obj s, tp_obj g) {
    tp_obj r = {TP_FUNC};
    tpd_func *info = (tpd_func*)tp_malloc(tp, sizeof(tpd_func));
    info->code = c;
    info->instance = s;
    info->globals = g;
    r.type.mask = t;
    r.func.info = info;
    r.func.cfnc = v;
    return r;
}

tp_obj tp_func_t(TP, int t, void *v, tp_obj c, tp_obj s, tp_obj g) {
    return tp_track(tp, tp_func_nt(tp, t, v, c, s, g));
}

tp_obj tp_bind(TP, tp_obj function, tp_obj self) {
    return tp_func_t(tp,
                function.type.mask | TP_FUNC_MASK_METHOD,
                function.func.cfnc,
                function.func.info->code,
                self,
                function.func.info->globals);
}

tp_obj tp_staticmethod(TP, tp_obj function) {
    return tp_func_t(tp,
                function.type.mask | TP_FUNC_MASK_STATIC,
                function.func.cfnc,
                function.func.info->code,
                tp_None,
                function.func.info->globals);
}

tp_obj tp_def(TP, tp_obj code, tp_obj g) {
    return tp_func_t(tp, 0, NULL, code, tp_None, g);
}

/* Function: tp_func
 * Creates a new tinypy function object.
 * 
 * This is how you can create a tinypy function object which, when called in
 * the script, calls the provided C function.
 */
tp_obj tp_function(TP, tp_obj v(TP)) {
    return tp_func_t(tp, 0, v, tp_None, tp_None, tp_None);
}

/* Function: tp_method
 * Creates a method for an instance. Use this in a C module to create objects by
 * proto-typing.
 */
tp_obj tp_method(TP,tp_obj self, tp_obj v(TP)) {
    return tp_func_t(tp, TP_FUNC_MASK_METHOD, v, tp_None, self, tp_None);
}


