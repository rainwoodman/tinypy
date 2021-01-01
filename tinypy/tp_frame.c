tp_obj tp_frame_t(TP) {
    tp_obj r = {TP_FRAME};
    r.frame.info = tp_malloc(tp, sizeof(tpd_frame));
    tpd_frame * f = r.frame.info;

    return r;
}

void tpd_frame_reset(TP, tpd_frame * f, tp_obj params, tp_obj globals, tp_obj code, tp_obj * ret_dest)
{
    f->globals = globals;
    f->code = code;
    f->cur = (tpd_code*) tp_string_getptr(f->code);
    f->jmp = 0;
    f->ret_dest = ret_dest;
    f->lineno = 0;
    f->args = params;
    f->line = tp_string_atom(tp, "");
    f->name = tp_string_atom(tp, "?");
    f->fname = tp_string_atom(tp, "?");
    f->cregs = 0;
}

void tpd_frame_alloc(TP, tpd_frame * f, tp_obj * regs, int cregs) {
    /*  call convention requires 1 reg for __params__.*/
    if(cregs < 1) {
        abort();
    }
    f->regs = regs;
    /* calling convention local #0 = params. */
    f->regs[0] = f->args;
    f->cregs = cregs;
}
