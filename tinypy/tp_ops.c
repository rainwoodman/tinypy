
/* Function: tp_true
 * Check the truth value of an object
 *
 * Returns false if v is a numeric object with a value of exactly 0, v is of
 * type None or v is a string list or dictionary with a length of 0. Else true
 * is returned.
 */
int tp_true(TP, tp_obj v) {
    switch(v.type.typeid) {
        case TP_NUMBER: {
            switch(v.type.magic) {
                case TP_NUMBER_FLOAT:
                    return TPN_AS_FLOAT(v) != 0;
                case TP_NUMBER_INT:
                    return TPN_AS_INT(v) != 0;
                default:
                    abort();
            }
        }
        case TP_NONE: return 0;
        case TP_STRING: return tp_string_len(v) != 0;
        case TP_LIST: return TPD_LIST(v)->len != 0;
        case TP_DICT: return TPD_DICT(v)->len != 0;
    }
    return 1;
}

int tp_none(tp_obj v) {
    return v.type.typeid == TP_NONE;
}

/* Function: tp_has
 * Checks if an object contains a key.
 *
 * Returns tp_True if self[k] exists, tp_False otherwise.
 */
tp_obj tp_has(TP,tp_obj self, tp_obj k) {
    int type = self.type.typeid;
    if (type == TP_DICT) {
        if (tpd_dict_hashfind(tp, TPD_DICT(self), tp_hash(tp, k), k) != -1) {
            return tp_True;
        }
        return tp_False;
    } else if (type == TP_STRING && k.type.typeid == TP_STRING) {
        return tp_bool(tp_str_index(self,k)!=-1);
    } else if (type == TP_LIST) {
        return tp_bool(tpd_list_find(tp, TPD_LIST(self), k, tp_equal)!=-1);
    }
    tp_raise(tp_None,tp_string_atom(tp, "(tp_has) TypeError: iterable argument required"));
}

/* Function: tp_del
 * Remove a dictionary entry.
 *
 * Removes the key k from self. Also works on classes and objects.
 *
 * Note that unlike with Python, you cannot use this to remove list items.
 */
void tp_del(TP, tp_obj self, tp_obj k) {
    int type = self.type.typeid;
    if (type == TP_DICT) {
        tp_dict_del(tp, self, k);
        return;
    }
    tp_raise(,tp_string_atom(tp, "(tp_del) TypeError: object does not support item deletion"));
}


/* Function: tp_iter
 * Iterate through a list or dict.
 *
 * If self is a list/string/dictionary, this will iterate over the
 * elements/characters/keys respectively, if k is an increasing index
 * starting with 0 up to the length of the object-1.
 *
 * In the case of a list of string, the returned items will correspond to the
 * item at index k. For a dictionary, no guarantees are made about the order.
 * You also cannot call the function with a specific k to get a specific
 * item -- it is only meant for iterating through all items, calling this
 * function len(self) times. Use <tp_get> to retrieve a specific item, and
 * <tp_len> to get the length.
 *
 * Parameters:
 * self - The object over which to iterate.
 * k - You must pass 0 on the first call, then increase it by 1 after each call,
 *     and don't call the function with k >= len(self).
 *
 * Returns:
 * The first (k = 0) or next (k = 1 .. len(self)-1) item in the iteration.
 */
tp_obj tp_iter(TP,tp_obj self, tp_obj k) {
    int type = self.type.typeid;
    if (type == TP_LIST || type == TP_STRING) { return tp_get(tp,self,k); }
    if (type == TP_DICT && k.type.typeid == TP_NUMBER) {
        return TPD_DICT(self)->items[tpd_dict_next(tp,TPD_DICT(self))].key;
    }
    tp_raise(tp_None,tp_string_atom(tp, "(tp_iter) TypeError: iteration over non-sequence"));
}


/* Function: tp_get
 * Attribute lookup.
 * 
 * This returns the result of using self[k] in actual code. It works for
 * dictionaries (including classes and instantiated objects), lists and strings.
 *
 * As a special case, if self is a list, self[None] will return the first
 * element in the list and subsequently remove it from the list.
 */
static tp_obj
_tp_get(TP, tp_obj self, tp_obj k, int mget);

