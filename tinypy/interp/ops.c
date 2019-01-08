/* File: Operations
 * Various tinypy operations.
 */

tp_obj tp_str_(TP, tp_obj self, tp_obj visited);
tp_obj tp_has(TP,tp_obj self, tp_obj k);

tp_obj tp_repr_(TP, tp_obj self, tp_obj visited) {
    TP_META_BEGIN(self,"__repr__");
        return tp_call(tp,meta,tp_params(tp));
    TP_META_END;
    if(self.type == TP_STRING) {
        tp_params_v(tp, 3, self, tp_string("'"), tp_string("\\'"));
        tp_obj replaced = tpy_str_replace(tp);
        return tp_printf_tracked(tp, "\'%s\'", replaced.string.val);
    }
    return tp_str_(tp, self, visited);
}

tp_obj tp_repr(TP, tp_obj self) {
    tp_obj visited = tpy_list(tp);
    tp_obj result = tp_repr_(tp, self, visited);
    return result;
}

/* Function: tp_str
 * String representation of an object.
 * Checks for recursive data structures
 *
 * Returns a string object representating self.
 */
tp_obj tp_str_(TP, tp_obj self, tp_obj visited) {
    /* if the class has __str__ or __repr__ use those */
    TP_META_BEGIN(self,"__str__");
        return tp_call(tp,meta,tp_params(tp));
    TP_META_END;
    TP_META_BEGIN(self,"__repr___");
        return tp_call(tp,meta,tp_params(tp));
    TP_META_END;
    int type = self.type;
    if(type == TP_DICT) {
        if(tp_has(tp, visited, tpy_data(tp, 0, self.dict.val)).number.val) return tp_string("{...}");
        tpd_list_append(tp, visited.list.val, tpy_data(tp, 0, self.dict.val));
    } else if(type == TP_LIST) {
        if(tp_has(tp, visited, tpy_data(tp, 0, self.list.val)).number.val) return tp_string("[...]");
        tpd_list_append(tp, visited.list.val, tpy_data(tp, 0, self.list.val));
    }
    tp_obj result = tp_None;
    if (type == TP_STRING) { 
        result = self; 
    } else if (type == TP_NUMBER) {
        tp_num v = self.number.val;
        if ((fabs(v-(long)v)) < 0.000001) { return tp_printf_tracked(tp,"%ld",(long)v); }
        result = tp_printf_tracked(tp,"%f",v);
    } else if(type == TP_DICT) {
        result = tp_string("{");
        int i, n = 0;
        for(i = 0; i < self.dict.val->alloc; i++) {
            if(self.dict.val->items[i].used > 0) {
                result = tp_add(tp, result, tp_repr_(tp, self.dict.val->items[i].key, visited));
                result = tp_add(tp, result, tp_string(": "));
                result = tp_add(tp, result, tp_repr_(tp, self.dict.val->items[i].val, visited));
                if(n < self.dict.val->len - 1) result = tp_add(tp, result, tp_string(", "));
                n += 1;
            }
        }
        result = tp_add(tp, result, tp_string("}"));
        /*result = tp_printf_tracked(tp,"<dict 0x%x>",self.dict.val);*/
    } else if(type == TP_LIST) {
        result = tp_string("[");
        int i;
        for(i = 0; i < self.list.val->len; i++) {
            result = tp_add(tp, result, tp_repr_(tp, self.list.val->items[i], visited));
            if(i < self.list.val->len - 1) result = tp_add(tp, result, tp_string(", "));
        }
        result = tp_add(tp, result, tp_string("]"));
        /*result = tp_printf_tracked(tp,"<list 0x%x>",self.list.val);*/
    } else if (type == TP_NONE) {
        result = tp_string("None");
    } else if (type == TP_DATA) {
        result = tp_printf_tracked(tp,"<data 0x%x>",self.data.val);
    } else if (type == TP_FNC) {
        result = tp_printf_tracked(tp,"<fnc 0x%x>",self.fnc.info);
    } else {
        result = tp_string("<?>");
    }
    if(type == TP_DICT || type == TP_LIST) {
        tpd_list_pop(tp, visited.list.val, visited.list.val->len - 1, "visited list is empty");
    }
    return result;
}

