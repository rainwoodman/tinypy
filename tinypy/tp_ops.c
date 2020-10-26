
/* Function: tp_true
 * Check the truth value of an object
 *
 * Returns false if v is a numeric object with a value of exactly 0, v is of
 * type None or v is a string list or dictionary with a length of 0. Else true
 * is returned.
 */
int tp_true(TP, tp_obj v) {
    switch(v.type.typeid) {
        case TP_NUMBER: return v.number.val != 0;
        case TP_NONE: return 0;
        case TP_STRING: return tp_string_len(v) != 0;
        case TP_LIST: return v.list.val->len != 0;
        case TP_DICT: return v.dict.val->len != 0;
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
        if (tpd_dict_hashfind(tp, self.dict.val, tp_hash(tp, k), k) != -1) {
            return tp_True;
        }
        return tp_False;
    } else if (type == TP_STRING && k.type.typeid == TP_STRING) {
        return tp_number(tp_str_index(self,k)!=-1);
    } else if (type == TP_LIST) {
        return tp_number(tpd_list_find(tp, self.list.val, k, tp_equal)!=-1);
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
        return self.dict.val->items[tpd_dict_next(tp,self.dict.val)].key;
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
    l = tp_len(tp, obj).number.val;
    tmp = tp_get(tp, slice, tp_number(0));
    if (tmp.type.typeid == TP_NUMBER) { a = tmp.number.val; }
    else if(tmp.type.typeid == TP_NONE) { a = 0; }
    else { tp_raise_printf(, "(tp_get) TypeError: indices must be numbers"); }
    tmp = tp_get(tp,slice,tp_number(1));
    if (tmp.type.typeid == TP_NUMBER) { b = tmp.number.val; }
    else if(tmp.type.typeid == TP_NONE) { b = l; }
    else { tp_raise_printf(, "(tp_get) TypeError: indices must be numbers"); }
    a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
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
            return tp_call(tp, __get__, tp_params_v(tp,1,k));
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
            int l = tp_len(tp,self).number.val;
            int n = k.number.val;
            n = (n<0?l+n:n);
            return tpd_list_get(tp, self.list.val, n, "tp_get");
        } else if (k.type.typeid == TP_STRING) {
            if (_tp_lookup(tp, self, k, &r)) { return r;}
        } else if (k.type.typeid == TP_LIST) {
            int a, b;
            tp_slice_get_indices(tp, k, self, &a, &b);
            return tp_list_from_items(tp,b-a,&self.list.val->items[a]);
        } else if (k.type.typeid == TP_NONE) {
            return tpd_list_pop(tp, self.list.val, 0, "tp_get");
        }
    } else if (type == TP_STRING) {
        if (k.type.typeid == TP_NUMBER) {
            int l = tp_string_len(self);
            int n = k.number.val;
            n = (n<0?l+n:n);
            if (n >= 0 && n < l) { return tp_string_t_from_const(tp, tp->chars[(unsigned char) tp_string_getptr(self)[n]], 1); }
        } else if (k.type.typeid == TP_STRING) {
            if (_tp_lookup(tp, self, k, &r)) { return r;}
        } else if (k.type.typeid == TP_LIST) {
            int a, b;
            tp_slice_get_indices(tp, k, self, &a, &b);
            return tp_string_view(tp,self,a,b);
        }
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
 * Failsafe attribute lookup.
 *
 * This is like <tp_get>, except it will return false if the attribute lookup
 * failed. Otherwise, it will return true, and the object will be returned
 * over the reference parameter r.
 */
int tp_iget(TP,tp_obj *r, tp_obj self, tp_obj k) {
    if (self.type.typeid == TP_DICT) {
        int n = tpd_dict_hashfind(tp, self.dict.val, tp_hash(tp, k), k);
        if (n == -1) { return 0; }
        *r = self.dict.val->items[n].val;
        tp_grey(tp,*r);
        return 1;
    }
    if (self.type.typeid == TP_LIST && !self.list.val->len) { return 0; }
    *r = tp_get(tp,self,k); tp_grey(tp,*r);
    return 1;
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
            if(tp_true(tp, tp_call(tp,__set__,tp_params_v(tp,2,k,v)))) {
                tp_dict_set(tp, self, k, v);
            }
            return;
        TP_META_END;
        tp_dict_set(tp, self, k, v);
        return;
    } else if (type == TP_LIST) {
        if (k.type.typeid == TP_NUMBER) {
            tpd_list_set(tp, self.list.val, k.number.val, v, "tp_set");
            return;
        } else if (k.type.typeid == TP_NONE) {
            tpd_list_append(tp, self.list.val, v);
            return;
        }
    }
    tp_raise(,tp_string_atom(tp, "(tp_set) TypeError: object does not support item assignment"));
}

tp_obj tp_add(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid == TP_NUMBER && a.type.typeid == b.type.typeid) {
        return tp_number(a.number.val+b.number.val);
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
        return tp_number(a.number.val*b.number.val);
    }
    if(a.type.typeid == TP_NUMBER) {
        tp_obj c = a; a = b; b = c;
    }
    if(a.type.typeid == TP_STRING && b.type.typeid == TP_NUMBER) {
        int n = b.number.val;
        return tp_string_mul(tp, a, n);
    }
    if(a.type.typeid == TP_LIST && b.type.typeid == TP_NUMBER) {
        int n = b.number.val;
        return tp_list_mul(tp, a, n);
    }
    tp_raise(tp_None,tp_string_atom(tp, "(tp_mul) TypeError: ?"));
}

