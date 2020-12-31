tp_obj tp_frame_t(TP) {
    tp_obj r = {TP_FRAME};
    r.frame.info = tp_malloc(tp, sizeof(tpd_frame));
    return r;
}

void tpd_frame_reset(TP, tpd_frame * f, tp_obj params, tp_obj globals, tp_obj code, tp_obj * ret_dest)
{
    tpd_frame * bf = tp->frames[tp->cur].frame.info;
    f->globals = globals;
    f->code = code;
    f->cur = (tpd_code*) tp_string_getptr(f->code);
    f->jmp = 0;
/*     fprintf(stderr,"tp->cur: %d\n",tp->cur);*/
    f->regs = (tp->cur <= 0?tp->regs:bf->regs+bf->cregs);

    f->regs[0] = f->globals;
    f->regs[1] = f->code;
    f->regs += TP_REGS_EXTRA;

    if (f->regs+(256+TP_REGS_EXTRA) >= tp->regs+TP_REGS || tp->cur >= TP_FRAMES-1) {
        tp_raise(, tp_string_atom(tp, "(tp_frame) RuntimeError: stack overflow"));
    }

    f->ret_dest = ret_dest;
    f->lineno = 0;
    f->line = tp_string_atom(tp, "");
    f->name = tp_string_atom(tp, "?");
    f->fname = tp_string_atom(tp, "?");
    f->cregs = 0;
    /* calling convention AX = params. who picks this up? */
    f->regs[0] = params;
}
