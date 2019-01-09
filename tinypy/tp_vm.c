/* write to standard output */
void tp_default_echo(const char* string, int length) {
    if(length < 0) length = strlen(string);
    fwrite(string, 1, length, stdout);
}

/* File: VM
 * Functionality pertaining to the virtual machine.
 */

tp_vm * tp_create_vm(void) {
    int i;
    tp_vm *tp = (tp_vm*)calloc(sizeof(tp_vm),1);
    tp->time_limit = TP_NO_LIMIT;
    tp->clocks = clock();
    tp->time_elapsed = 0.0;
    tp->mem_limit = TP_NO_LIMIT;
    tp->mem_exceeded = 0;
    tp->mem_used = sizeof(tp_vm);
    tp->cur = 0;
    tp->jmp = 0;
    tp->ex = tp_None;
    tp->root = tp_list_nt(tp); /* root is not tracked by gc, as gc is not defined yet.*/

    for (i=0; i<256; i++) { tp->chars[i][0]=i; }

    tp_gc_init(tp);
    
    /* gc initialized, can use tpy_ functions. */
    tp->_regs = tp_list_t(tp);
    for (i=0; i<TP_REGS; i++) { tp_set(tp, tp->_regs, tp_None, tp_None); }
    tp->builtins = tp_dict_t(tp);
    tp->modules = tp_dict_t(tp);
    tp->_params = tp_list_t(tp);
    for (i=0; i<TP_FRAMES; i++) { tp_set(tp, tp->_params, tp_None, tp_list_t(tp)); }
    tp->echo = tp_default_echo;
    tp_set(tp,tp->root,tp_None,tp->builtins);
    tp_set(tp,tp->root,tp_None,tp->modules);
    tp_set(tp,tp->root,tp_None,tp->_regs);
    tp_set(tp,tp->root,tp_None,tp->_params);
    
    
    tp->regs = tp->_regs.list.val->items;
    tp->last_result = tp_None;
    tp_full(tp);
    return tp;
}

void tp_enter_frame(TP, tp_obj globals, tp_obj code, tp_obj * ret_dest) {
    tpd_frame f = tp_frame_nt(tp, globals, code, ret_dest);

    if (f.regs+(256+TP_REGS_EXTRA) >= tp->regs+TP_REGS || tp->cur >= TP_FRAMES-1) {
        tp_raise(,tp_string_const("(tp_frame) RuntimeError: stack overflow"));
    }
    tp->cur += 1;
    tp->frames[tp->cur] = f;
}