tp_obj tp_str(TP, tp_obj self) {
    tp_obj visited = tpy_list(tp);
    tp_obj result = tp_str_(tp, self, visited);
    return result;
}

tp_obj tp_str_old(TP,tp_obj self) {
    int type = self.type;
    if (type == TP_STRING) { return self; }
    if (type == TP_NUMBER) {
        tp_num v = self.number.val;
        if ((fabs(v)-fabs((long)v)) < 0.000001) { return tp_printf_tracked(tp,"%ld",(long)v); }
        return tp_printf_tracked(tp,"%f",v);
    } else if(type == TP_DICT) {
        return tp_printf_tracked(tp,"<dict 0x%x>",self.dict.val);
    } else if(type == TP_LIST) {
        return tp_printf_tracked(tp,"<list 0x%x>",self.list.val);
    } else if (type == TP_NONE) {
        return tp_string("None");
    } else if (type == TP_DATA) {
        return tp_printf_tracked(tp,"<data 0x%x>",self.data.val);
    } else if (type == TP_FNC) {
        return tp_printf_tracked(tp,"<fnc 0x%x>",self.fnc.info);
    }
    return tp_string("<?>");
}


/* Function: tp_true
 * Check the truth value of an object
 *
 * Returns false if v is a numeric object with a value of exactly 0, v is of
 * type None or v is a string list or dictionary with a length of 0. Else true
 * is returned.
 */
int tp_true(TP,tp_obj v) {
    switch(v.type) {
        case TP_NUMBER: return v.number.val != 0;
        case TP_NONE: return 0;
        case TP_STRING: return v.string.len != 0;
        case TP_LIST: return v.list.val->len != 0;
        case TP_DICT: return v.dict.val->len != 0;
    }
    return 1;
}


/* Function: tp_has
 * Checks if an object contains a key.
 *
 * Returns tp_True if self[k] exists, tp_False otherwise.
 */
tp_obj tp_has(TP,tp_obj self, tp_obj k) {
    int type = self.type;
    if (type == TP_DICT) {
        if (tpd_dict_hashfind(tp, self.dict.val, tp_hash(tp, k), k) != -1) {
            return tp_True;
        }
        return tp_False;
    } else if (type == TP_STRING && k.type == TP_STRING) {
        return tp_number(_tp_str_index(self,k)!=-1);
    } else if (type == TP_LIST) {
        return tp_number(tpd_list_find(tp,self.list.val,k)!=-1);
    }
    tp_raise(tp_None,tp_string("(tp_has) TypeError: iterable argument required"));
}

/* Function: tp_del
 * Remove a dictionary entry.
 *
 * Removes the key k from self. Also works on classes and objects.
 *
 * Note that unlike with Python, you cannot use this to remove list items.
 */
