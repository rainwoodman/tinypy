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

    tp_gc_init(tp);

    /* gc initialized, can use tpy_ functions. */

    tp->_chars = tp_list_t(tp);
    for (i=0; i<256; i++) {
        tp->chars[i] = tp_string_t(tp, 1);
        *tp_string_getptr(tp->chars[i]) = i;
        tp_set(tp, tp->_chars, tp_None, tp->chars[i]);
    }

    tp->_regs = tp_list_t(tp);
    for (i=0; i < TP_REGS + 3; i++) { tp_set(tp, tp->_regs, tp_None, tp_None); }

    tp->last_result = tp->_regs.list.val->items + 0;
    tp->exc = tp->_regs.list.val->items + 1;
    tp->exc_stack = tp->_regs.list.val->items + 2;
    tp->regs = tp->_regs.list.val->items + 3;

    tp_obj object_class = tp_object(tp);
    object_class.type.magic = TP_DICT_CLASS;
    tp->object_class = object_class;

    tp->list_class = tp_class(tp);
    tp->dict_class = tp_class(tp);
    tp->string_class = tp_class(tp);

    tp->builtins = tp_dict_t(tp);
    tp->modules = tp_dict_t(tp);

    tp->_params = tp_list_t(tp);

    for (i=0; i<TP_FRAMES; i++) { tp_set(tp, tp->_params, tp_None, tp_list_t(tp)); }
    tp->echo = tp_default_echo;

    tp_gc_set_reachable(tp, tp->_chars);
    tp_gc_set_reachable(tp, tp->builtins);
    tp_gc_set_reachable(tp, tp->modules);
    tp_gc_set_reachable(tp, tp->object_class);
    tp_gc_set_reachable(tp, tp->_regs);
    tp_gc_set_reachable(tp, tp->_params);

    tp_gc_set_reachable(tp, tp->list_class);
    tp_gc_set_reachable(tp, tp->dict_class);
    tp_gc_set_reachable(tp, tp->string_class);

    *tp->last_result = tp_None;
    tp_full(tp);
    return tp;
}

void tp_enter_frame(TP, tp_obj globals, tp_obj code, tp_obj * ret_dest) {
    tpd_frame f = tp_frame_nt(tp, globals, code, ret_dest);

    tp->cur += 1;
    tp->frames[tp->cur] = f;
}

void _tp_raise(TP, tp_obj e) {
    /*char *x = 0; x[0]=0;*/
    if (!tp || !tp->jmp) {
#ifndef CPYTHON_MOD
        tp->echo("\nException:\n", -1); tp_echo(tp,e); tp->echo("\n", -1);
        abort(); \
        exit(-1);
#else
        *(tp->exc) = e;
        *(tp->exc_stack) = tp_format_stack(tp);
        longjmp(tp->nextexpr,1);
#endif
    }
    if (e.type.typeid != TP_NONE) {
        *(tp->exc) = e;
        *(tp->exc_stack) = tp_format_stack(tp);
    }
    tp_grey(tp,e);
    longjmp(tp->buf,1);
}

void tp_format_stack_internal(TP, StringBuilder * sb)
{
    int i;
    string_builder_write(sb, "\n", -1);

    for (i=0; i<=tp->cur; i++) {
        if (!tp->frames[i].lineno) { continue; }
        string_builder_write(sb, "File \"", -1);
        string_builder_echo(sb, *tp->frames[i].fname);
        string_builder_write(sb, "\", ", -1);
        string_builder_echo(sb, tp_printf(tp, "line %d, in ", tp->frames[i].lineno));
        string_builder_echo(sb, *tp->frames[i].name);
        string_builder_write(sb, "\n ", -1);
        string_builder_echo(sb, *tp->frames[i].line);
        string_builder_write(sb, "\n", -1);
    }
}

tp_obj tp_format_stack(TP)
{
    StringBuilder sb[1] = {tp};
    tp_format_stack_internal(tp, sb);
    return tp_string_steal_from_builder(tp, sb);
}

