tp_obj tp_frame_t(TP, tp_obj lparams, tp_obj dparams,
        tp_obj globals, tp_obj code,
        tp_obj args, tp_obj defaults, tp_obj * ret_dest) {
    tp_obj r = {TP_FRAME};
    r.info = tp_malloc(tp, sizeof(tpd_frame));
    tpd_frame * f = TPD_FRAME(r);

    f->globals = globals;
    f->code = code;
    f->cur = (tpd_code*) tp_string_getptr(f->code);
    f->jmp = 0;
    f->ret_dest = ret_dest;
    f->lineno = 0;
    f->lparams = lparams;
    f->dparams = dparams;
    f->args = args;
    f->defaults = defaults;
    f->line = tp->chars['?'];
    f->name = tp->chars['?'];
    f->fname = tp->chars['?'];
    f->cregs = 0;
    return tp_track(tp, r);
}

void tpd_frame_alloc(TP, tpd_frame * f, tp_obj * regs, int cregs) {
    /*  call convention requires 1 reg for __params__.*/
    f->regs = regs;

    int nargs = tp_none(f->args)?0:TPD_LIST(f->args)->len;
    tp_obj * args = tp_none(f->args)?NULL:TPD_LIST(f->args)->items;

    int ndefaults = tp_none(f->defaults)?0:TPD_LIST(f->defaults)->len;
    tp_obj * defaults = tp_none(f->defaults)?NULL:TPD_LIST(f->defaults)->items;
    int i;
    int nrequired = nargs - ndefaults;
    int nlparams = tp_none(f->lparams)?0:TPD_LIST(f->lparams)->len;

    tp_obj varkw = tp_none(f->dparams)?tp_dict_t(tp):tp_dict_copy(tp, f->dparams);

    for(i = 0; i < nargs; i ++) {
        /* calling convention local 0...len(args)-1 = args. local len(args) = varargs. local len(args)+1 = varkw */
        if(i < nlparams) {
            if(tp_dict_has(tp, varkw, args[i])) {
                tp_raise_printf(, "(tp_call): argument %O given both as positional and keyword", &args[i]);
            }
            f->regs[i] = TPD_LIST(f->lparams)->items[i];
        } else {
            if(tp_dict_has(tp, varkw, args[i])) {
                f->regs[i] = tp_dict_get(tp, varkw, args[i]);
                tp_dict_del(tp, varkw, args[i]);
            } else if(i >= nrequired) {
                f->regs[i] = defaults[i - nrequired];
            } else {
                tp_raise_printf(, "(tp_call): argument %O is missing", &args[i]);
            }
        }
    }
    if (nargs < nlparams) {
        f->regs[nargs] = tp_list_from_items(tp, nlparams - nargs, &TPD_LIST(f->lparams)->items[nargs]);
    } else {
        f->regs[nargs] = tp_None;
    }

    f->regs[nargs + 1] = varkw;

    if(cregs < nargs + 1) {
        abort();
    }
    #if 0
    printf("nargs = %d f->args = %s\n", nargs, tp_cstr(tp, tp_str(tp, f->args)));
    for(i = 0; i < cregs; i ++) {
        printf("regs[%d] %s\n", i, tp_cstr(tp, tp_str(tp, f->regs[i])));
    }
    #endif
    f->cregs = cregs;
}