void _tp_raise(TP,tp_obj e) {
    /*char *x = 0; x[0]=0;*/
    if (!tp || !tp->jmp) {
#ifndef CPYTHON_MOD
        tp->echo("\nException:\n", -1); tp_echo(tp,e); tp->echo("\n", -1);
        exit(-1);
#else
        tp->ex = e;
        longjmp(tp->nextexpr,1);
#endif
    }
    if (e.type != TP_NONE) { tp->ex = e; }
    tp_grey(tp,e);
    longjmp(tp->buf,1);
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


void tp_handle(TP) {
    int i;
    for (i=tp->cur; i>=0; i--) {
        if (tp->frames[i].jmp) { break; }
    }
    if (i >= 0) {
        tp->cur = i;
        tp->frames[i].cur = tp->frames[i].jmp;
        tp->frames[i].jmp = 0;
        return;
    }
#ifndef CPYTHON_MOD
    tp_print_stack(tp);
    exit(-1);
#else
    longjmp(tp->nextexpr,1);
#endif
}


int tp_step(TP);
void tp_continue_frame(TP, int cur) {
    jmp_buf tmp;
    memcpy(tmp, tp->buf, sizeof(jmp_buf));
    tp->jmp += 1;
    if (setjmp(tp->buf)) {
        tp_handle(tp);
    }
    /* keep runing till the frame drops back (aka function returns) */
    while (tp->cur >= cur) {
        if (tp_step(tp) == -1) break;
    }

    tp->jmp -= 1;
    memcpy(tp->buf, tmp, sizeof(jmp_buf));
}

/* run the current frame till it returns */
void tp_run_frame(TP) {
    tp_continue_frame(tp, tp->cur);
}


void tp_return(TP, tp_obj v) {
    tp_obj *dest = tp->frames[tp->cur].ret_dest;
    if (dest) { *dest = v; tp_grey(tp,v); }
/*     memset(tp->frames[tp->cur].regs,0,TP_REGS_PER_FRAME*sizeof(tp_obj));
       fprintf(stderr,"regs:%d\n",(tp->frames[tp->cur].cregs+1));*/
    memset(tp->frames[tp->cur].regs-TP_REGS_EXTRA,0,(TP_REGS_EXTRA+tp->frames[tp->cur].cregs)*sizeof(tp_obj));
    tp->cur -= 1;
}

enum {
    TP_IEOF,TP_IADD,TP_ISUB,TP_IMUL,TP_IDIV,TP_IPOW,TP_IBITAND,TP_IBITOR,TP_ICMP,TP_IGET,TP_ISET,
    TP_INUMBER,TP_ISTRING,TP_IGGET,TP_IGSET,TP_IMOVE,TP_IDEF,TP_IPASS,TP_IJUMP,TP_ICALL,
    TP_IRETURN,TP_IIF,TP_IDEBUG,TP_IEQ,TP_ILE,TP_ILT,TP_IDICT,TP_ILIST,TP_INONE,TP_ILEN,
    TP_ILINE,TP_IPARAMS,TP_IIGET,TP_IFILE,TP_INAME,TP_INE,TP_IHAS,TP_IRAISE,TP_ISETJMP,
    TP_IMOD,TP_ILSH,TP_IRSH,TP_IITER,TP_IDEL,TP_IREGS,TP_IBITXOR, TP_IIFN, 
    TP_INOT, TP_IBITNOT,
    TP_ITOTAL
};

/* char *tp_strings[TP_ITOTAL] = {
       "EOF","ADD","SUB","MUL","DIV","POW","BITAND","BITOR","CMP","GET","SET","NUM",
       "STR","GGET","GSET","MOVE","DEF","PASS","JUMP","CALL","RETURN","IF","DEBUG",
       "EQ","LE","LT","DICT","LIST","NONE","LEN","LINE","PARAMS","IGET","FILE",
       "NAME","NE","HAS","RAISE","SETJMP","MOD","LSH","RSH","ITER","DEL","REGS",
       "BITXOR", "IFN", "NOT", "BITNOT",
   };*/

#define VA ((int)e.regs.a)
#define VB ((int)e.regs.b)
#define VC ((int)e.regs.c)
#define RA regs[e.regs.a]
#define RB regs[e.regs.b]
#define RC regs[e.regs.c]
#define UVBC (unsigned short)(((VB<<8)+VC))
#define SVBC (short)(((VB<<8)+VC))
#define GA tp_grey(tp,RA)
#define SR(v) f->cur = cur; return(v);


int tp_step(TP) {
    tpd_frame *f = &tp->frames[tp->cur];
    tp_obj *regs = f->regs;
    tpd_code *cur = f->cur;
    while(1) {
    #ifdef TP_SANDBOX
    tp_bounds(tp,cur,1);
    #endif
    tpd_code e = *cur;
    /*
     fprintf(stderr,"%2d.%4d: %-6s %3d %3d %3d\n",tp->cur,cur - (tpd_code*)f->code.string.val,tp_strings[e.i],VA,VB,VC);
       int i; for(i=0;i<16;i++) { fprintf(stderr,"%d: %s\n",i,TP_xSTR(regs[i])); }
    */
    switch (e.i) {
        case TP_IEOF: tp->last_result = RA; tp_return(tp,tp_None); SR(0); break;
        case TP_IADD: RA = tp_add(tp,RB,RC); break;
        case TP_ISUB: RA = tp_sub(tp,RB,RC); break;
        case TP_IMUL: RA = tp_mul(tp,RB,RC); break;
        case TP_IDIV: RA = tp_div(tp,RB,RC); break;
        case TP_IPOW: RA = tp_pow(tp,RB,RC); break;
        case TP_IBITAND: RA = tp_bitwise_and(tp,RB,RC); break;
        case TP_IBITOR:  RA = tp_bitwise_or(tp,RB,RC); break;
        case TP_IBITXOR:  RA = tp_bitwise_xor(tp,RB,RC); break;
        case TP_IMOD:  RA = tp_mod(tp,RB,RC); break;
        case TP_ILSH:  RA = tp_lsh(tp,RB,RC); break;
        case TP_IRSH:  RA = tp_rsh(tp,RB,RC); break;
        case TP_ICMP: RA = tp_number(tp_cmp(tp,RB,RC)); break;
        case TP_INE: RA = tp_number(tp_cmp(tp,RB,RC)!=0); break;
        case TP_IEQ: RA = tp_number(tp_cmp(tp,RB,RC)==0); break;
        case TP_ILE: RA = tp_number(tp_cmp(tp,RB,RC)<=0); break;
        case TP_ILT: RA = tp_number(tp_cmp(tp,RB,RC)<0); break;
        case TP_IBITNOT:  RA = tp_bitwise_not(tp,RB); break;
        case TP_INOT: RA = tp_number(!tp_true(tp,RB)); break;
        case TP_IPASS: break;
        case TP_IIF: if (tp_true(tp,RA)) { cur += 1; } break;
        case TP_IIFN: if (!tp_true(tp,RA)) { cur += 1; } break;
        case TP_IGET: RA = tp_get(tp,RB,RC); GA; break;
        case TP_IITER:
            if (RC.number.val < tp_len(tp,RB).number.val) {
                RA = tp_iter(tp,RB,RC); GA;
                RC.number.val += 1;
                #ifdef TP_SANDBOX
                tp_bounds(tp,cur,1);
                #endif
                cur += 1;
            }
            break;
        case TP_IHAS: RA = tp_has(tp,RB,RC); break;
        case TP_IIGET: tp_iget(tp,&RA,RB,RC); break;
        case TP_ISET: tp_set(tp,RA,RB,RC); break;
        case TP_IDEL: tp_del(tp,RA,RB); break;
        case TP_IMOVE: RA = RB; break;
        case TP_INUMBER:
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,sizeof(tp_num)/4);
            #endif
            RA = tp_number(*(tp_num*)(*++cur).string.val);
            cur += sizeof(tp_num)/4;
            continue;
        case TP_ISTRING: {
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,(UVBC/4)+1);
            #endif
            /* RA = tp_string_nt((*(cur+1)).string.val,UVBC); */
            int a = (*(cur+1)).string.val-f->code.string.val;
            RA = tp_string_sub(tp,f->code,a,a+UVBC),
            cur += (UVBC/4)+1;
            }
            break;
        case TP_IDICT: RA = tp_dict_from_items(tp, VC/2, &RB); break;
        case TP_ILIST: RA = tp_list_from_items(tp, VC, &RB); break;
        case TP_IPARAMS: RA = tp_params_n(tp,VC,&RB); break;
        case TP_ILEN: RA = tp_len(tp,RB); break;
        case TP_IJUMP: cur += SVBC; continue; break;
        case TP_ISETJMP: f->jmp = SVBC?cur+SVBC:0; break;
        case TP_ICALL:
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,1);
            #endif
            f->cur = cur + 1;  RA = tp_call(tp,RB,RC); GA;
            return 0; break;
        case TP_IGGET:
            if (!tp_iget(tp,&RA,f->globals,RB)) {
                RA = tp_get(tp,tp->builtins,RB); GA;
            }
            break;
        case TP_IGSET: tp_set(tp,f->globals,RA,RB); break;
        case TP_IDEF: {
/*            RA = tp_def(tp,(*(cur+1)).string.val,f->globals);*/
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,SVBC);
            #endif
            int a = (*(cur+1)).string.val-f->code.string.val;
            RA = tp_def(tp,
                /*tp_string_nt((*(cur+1)).string.val,(SVBC-1)*4),*/
                tp_string_sub(tp,f->code,a,a+(SVBC-1)*4),
                f->globals);
            cur += SVBC; continue;
            }
            break;
            
        case TP_IRETURN: tp_return(tp,RA); SR(0); break;
        case TP_IRAISE: _tp_raise(tp,RA); SR(0); break;
        case TP_IDEBUG:
            tp_echo(tp, tp_string_const("DEBUG:"));
            tp_echo(tp, tp_number(VA));
            tp_echo(tp, RA);
            break;
        case TP_INONE: RA = tp_None; break;
        case TP_ILINE: {
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,VA);
            #endif
            ;
            int a = (*(cur+1)).string.val-f->code.string.val;
