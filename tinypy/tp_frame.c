tp_obj tp_frame_t(TP) {
    tp_obj r = {TP_FRAME};
    r.frame.info = tp_malloc(tp, sizeof(tpd_frame));
    tpd_frame * f = r.frame.info;
    /* FIXME: number of regs per frame, make this a parameter. */
    f->nregs = 256;
    f->regs = tp_malloc(tp, sizeof(tp_obj) * f->nregs);

    return r;
}

void tpd_frame_reset(TP, tpd_frame * f, tp_obj params, tp_obj globals, tp_obj code, tp_obj * ret_dest)
{
    tpd_frame * bf = tp->frames[tp->cur].frame.info;
    f->globals = globals;
    f->code = code;
    f->cur = (tpd_code*) tp_string_getptr(f->code);
    f->jmp = 0;
    f->regs[0] = f->globals;
    f->regs[1] = f->code;
    f->ret_dest = ret_dest;
    f->lineno = 0;
    f->line = tp_string_atom(tp, "");
    f->name = tp_string_atom(tp, "?");
    f->fname = tp_string_atom(tp, "?");
    f->cregs = TP_REGS_START + 1;
    /* calling convention AX = params. who picks this up? */
    f->regs[TP_REGS_START] = params;
}