void tp_del(TP, tp_obj self, tp_obj k) {
    int type = self.type;
    if (type == TP_DICT) {
        tp_dict_del(tp, self, k);
        return;
    }
    tp_raise(,tp_string("(tp_del) TypeError: object does not support item deletion"));
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
    int type = self.type;
    if (type == TP_LIST || type == TP_STRING) { return tp_get(tp,self,k); }
    if (type == TP_DICT && k.type == TP_NUMBER) {
        return self.dict.val->items[tpd_dict_next(tp,self.dict.val)].key;
    }
    tp_raise(tp_None,tp_string("(tp_iter) TypeError: iteration over non-sequence"));
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
tp_obj tp_get(TP, tp_obj self, tp_obj k) {
    int type = self.type;
    tp_obj r;
    if (type == TP_DICT) {
        TP_META_BEGIN(self,"__get__");
            return tp_call(tp,meta,tp_params_v(tp,1,k));
        TP_META_END;
        if (self.dict.dtype && _tp_lookup(tp,self,k,&r)) { return r; }
        return tp_dict_get(tp, self, k);
    } else if (type == TP_LIST) {
        if (k.type == TP_NUMBER) {
            int l = tp_len(tp,self).number.val;
            int n = k.number.val;
            n = (n<0?l+n:n);
            return tpd_list_get(tp, self.list.val, n, "tp_get");
        } else if (k.type == TP_STRING) {
            /* FIXME: move these to the prototype object of list, after
             * adding meta to all objects */
            if (tp_cmp(tp,tp_string("append"),k) == 0) {
                return tpy_method(tp, self, tpy_list_append);
            } else if (tp_cmp(tp,tp_string("pop"),k) == 0) {
                return tpy_method(tp, self,tpy_list_pop);
            } else if (tp_cmp(tp,tp_string("index"),k) == 0) {
                return tpy_method(tp, self,tpy_list_index);
            } else if (tp_cmp(tp,tp_string("sort"),k) == 0) {
                return tpy_method(tp, self,tpy_list_sort);
            } else if (tp_cmp(tp,tp_string("extend"),k) == 0) {
                return tpy_method(tp, self,tpy_list_extend);
            } else if (tp_cmp(tp,tp_string("*"),k) == 0) {
                tp_params_v(tp,1,self);
                r = tpy_copy(tp);
                self.list.val->len=0;
                return r;
            }
        } else if (k.type == TP_NONE) {
            return tpd_list_pop(tp, self.list.val, 0, "tp_get");
        }
    } else if (type == TP_STRING) {
        if (k.type == TP_NUMBER) {
            int l = self.string.len;
            int n = k.number.val;
            n = (n<0?l+n:n);
            if (n >= 0 && n < l) { return tp_string_n(tp->chars[(unsigned char)self.string.val[n]],1); }
        } else if (k.type == TP_STRING) {
            if (tp_cmp(tp,tp_string("join"),k) == 0) {
                return tpy_method(tp,self,tpy_str_join);
            } else if (tp_cmp(tp,tp_string("split"),k) == 0) {
                return tpy_method(tp,self,tpy_str_split);
            } else if (tp_cmp(tp,tp_string("index"),k) == 0) {
                return tpy_method(tp,self,tpy_str_index);
            } else if (tp_cmp(tp,tp_string("strip"),k) == 0) {
                return tpy_method(tp,self,tpy_str_strip);
            } else if (tp_cmp(tp,tp_string("replace"),k) == 0) {
                return tpy_method(tp,self,tpy_str_replace);
            }
        }
    }

    if (k.type == TP_LIST) {
        int a,b,l;
        tp_obj tmp;
        l = tp_len(tp,self).number.val;
        tmp = tp_get(tp,k,tp_number(0));
        if (tmp.type == TP_NUMBER) { a = tmp.number.val; }
        else if(tmp.type == TP_NONE) { a = 0; }
        else { tp_raise(tp_None,tp_string("(tp_get) TypeError: indices must be numbers")); }
        tmp = tp_get(tp,k,tp_number(1));
        if (tmp.type == TP_NUMBER) { b = tmp.number.val; }
        else if(tmp.type == TP_NONE) { b = l; }
        else { tp_raise(tp_None,tp_string("(tp_get) TypeError: indices must be numbers")); }
        a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
        if (type == TP_LIST) {
            return tpy_list_n(tp,b-a,&self.list.val->items[a]);
        } else if (type == TP_STRING) {
            return tp_string_sub(tp,self,a,b);
        }
    }

    tp_raise(tp_None,tp_string("(tp_get) TypeError: ?"));
}

/* Function: tp_iget
 * Failsafe attribute lookup.
 *
 * This is like <tp_get>, except it will return false if the attribute lookup
 * failed. Otherwise, it will return true, and the object will be returned
 * over the reference parameter r.
 */
int tp_iget(TP,tp_obj *r, tp_obj self, tp_obj k) {
    if (self.type == TP_DICT) {
        int n = tpd_dict_hashfind(tp, self.dict.val, tp_hash(tp, k), k);
        if (n == -1) { return 0; }
        *r = self.dict.val->items[n].val;
        tp_grey(tp,*r);
        return 1;
    }
    if (self.type == TP_LIST && !self.list.val->len) { return 0; }
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
    int type = self.type;

    if (type == TP_DICT) {
        TP_META_BEGIN(self,"__set__");
            tp_call(tp,meta,tp_params_v(tp,2,k,v));
            return;
        TP_META_END;

        tpd_dict_hashset(tp, self.dict.val, tp_hash(tp, k), k, v);
        return;
    } else if (type == TP_LIST) {
        if (k.type == TP_NUMBER) {
            tpd_list_set(tp, self.list.val, k.number.val, v, "tp_set");
            return;
        } else if (k.type == TP_NONE) {
            tpd_list_append(tp, self.list.val, v);
            return;
        } else if (k.type == TP_STRING) {
            /* WTF is this syntax? a['*'] = b will extend a by b ??
             * FIXME: remove this support. Use a + b */
            if (tp_cmp(tp, tp_string("*"), k) == 0) {
                tpd_list_extend(tp, self.list.val, v.list.val);
                return;
            }
        }
    }
    tp_raise(,tp_string("(tp_set) TypeError: object does not support item assignment"));
}

tp_obj tp_add(TP, tp_obj a, tp_obj b) {
    if (a.type == TP_NUMBER && a.type == b.type) {
        return tp_number(a.number.val+b.number.val);
    } else if (a.type == TP_STRING && a.type == b.type) {
        return tp_track(tp, tp_string_add(tp, a, b));
    } else if (a.type == TP_LIST && a.type == b.type) {
        return tp_track(tp, tp_list_add(tp, a, b));
    }
    tp_raise(tp_None,tp_string("(tp_add) TypeError: ?"));
}

tp_obj tp_mul(TP,tp_obj a, tp_obj b) {
    if (a.type == TP_NUMBER && a.type == b.type) {
        return tp_number(a.number.val*b.number.val);
    }
    if(a.type == TP_NUMBER) {
        tp_obj c = a; a = b; b = c;
    }
    if(a.type == TP_STRING && b.type == TP_NUMBER) {
        int n = b.number.val;
        return tp_track(tp, tp_string_mul(tp, a, n));
    }
    if(a.type == TP_LIST && b.type == TP_NUMBER) {
        int n = b.number.val;
        return tp_track(tp, tp_list_mul(tp, a, n));
    }
    tp_raise(tp_None,tp_string("(tp_mul) TypeError: ?"));
}

/* Function: tp_len
 * Returns the length of an object.
 *
 * Returns the number of items in a list or dict, or the length of a string.
 */
tp_obj tp_len(TP,tp_obj self) {
    int type = self.type;
    if (type == TP_STRING) {
        return tp_number(self.string.len);
    } else if (type == TP_DICT) {
        return tp_number(self.dict.val->len);
    } else if (type == TP_LIST) {
        return tp_number(self.list.val->len);
    }
    
    tp_raise(tp_None,tp_string("(tp_len) TypeError: len() of unsized object"));
}

int tp_cmp(TP, tp_obj a, tp_obj b) {
    if (a.type != b.type) { return a.type-b.type; }
    switch(a.type) {
        case TP_NONE: return 0;
        case TP_NUMBER: return _tp_sign(a.number.val-b.number.val);
        case TP_STRING: return _tp_string_cmp(&a, &b);
        case TP_LIST: return tpd_list_cmp(tp, a.list.val, b.list.val);
        case TP_DICT: return a.dict.val - b.dict.val;
        case TP_FNC: return a.fnc.info - b.fnc.info;
        case TP_DATA: return (char*)a.data.val - (char*)b.data.val;
    }
    tp_raise(0,tp_string("(tp_cmp) TypeError: ?"));
}

tp_obj tp_mod(TP, tp_obj a, tp_obj b) {
    switch(a.type) {
        case TP_NUMBER:
            if(b.type == TP_NUMBER)
                return tp_number(((long)a.number.val) % ((long)b.number.val));
            break;
        case TP_STRING:
            return tp_ez_call(tp, "__builtins__", "format", tp_params_v(tp, 2, a, b));
        
    }
    tp_raise(tp_None, tp_string("(tp_mod) TypeError: ?"));
}

#define TP_OP(name,expr) \
    tp_obj name(TP,tp_obj _a,tp_obj _b) { \
    if (_a.type == TP_NUMBER && _a.type == _b.type) { \
        tp_num a = _a.number.val; tp_num b = _b.number.val; \
        return tp_number(expr); \
    } \
    tp_raise(tp_None,tp_string("(" #name ") TypeError: unsupported operand type(s)")); \
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
    if (a.type == TP_NUMBER) {
        return tp_number(~(long)a.number.val);
    }
    tp_raise(tp_None,tp_string("(tp_bitwise_not) TypeError: unsupported operand type"));
}

/**/