/* lookup by []. */
tp_obj tp_get(TP, tp_obj self, tp_obj k) {
    return _tp_get(tp, self, k, 0);
}

/* look up by ".". */
tp_obj tp_mget(TP, tp_obj self, tp_obj k) {
    return _tp_get(tp, self, k, 1);
}

tp_obj tp_getraw(TP, tp_obj self) {
    tp_obj r = self;
    r.type.magic = TP_DICT_RAW;
    return r;
}

static void tp_slice_get_indices(TP, tp_obj slice, tp_obj obj, int * start, int * stop) {
    int a, b, l;
    tp_obj tmp;
    l = TPN_AS_INT(tp_len(tp, obj));
    tmp = tp_get(tp, slice, tp_int(0));
    if (tmp.type.typeid == TP_NUMBER) { a = TPN_AS_INT(tmp); }
    else if(tmp.type.typeid == TP_NONE) { a = 0; }
    else { tp_raise_printf(, "(tp_get) TypeError: indices must be numbers"); }
    tmp = tp_get(tp,slice,tp_int(1));
    if (tmp.type.typeid == TP_NUMBER) { b = TPN_AS_INT(tmp); }
    else if(tmp.type.typeid == TP_NONE) { b = l; }
    else { tp_raise_printf(, "(tp_get) TypeError: indices must be numbers"); }

    a = a<0?l+a:a;
    a = a>l?l:a;

    b = b<0?l+b:b;
    b = b>l?l:b;
    *start = a;
    *stop = b;
}

static tp_obj
_tp_get(TP, tp_obj self, tp_obj k, int mget)
{
    /* mget is currently ignored. attributes and items are still treated the same way. */
    int type = self.type.typeid;
    tp_obj r;
    if (type == TP_DICT && self.type.magic != TP_DICT_RAW) {
        if (tp_dict_has(tp, self, k)) {
            return tp_dict_get(tp, self, k);
        }
        /* want to check the raw dict? */
        if (tp_string_equal_atom(k, "__dict__")) {
            return tp_getraw(tp, self);
        }
        /* method? */
        if (_tp_lookup(tp, self, k, &r)) {
            return r;
        }
        /* getter? */
        TP_META_BEGIN(self, __get__);
            return tp_call(tp, __get__, tp_list_v(tp,1,k), tp_None);
        TP_META_END;

        tp_raise_printf(tp_None, "(tpd_dict_get) KeyError: %O", &k);
    } else if (type == TP_DICT && self.type.magic == TP_DICT_RAW) {
        /* raw dict distinguishes [] and . */
        if(mget == 0) {
            if (tp_dict_has(tp, self, k)) {
                return tp_dict_get(tp, self, k);
            }
        } else {
            if (_tp_lookup(tp, self, k, &r)) {
                return r;
            }
        }
        tp_raise_printf(tp_None, "(tpd_dict_get) KeyError: %O", &k);
    } else if (type == TP_LIST) {
        if (k.type.typeid == TP_NUMBER) {
            int l = TPN_AS_INT(tp_len(tp,self));
            int n = TPN_AS_INT(k);
            n = (n<0?l+n:n);
            return tpd_list_get(tp, TPD_LIST(self), n, "tp_get");
        } else if (k.type.typeid == TP_STRING) {
            if (_tp_lookup(tp, self, k, &r)) { return r;}
        } else if (k.type.typeid == TP_LIST) {
            int a, b;
            tp_slice_get_indices(tp, k, self, &a, &b);
            /* FIXME: Unlike CPython, I think indexing shall return a view like numpy. */
            return tp_list_from_items(tp,b-a,&TPD_LIST(self)->items[a]);
        } else if (k.type.typeid == TP_NONE) {
            return tpd_list_pop(tp, TPD_LIST(self), 0, "tp_get");
        }
    } else if (type == TP_STRING) {
        if (k.type.typeid == TP_NUMBER) {
            int l = tp_string_len(self);
            int n = TPN_AS_INT(k);
            n = (n<0?l+n:n);
            if (n >= 0 && n < l) {
                return tp->chars[(unsigned char) tp_string_getptr(self)[n]];
            }
        } else if (k.type.typeid == TP_STRING) {
            if (_tp_lookup(tp, self, k, &r)) { return r;}
        } else if (k.type.typeid == TP_LIST) {
            int a, b;
            tp_slice_get_indices(tp, k, self, &a, &b);
            return tp_string_view(tp,self,a,b);
        }
    } else if (type == TP_FUNC) {
        if (k.type.typeid == TP_STRING) {
            if(tp_string_equal_atom(k, "__args__")) {
                return TPD_FUNC(self)->args;
            } else if(tp_string_equal_atom(k, "__defaults__")) {
                return TPD_FUNC(self)->defaults;
            } else if(tp_string_equal_atom(k, "__varargs__")) {
                return TPD_FUNC(self)->varargs;
            } else if(tp_string_equal_atom(k, "__varkw__")) {
                return TPD_FUNC(self)->varkw;
            }
        }
        tp_raise_printf(tp_None, "(tp_get) TypeError: func does not support item get with key %O", &k);
    }
    /* We use '*' for copy during args handling. See if we can get rid of this after args are fixed. */
    if (k.type.typeid == TP_STRING) {
        return tp_copy(tp, self);
    }
    tp_raise_printf(tp_None, "(tp_get) TypeError: ?");
}

