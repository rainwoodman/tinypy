tp_obj tp_copy(TP, tp_obj obj);
void   tp_set(TP, tp_obj, tp_obj, tp_obj);
int    tp_iget(TP,tp_obj *r, tp_obj self, tp_obj k);

tp_obj tp_mget(TP, tp_obj, tp_obj);
tp_obj tp_get(TP, tp_obj, tp_obj);
tp_obj tp_has(TP, tp_obj self, tp_obj k);
tp_obj tp_len(TP, tp_obj);
tp_obj tp_call(TP, tp_obj func, tp_obj params);
tp_obj tp_iter(TP, tp_obj self, tp_obj k);

void   tp_del(TP, tp_obj, tp_obj);
tp_obj tp_str(TP, tp_obj);
tp_obj tp_repr(TP, tp_obj);
int    tp_true(TP, tp_obj);
int    tp_cmp(TP, tp_obj, tp_obj);
tp_obj tp_add(TP,tp_obj a, tp_obj b) ;
tp_obj tp_mul(TP, tp_obj a, tp_obj b);
int    tp_hash(TP, tp_obj v);

tp_obj tp_bitwise_and(TP,tp_obj a, tp_obj b) ;
tp_obj tp_bitwise_or(TP,tp_obj a, tp_obj b) ;
tp_obj tp_bitwise_xor(TP,tp_obj a, tp_obj b) ;
tp_obj tp_lsh(TP,tp_obj a, tp_obj b) ;
tp_obj tp_rsh(TP,tp_obj a, tp_obj b) ;
tp_obj tp_sub(TP,tp_obj a, tp_obj b) ;
tp_obj tp_mod(TP,tp_obj a, tp_obj b) ;
tp_obj tp_div(TP,tp_obj a, tp_obj b) ;
tp_obj tp_pow(TP,tp_obj a, tp_obj b) ;
tp_obj tp_bitwise_not(TP, tp_obj);

void tp_assert(TP, tp_obj r, tp_obj b, tp_obj c);
