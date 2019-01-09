/* tp_frame_*/
void tp_enter_frame(TP, tp_obj globals, tp_obj code, tp_obj * ret_dest) {
    tpd_frame f;
    f.globals = globals;
    f.code = code;
    f.cur = (tpd_code*)f.code.string.val;
    f.jmp = 0;
/*     fprintf(stderr,"tp->cur: %d\n",tp->cur);*/
    f.regs = (tp->cur <= 0?tp->regs:tp->frames[tp->cur].regs+tp->frames[tp->cur].cregs);
    
    f.regs[0] = f.globals;
    f.regs[1] = f.code;
    f.regs += TP_REGS_EXTRA;
    
    f.ret_dest = ret_dest;
    f.lineno = 0;
    f.line = tp_string_const("");
    f.name = tp_string_const("?");
    f.fname = tp_string_const("?");
    f.cregs = 0;
/*     return f;*/
    if (f.regs+(256+TP_REGS_EXTRA) >= tp->regs+TP_REGS || tp->cur >= TP_FRAMES-1) {
        tp_raise(,tp_string_const("(tp_frame) RuntimeError: stack overflow"));
    }
    tp->cur += 1;
    tp->frames[tp->cur] = f;
}

void tp_print_stack(TP) {
    int i;
    tp->echo("\n", -1);
    for (i=0; i<=tp->cur; i++) {
        if (!tp->frames[i].lineno) { continue; }
        tp->echo("File \"", -1); tp_echo(tp,tp->frames[i].fname); tp->echo("\", ", -1);
        tp_echo(tp, tp_printf(tp, "line %d, in ",tp->frames[i].lineno));
        tp_echo(tp,tp->frames[i].name); tp->echo("\n ", -1);
        tp_echo(tp,tp->frames[i].line); tp->echo("\n", -1);
    }
    tp->echo("\nException:\n", -1); tp_echo(tp,tp->ex); tp->echo("\n", -1);
}