/* function: tp_copy
 * */
tp_obj tp_copy(TP, tp_obj self) {
    int type = self.type.typeid;
    if (type == TP_NUMBER) {
        return self;
    }
    if (type == TP_STRING) {
        return tp_string_from_buffer(tp, tp_string_getptr(self), tp_string_len(self));
    }
    if (type == TP_LIST) {
        return tp_list_copy(tp, self);
    }
    if (type == TP_DICT) {
        return tp_dict_copy(tp, self);
    }
    tp_raise(tp_None, tp_string_atom(tp, "(tp_copy) TypeError: object does not support copy"));
}

/* Function: tp_iget
 *
 * This is used in argument resolution. Therefore we only support
 * list and dict.
 */
int tp_iget(TP, tp_obj *r, tp_obj self, tp_obj k) {
    /* Getting item from None always fails.
     * This is used by param default value handling.
     * TODO(rainwoodman): maybe we can avoid this hack.
     * */
    if (self.type.typeid == TP_NONE) {
        return 0;
    }
    if (self.type.typeid == TP_DICT) {
        int n = tpd_dict_hashfind(tp, TPD_DICT(self), tp_hash(tp, k), k);
        if (n == -1) { return 0; }
        *r = TPD_DICT(self)->items[n].val;
        tp_grey(tp,*r);
        return 1;
    }
    if (self.type.typeid == TP_LIST) {
        if (k.type.typeid == TP_NUMBER) {
            int l = TPD_LIST(self)->len;
            int n = TPN_AS_INT(k);
            if(n >=0 && n < l) {
                *r = tpd_list_get(tp, TPD_LIST(self), n, "tp_iget");
                tp_grey(tp, *r);
                return 1;
            } else {
                return 0;
            }
        }
        if (k.type.typeid == TP_NONE) {
            if(TPD_LIST(self)->len > 0) {
                *r = tpd_list_pop(tp, TPD_LIST(self), 0, "tp_get");
                tp_grey(tp, *r);
                return 1;
            }
            return 0;
        }
    }
    tp_raise(1, tp_string_atom(tp, "(tp_iget) TypeError: object type not supported"));
}

/* Function: tp_set
 * Attribute modification.
 * 
 * This is the counterpart of tp_get, it does the same as self[k] = v would do
 * in actual tinypy code.
 */
