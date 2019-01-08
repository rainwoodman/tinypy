
tp_obj tpd_call(TP, tp_obj fnc(TP)) {
    return fnc(tp);
}

tp_obj tp_tcall(TP, tp_obj fnc) {
    if (fnc.fnc.ftype & 2) {
        /* METHOD */
        tpd_list_insert(tp, tp->params.list.val, 0, fnc.fnc.info->self);
    }
    return tpd_call(tp,(tp_obj (*)(tp_vm *))fnc.fnc.cfnc);
}

tp_obj tp_fnc_new(TP, int t, void *v, tp_obj c,tp_obj s, tp_obj g) {
    tp_obj r = {TP_FNC};
    _tp_fnc *info = (_tp_fnc*)tp_malloc(tp, sizeof(_tp_fnc));
    info->code = c;
    info->self = s;
    info->globals = g;
    r.fnc.ftype = t;
    r.fnc.info = info;
    r.fnc.cfnc = v;
    return r;
}

tp_obj tpy_def(TP, tp_obj code, tp_obj g) {
    tp_obj r = tp_fnc_new(tp,1,0,code,tp_None,g);
    return tp_track(tp, r);
}

/* Function: tp_fnc
 * Creates a new tinypy function object.
 * 
 * This is how you can create a tinypy function object which, when called in
 * the script, calls the provided C function.
 */
tp_obj tpy_fnc(TP, tp_obj v(TP)) {
    return tp_track(tp, tp_fnc_new(tp,0,v,tp_None,tp_None,tp_None));
}

tp_obj tpy_method(TP,tp_obj self,tp_obj v(TP)) {
    return tp_track(tp, tp_fnc_new(tp,2,v,tp_None,self,tp_None));
}