void tp_print_exc(TP) {
    StringBuilder sb[1] = {tp};
    tp_format_stack_internal(tp, sb);
    string_builder_write(sb, "\nException:\n", -1);
    string_builder_echo(sb, *(tp->exc));
    string_builder_write(sb, "\n", -1);
    tp->echo(sb->buffer, sb->len);
    tp_free(tp, sb->buffer);
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
    tp_print_exc(tp);
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
        tp_gcinc(tp);
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

#include "tinypy/tp_opcodes.h"

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
    tpd_code *base = (tpd_code*)f->code.string.info->s;
    /* FIXME: convert this to a flag */
    // fprintf(stdout,"%2d.%4d: %-6s %3d %3d %3d\n",tp->cur, (cur - base) * 4,tp_get_opcode_name(e.i),VA,VB,VC);
//       int i; for(i=0;i<16;i++) { fprintf(stderr,"%d: %s\n",i,TP_xSTR(regs[i])); }
   
//    tp_obj tpy_print(TP);
    switch (e.i) {
        case TP_IEOF: *tp->last_result = RA; tp_return(tp,tp_None); SR(0); break;
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
        case TP_INE: RA = tp_number(!tp_equal(tp,RB,RC)); break;
        case TP_IEQ: RA = tp_number(tp_equal(tp,RB,RC)); break;
        case TP_ILE: RA = tp_number(tp_equal(tp,RB,RC) || tp_lessthan(tp, RB, RC)); break;
        case TP_ILT: RA = tp_number(tp_lessthan(tp,RB,RC)); break;
        case TP_IBITNOT:  RA = tp_bitwise_not(tp,RB); break;
        case TP_INOT: RA = tp_number(!tp_true(tp,RB)); break;
        case TP_IPASS: break;
        case TP_IIF: if (tp_true(tp,RA)) { cur += 1; } break;
        case TP_IIFN: if (!tp_true(tp,RA)) { cur += 1; } break;
        case TP_IGET: RA = tp_get(tp,RB,RC); GA; break;
        case TP_IMGET: RA = tp_mget(tp,RB,RC); GA; break;
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
        case TP_IIN: RA = tp_has(tp,RC,RB); break;
        case TP_INOTIN: RA = tp_number(!tp_true(tp, tp_has(tp,RC,RB))); break;
        case TP_IIGET: tp_iget(tp,&RA,RB,RC); break;
        case TP_ISET: tp_set(tp,RA,RB,RC); break;
        case TP_IDEL: tp_del(tp,RA,RB); break;
        case TP_IUPDATE:
            tp_dict_update(tp, RA, RB);
            break;
        case TP_IMOVE: RA = RB; break;
        case TP_INUMBER:
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,sizeof(tp_num)/4);
            #endif
            cur++;
            RA = tp_number((*cur).number.val[0]);
            cur+= sizeof(tp_num)/4;
            continue;
        case TP_ISTRING: {
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,(UVBC/4)+1);
            #endif
            int a = (*(cur+1)).string.val - tp_string_getptr(f->code);
            RA = tp_string_view(tp, f->code, a, a+UVBC);
            cur += (UVBC/4)+1;
            }
            break;
        case TP_IDICT: RA = tp_dict_from_items(tp, VC/2, &RB); break;
        case TP_ICLASS: RA = tp_class(tp); break;
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
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,SVBC);
            #endif
            int a = (*(cur+1)).string.val - tp_string_getptr(f->code);
            if(tp_string_getptr(f->code)[a] == ';') abort();
            RA = tp_def(tp,
                tp_string_view(tp, f->code, a, a + (SVBC-1)*4),
                f->globals);
            cur += SVBC; continue;
            }
            break;
            
        case TP_IRETURN: tp_return(tp,RA); SR(0); break;
        case TP_IRAISE: _tp_raise(tp,RA); SR(0); break;
        case TP_IASSERT:
            tp_assert(tp, RA, RB, RC);
            break;
        case TP_INONE: RA = tp_None; break;
        case TP_ILINE: {
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,VA);
            #endif
            ;
            int a = (*(cur+1)).string.val - tp_string_getptr(f->code);
            if(tp_string_getptr(f->code)[a] == ';') abort();
            *f->line = tp_string_view(tp, f->code, a, a+VA*4-1);
            cur += VA; f->lineno = UVBC;
            }
            break;
        case TP_IFILE: *f->fname = RA; break;
        case TP_INAME: *f->name = RA; break;
        case TP_IVAR: {
            cur += (UVBC/4) + 1;
            /* Watch out: crash if continue. */
            break;
        }
        case TP_IREGS: f->cregs = VA; break;
        default:
            tp_raise(0,tp_string_atom(tp, "(tp_step) RuntimeError: invalid instruction"));
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
    tp_obj code = tp_compile(tp, tp_string_atom(tp, text), tp_string_atom(tp, "<eval>"));
    tp_exec(tp,code,globals);
    return *tp->last_result;
}