void tp_set(TP,tp_obj self, tp_obj k, tp_obj v) {
    int type = self.type.typeid;

    if (type == TP_DICT) {
        TP_META_BEGIN(self, __set__);
            /* unhandled case returns true, and uses the default. */
            if(tp_true(tp, tp_call(tp, __set__, tp_list_v(tp,2,k,v), tp_None))) {
                tp_dict_set(tp, self, k, v);
            }
            return;
        TP_META_END;
        tp_dict_set(tp, self, k, v);
        return;
    } else if (type == TP_LIST) {
        if (k.type.typeid == TP_NUMBER) {
            tpd_list_set(tp, TPD_LIST(self), TPN_AS_INT(k), v, "tp_set");
            return;
        } else if (k.type.typeid == TP_NONE) {
            tpd_list_append(tp, TPD_LIST(self), v);
            return;
        }
    } else if (type == TP_FUNC) {
        if (k.type.typeid == TP_STRING) {
            if(tp_string_equal_atom(k, "__args__")) {
                TPD_FUNC(self)->args = v;
                return;
            } else if(tp_string_equal_atom(k, "__defaults__")) {
                TPD_FUNC(self)->defaults = v;
                return;
            } else if(tp_string_equal_atom(k, "__varargs__")) {
                TPD_FUNC(self)->varargs = v;
                return;
            } else if(tp_string_equal_atom(k, "__varkw__")) {
                TPD_FUNC(self)->varkw = v;
                return;
            }
        }
        tp_raise_printf(,"(tp_set) TypeError: func does not support item assignment with key %O", &k);
    }
    tp_raise(,tp_string_atom(tp, "(tp_set) TypeError: object does not support item assignment"));
}

tp_obj tp_add(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid == TP_NUMBER && a.type.typeid == b.type.typeid) {
        switch(tp_number_upcast(tp, &a, &b)) {
            case TP_NUMBER_INT:
                return tp_int(TPN_AS_INT(a) + TPN_AS_INT(b));
            case TP_NUMBER_FLOAT:
                return tp_float(TPN_AS_FLOAT(a) + TPN_AS_FLOAT(b));
            default: abort();
        }
    } else if (a.type.typeid == TP_STRING && a.type.typeid == b.type.typeid) {
        return tp_string_add(tp, a, b);
    } else if (a.type.typeid == TP_LIST && a.type.typeid == b.type.typeid) {
        return tp_list_add(tp, a, b);
    } else if (a.type.typeid == TP_DICT && a.type.typeid == b.type.typeid) {
        tp_obj r = tp_dict_copy(tp, a);
        tp_dict_update(tp, r, b);
        return r;
    }
    tp_raise(tp_None,tp_string_atom(tp, "(tp_add) TypeError: ?"));
}

tp_obj tp_mul(TP,tp_obj a, tp_obj b) {
    if (a.type.typeid == TP_NUMBER && a.type.typeid == b.type.typeid) {
        switch(tp_number_upcast(tp, &a, &b)) {
            case TP_NUMBER_INT:
                return tp_int(TPN_AS_INT(a) * TPN_AS_INT(b));
            case TP_NUMBER_FLOAT:
                return tp_float(TPN_AS_FLOAT(a) * TPN_AS_FLOAT(b));
            default: abort();
        }
    }
    if(a.type.typeid == TP_STRING && b.type.typeid == TP_NUMBER) {
        int n = TPN_AS_INT(b);
        return tp_string_mul(tp, a, n);
    }
    if(a.type.typeid == TP_LIST && b.type.typeid == TP_NUMBER) {
        int n = TPN_AS_INT(b);
        return tp_list_mul(tp, a, n);
    }
    tp_raise(tp_None,tp_string_atom(tp, "(tp_mul) TypeError: ?"));
}

tp_obj tp_mod(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid == TP_NUMBER && a.type.typeid == b.type.typeid) {
        switch(tp_number_upcast(tp, &a, &b)) {
            case TP_NUMBER_INT:
                return tp_int(TPN_AS_INT(a) % TPN_AS_INT(b));
            case TP_NUMBER_FLOAT:
                return tp_float(fmod(TPN_AS_FLOAT(a), TPN_AS_FLOAT(b)));
            default: abort();
        }
    }
    if(a.type.typeid == TP_STRING) {
            TP_META_BEGIN(a, format);
            return tp_call(tp, format, tp_list_v(tp, 1, b), tp_None);
            TP_META_END;
    }
    tp_raise(tp_None, tp_string_atom(tp, "(tp_mod) TypeError: ?"));
}

tp_obj tp_sub(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid == TP_NUMBER && a.type.typeid == b.type.typeid) {
        switch(tp_number_upcast(tp, &a, &b)) {
            case TP_NUMBER_INT:
                return tp_int(TPN_AS_INT(a) - TPN_AS_INT(b));
            case TP_NUMBER_FLOAT:
                return tp_float(TPN_AS_FLOAT(a) - TPN_AS_FLOAT(b));
            default: abort();
        }
    }
    tp_raise(tp_None, tp_string_atom(tp, "(tp_sub) TypeError: ?"));
}

