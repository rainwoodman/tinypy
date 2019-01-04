#ifndef _OPS_H_
# define _OPS_H_

# include "tp.h"

tp_obj tp_str(TP,tp_obj self);
int tp_bool(TP,tp_obj v);
tp_obj tp_has(TP,tp_obj self, tp_obj k);
void tp_del(TP,tp_obj self, tp_obj k);
tp_obj tp_iter(TP,tp_obj self, tp_obj k);
tp_obj tp_get(TP,tp_obj self, tp_obj k);
int tp_iget(TP,tp_obj *r, tp_obj self, tp_obj k);
void tp_set(TP,tp_obj self, tp_obj k, tp_obj v);
tp_obj tp_add(TP,tp_obj a, tp_obj b);
tp_obj tp_mul(TP,tp_obj a, tp_obj b);
tp_obj tp_len(TP,tp_obj self);
int tp_cmp(TP,tp_obj a, tp_obj b);

# include "string.h"

#define TP_OP_H(name) \
  tp_obj name(TP,tp_obj _a, tp_obj _b)

#define TP_OP(name,expr) \
    tp_obj name(TP,tp_obj _a,tp_obj _b) { \
    if (_a.type == TP_NUMBER && _a.type == _b.type) { \
        tp_num a = _a.number.val; tp_num b = _b.number.val; \
        return tp_number(expr); \
    } \
    tp_raise(tp_None,tp_string("(" #name ") TypeError: unsupported operand type(s)")); \
}

TP_OP_H(tp_bitwise_and);
TP_OP_H(tp_bitwise_or);
TP_OP_H(tp_bitwise_xor);
TP_OP_H(tp_mod);
TP_OP_H(tp_lsh);
TP_OP_H(tp_rsh);
TP_OP_H(tp_sub);
TP_OP_H(tp_div);
TP_OP_H(tp_pow);

tp_obj tp_bitwise_not(TP, tp_obj a);

#endif /*!_OPS_H_*/
