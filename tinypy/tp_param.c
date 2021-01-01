/* Function: tp_params
 * Initialize the tinypy parameters.
 *
 * When you are calling a tinypy function, you can use this to initialize the
 * list of parameters getting passed to it. Usually, you may want to use
 * <tp_params_n> or <tp_params_v>.
 *
 * In a C-API function, always finish 'parsing' the params before calling other
 * tpy functions or tp_exec (which may eventually call other tpy functions).
 */
tp_obj tp_params(TP) {
    *tp->params = tp_list_t(tp);
    return *tp->params;
}

/* Function: tp_params_n
 * Specify a list of objects as function call parameters.
 *
 * See also: <tp_params>, <tp_params_v>
 *
 * Parameters:
 * n - The number of parameters.
 * argv - A list of n tinypy objects, which will be passed as parameters.
 *
 * Returns:
 * The parameters list. You may modify it before performing the function call.
 */
tp_obj tp_params_n(TP,int n, tp_obj argv[]) {
    tp_obj r = tp_params(tp);
    int i;
    for (i=0; i<n; i++) {
        tpd_list_append(tp, r.list.val, argv[i]);
    }
    return r;
}

/* Function: tp_params_v
 * Pass parameters for a tinypy function call.
 * 
 * When you want to call a tinypy method, then you use this to pass parameters
 * to it.
 * 
 * Parameters:
 * n   - The number of variable arguments following.
 * ... - Pass n tinypy objects, which a subsequently called tinypy method will
 *       receive as parameters.
 * 
 * Returns:
 * A tinypy list object representing the current call parameters. You can modify
 * the list before doing the function call.
 */
tp_obj tp_params_v(TP,int n,...) {
    int i;
    tp_obj r = tp_params(tp);
    va_list a; va_start(a,n);
    for (i=0; i<n; i++) {
        tpd_list_append(tp, r.list.val, va_arg(a, tp_obj));
    }
    va_end(a);
    return r;
}