tp_obj tp_div(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid == TP_NUMBER && a.type.typeid == b.type.typeid) {
        switch(tp_number_upcast(tp, &a, &b)) {
            case TP_NUMBER_INT:
                return tp_int(TPN_AS_INT(a) / TPN_AS_INT(b));
            case TP_NUMBER_FLOAT:
                return tp_float(TPN_AS_FLOAT(a) / TPN_AS_FLOAT(b));
            default: abort();
        }
    }
    tp_raise(tp_None, tp_string_atom(tp, "(tp_div) TypeError: ?"));
}

tp_obj tp_pow(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid == TP_NUMBER && a.type.typeid == b.type.typeid) {
        switch(tp_number_upcast(tp, &a, &b)) {
            case TP_NUMBER_INT:
                return tp_int(pow(TPN_AS_INT(a), TPN_AS_INT(b)));
            case TP_NUMBER_FLOAT:
                return tp_float(pow(TPN_AS_FLOAT(a), TPN_AS_FLOAT(b)));
            default: abort();
        }
    }
    tp_raise(tp_None, tp_string_atom(tp, "(tp_div) TypeError: ?"));
}

/* Function: tp_len
 * Returns the length of an object.
 *
 * Returns the number of items in a list or dict, or the length of a string.
 */
tp_obj tp_len(TP,tp_obj self) {
    int type = self.type.typeid;
    if (type == TP_STRING) {
        return tp_int(tp_string_len(self));
    } else if (type == TP_DICT) {
        return tp_int(TPD_DICT(self)->len);
    } else if (type == TP_LIST) {
        return tp_int(TPD_LIST(self)->len);
    }
    
    tp_raise(tp_None,tp_string_atom(tp, "(tp_len) TypeError: len() of unsized object"));
}

int tp_equal(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid != b.type.typeid) { return 0;}
    switch(a.type.typeid) {
        case TP_NONE: return 1;
        case TP_NUMBER: 
            switch(tp_number_upcast(tp, &a, &b)) {
                case TP_NUMBER_INT:
                    return TPN_AS_INT(a) == TPN_AS_INT(b);
                case TP_NUMBER_FLOAT:
                    return TPN_AS_FLOAT(a) == TPN_AS_FLOAT(b);
                default: abort();
            }
        case TP_STRING: return tp_string_cmp(a, b) == 0;
        case TP_LIST: return tp_list_equal(tp, a, b);
        case TP_DICT: return tp_dict_equal(tp, a, b);
        case TP_FUNC: return TPD_FUNC(a) == TPD_FUNC(b);
        case TP_DATA: return (char*)a.ptr == (char*)b.ptr;
    }
    tp_raise(0,tp_string_atom(tp, "(tp_equal) TypeError: Unknown types."));
}

int tp_lessthan(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid != b.type.typeid) { 
        tp_raise(0,tp_string_atom(tp, "(tp_lessthan) TypeError: Cannot compare different types."));
    }
    switch(a.type.typeid) {
        case TP_NONE: return 0;
        case TP_NUMBER:
            switch(tp_number_upcast(tp, &a, &b)) {
                case TP_NUMBER_INT:
                    return TPN_AS_INT(a) < TPN_AS_INT(b);
                case TP_NUMBER_FLOAT:
                    return TPN_AS_FLOAT(a) < TPN_AS_FLOAT(b);
                default: abort();
            }
        case TP_STRING: return tp_string_cmp(a, b) < 0;
        case TP_LIST: return tp_list_lessthan(tp, a, b);
        case TP_DICT: {
            tp_raise(0,tp_string_atom(tp, "(tp_lessthan) TypeError: Cannot compare dict."));
        }
        case TP_FUNC: return TPD_FUNC(a) < TPD_FUNC(b);
        case TP_DATA: return (char*) a.ptr < (char*) b.ptr;
    }
    tp_raise(0,tp_string_atom(tp, "(tp_lessthan) TypeError: Unknown types."));
}