/* Function: tp_len
 * Returns the length of an object.
 *
 * Returns the number of items in a list or dict, or the length of a string.
 */
tp_obj tp_len(TP,tp_obj self) {
    int type = self.type.typeid;
    if (type == TP_STRING) {
        return tp_number(tp_string_len(self));
    } else if (type == TP_DICT) {
        return tp_number(self.dict.val->len);
    } else if (type == TP_LIST) {
        return tp_number(self.list.val->len);
    }
    
    tp_raise(tp_None,tp_string_atom(tp, "(tp_len) TypeError: len() of unsized object"));
}

int tp_equal(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid != b.type.typeid) { return 0;}
    switch(a.type.typeid) {
        case TP_NONE: return 1;
        case TP_NUMBER: return a.number.val == b.number.val;
        case TP_STRING: return tp_string_cmp(a, b) == 0;
        case TP_LIST: return tp_list_equal(tp, a, b);
        case TP_DICT: return tp_dict_equal(tp, a, b);
        case TP_FUNC: return a.func.info == b.func.info;
        case TP_DATA: return (char*)a.data.val == (char*)b.data.val;
    }
    tp_raise(0,tp_string_atom(tp, "(tp_equal) TypeError: Unknown types."));
}

int tp_lessthan(TP, tp_obj a, tp_obj b) {
    if (a.type.typeid != b.type.typeid) { 
        tp_raise(0,tp_string_atom(tp, "(tp_lessthan) TypeError: Cannot compare different types."));
    }
    switch(a.type.typeid) {
        case TP_NONE: return 0;
        case TP_NUMBER: return a.number.val < b.number.val;
        case TP_STRING: return tp_string_cmp(a, b) < 0;
        case TP_LIST: return tp_list_lessthan(tp, a, b);
        case TP_DICT: {
            tp_raise(0,tp_string_atom(tp, "(tp_lessthan) TypeError: Cannot compare dict."));
        }
        case TP_FUNC: return a.func.info < b.func.info;
        case TP_DATA: return (char*)a.data.val < (char*)b.data.val;
    }
    tp_raise(0,tp_string_atom(tp, "(tp_lessthan) TypeError: Unknown types."));
}

tp_obj tp_mod(TP, tp_obj a, tp_obj b) {
    switch(a.type.typeid) {
        case TP_NUMBER:
            if(b.type.typeid == TP_NUMBER)
                return tp_number(((long)a.number.val) % ((long)b.number.val));
            break;
        case TP_STRING:
            TP_META_BEGIN(a, format);
            return tp_call(tp, format, tp_params_v(tp, 1, b));
            TP_META_END;
    }
    tp_raise(tp_None, tp_string_atom(tp, "(tp_mod) TypeError: ?"));
}

#define TP_OP(name,expr) \
    tp_obj name(TP,tp_obj _a,tp_obj _b) { \
    if (_a.type.typeid == TP_NUMBER && _a.type.typeid == _b.type.typeid) { \
        tp_num a = _a.number.val; tp_num b = _b.number.val; \
        return tp_number(expr); \
    } \
    tp_raise(tp_None,tp_string_atom(tp, "(" #name ") TypeError: unsupported operand type(s)")); \
}

TP_OP(tp_bitwise_and,((long)a)&((long)b));
TP_OP(tp_bitwise_or,((long)a)|((long)b));
TP_OP(tp_bitwise_xor,((long)a)^((long)b));
TP_OP(tp_lsh,((long)a)<<((long)b));
TP_OP(tp_rsh,((long)a)>>((long)b));
TP_OP(tp_sub,a-b);
TP_OP(tp_div,a/b);
TP_OP(tp_pow,pow(a,b));

tp_obj tp_bitwise_not(TP, tp_obj a) {
    if (a.type.typeid == TP_NUMBER) {
        return tp_number(~(long)a.number.val);
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
tp_obj tp_call(TP, tp_obj self, tp_obj params) {
    if (self.type.typeid == TP_DICT) {
        if (self.type.magic == TP_DICT_CLASS) {
            TP_META_BEGIN_CLASS(self, __new__)
                tpd_list_insert(tp, params.list.val, 0, self);
                return tp_call(tp, __new__, params);
            TP_META_END_CLASS;
        } else if (self.type.magic == TP_DICT_OBJECT) {
            TP_META_BEGIN(self, __call__);
                return tp_call(tp, __call__, params);
            TP_META_END;
        }
    }

    if (self.type.typeid == TP_FUNC) {
        if (self.type.mask & TP_FUNC_MASK_METHOD) {
            /* method, add instance */
            tpd_list_insert(tp, params.list.val, 0, self.func.info->instance);
        }
        if(self.func.cfnc != NULL) {
            /* C func, set tp->params for the CAPI calling convention. */
            tp->params = params;

            tp_obj (* cfunc)(tp_vm *);
            cfunc = self.func.cfnc;

            tp_obj r = cfunc(tp);
            tp_grey(tp, r);
            return r;
        } else {
            /* compiled Python function */
            tp_obj dest = tp_None;
            tp_enter_frame(tp, self.func.info->globals,
                               self.func.info->code,
                              &dest);
            tp->frames[tp->cur].regs[0] = params;
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
