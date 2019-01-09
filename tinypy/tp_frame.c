/* tp_frame_*/
tpd_frame tp_frame_nt(TP, tp_obj globals, tp_obj code, tp_obj * ret_dest)
{
    tpd_frame f;
    f.globals = globals;
    f.code = code;
    f.cur = (tpd_code*)f.code.string.info->s;
    f.jmp = 0;
/*     fprintf(stderr,"tp->cur: %d\n",tp->cur);*/
    f.regs = (tp->cur <= 0?tp->regs:tp->frames[tp->cur].regs+tp->frames[tp->cur].cregs);
    
    f.regs[0] = f.globals;
    f.regs[1] = f.code;
    f.regs += TP_REGS_EXTRA;
    
    f.ret_dest = ret_dest;
    f.lineno = 0;
    f.line = tp_string_atom(tp, "");
    f.name = tp_string_atom(tp, "?");
    f.fname = tp_string_atom(tp, "?");
    f.cregs = 0;
    return f;
}