#define TP_DEF_OP_INT(name, expr) \
    tp_obj name(TP,tp_obj _a,tp_obj _b) { \
    if (_a.type.typeid == TP_NUMBER && _b.type.typeid == TP_NUMBER) { \
        long a = TPN_AS_INT(_a); long b = TPN_AS_INT(_b); \
        return tp_int(expr); \
    } \
    tp_raise(tp_None,tp_string_atom(tp, "(" #name ") TypeError: unsupported operand type(s)")); \
}

TP_DEF_OP_INT(tp_bitwise_and, a & b);
TP_DEF_OP_INT(tp_bitwise_or, a | b);
TP_DEF_OP_INT(tp_bitwise_xor, a ^ b);
TP_DEF_OP_INT(tp_lsh, a << b);
TP_DEF_OP_INT(tp_rsh, a >> b);

tp_obj tp_bitwise_not(TP, tp_obj a) {
    if (a.type.typeid == TP_NUMBER) {
        return tp_int(~TPN_AS_INT(a));
    }
    tp_raise(tp_None,tp_string_atom(tp, "(tp_bitwise_not) TypeError: unsupported operand type"));
}

/* Function: tp_call
 * Calls a tinypy function.
 *
 * Use this to call a tinypy function.
 *
 * Parameters:
 * tp - The VM instance.
 * self - The object to call.
 * params - Parameters to pass.
 *
 * Example:
 * > tp_call(tp,
 * >     tp_get(tp, tp->builtins, tp_string_atom(tp, "foo")),
 * >     tp_params_v(tp, tp_string_atom(tp, "hello")))
 * This will look for a global function named "foo", then call it with a single
 * positional parameter containing the string "hello".
 */
tp_obj tp_call(TP, tp_obj self, tp_obj lparams, tp_obj dparams) {
    if (self.type.typeid == TP_DICT) {
        if (self.type.magic == TP_DICT_CLASS) {
            TP_META_BEGIN_CLASS(self, __new__)
                tpd_list_insert(tp, TPD_LIST(lparams), 0, self);
                return tp_call(tp, __new__, lparams, dparams);
            TP_META_END_CLASS;
        } else if (self.type.magic == TP_DICT_OBJECT) {
            TP_META_BEGIN(self, __call__);
                return tp_call(tp, __call__, lparams, dparams);
            TP_META_END;
        }
    }

    if (self.type.typeid == TP_FUNC) {
        if (self.type.mask & TP_FUNC_MASK_METHOD) {
            if (lparams.type.typeid == TP_NONE) {
                lparams = tp_list_t(tp);
            }
            /* method, add instance */
            tpd_list_insert(tp, TPD_LIST(lparams), 0, TPD_FUNC(self)->instance);
        }
        if(self.ptr != NULL) {
            /* C func, set tp->lparams for the CAPI calling convention. */
            *tp->lparams = lparams;
            *tp->dparams = dparams;

            tp_obj (* cfunc)(tp_vm *);
            cfunc = self.ptr;
            tp_obj r = cfunc(tp);
            tp_grey(tp, r);
            return r;
        } else {
            /* compiled Python function */
            tp_obj dest = tp_None;
            tp_enter_frame(tp, lparams, dparams,
                           TPD_FUNC(self)->globals,
                           TPD_FUNC(self)->code,
                           TPD_FUNC(self)->args,
                           TPD_FUNC(self)->defaults,
                           &dest);
            tp_run_frame(tp);
            return dest;
        }
    }
    tp_echo(tp, self);
    tp_raise(tp_None,tp_string_atom(tp, "(tp_call) TypeError: object is not callable"));
}

void tp_assert(TP, tp_obj r, tp_obj b, tp_obj c)
{
    if (tp_true(tp, r)) { return; }
    tp_obj msg = tp_string_atom(tp, "(tp_assert) AssertionError: Failure");
    msg = tp_add(tp, msg, tp_string_atom(tp, "\nLeft Side : "));
    msg = tp_add(tp, msg, tp_repr(tp, b));
    msg = tp_add(tp, msg, tp_string_atom(tp, "\nRight Side : "));
    msg = tp_add(tp, msg, tp_repr(tp, c));
    tp_raise(, msg);
}
/**/