/*            f->line = tp_string_nt((*(cur+1)).string.val,VA*4-1);*/
            f->line = tp_string_sub(tp,f->code,a,a+VA*4-1);
/*             fprintf(stderr,"%7d: %s\n",UVBC,f->line.string.val);*/
            cur += VA; f->lineno = UVBC;
            }
            break;
        case TP_IFILE: f->fname = RA; break;
        case TP_INAME: f->name = RA; break;
        case TP_IREGS: f->cregs = VA; break;
        default:
            tp_raise(0,tp_string_const("(tp_step) RuntimeError: invalid instruction"));
            break;
    }
    #ifdef TP_SANDBOX
    tp_time_update(tp);
    tp_mem_update(tp);
    tp_bounds(tp,cur,1);
    #endif
    cur += 1;
    }
    SR(0);
}


/* Function: tp_exec
 * Execute VM code.
 */
tp_obj tp_exec(TP, tp_obj code, tp_obj globals) {
    tp_obj r = tp_None;
    tp_enter_frame(tp, globals, code, &r);
    tp_run_frame(tp);
    return r;
}

tp_obj tp_eval_from_cstr(TP, const char *text, tp_obj globals) {
    tp_obj code = tp_compile(tp,tp_string_const(text),tp_string_const("<eval>"));
    tp_exec(tp,code,globals);
    return tp->last_result;
}

