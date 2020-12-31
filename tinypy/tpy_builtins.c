/* File: Builtins
 * Builtin tinypy functions.
 */
tp_obj tpy_print(TP);

tp_obj tpy_bind(TP) {
    tp_obj r = TP_TYPE(TP_FUNC);
    tp_obj self = TP_OBJ();
    return tp_bind(tp, r, self);
}

tp_obj tpy_staticmethod(TP) {
    tp_obj r = TP_TYPE(TP_FUNC);
    return tp_staticmethod(tp, r);
}

tp_obj tpy_min(TP) {
    tp_obj r = TP_OBJ();
    tp_obj e;
    TP_LOOP(e)
        if (!tp_lessthan(tp,r,e)) { r = e; }
    TP_END;
    return r;
}

tp_obj tpy_max(TP) {
    tp_obj r = TP_OBJ();
    tp_obj e;
    TP_LOOP(e)
        if (tp_lessthan(tp,r,e)) { r = e; }
    TP_END;
    return r;
}

tp_obj tpy_copy(TP) {
    tp_obj r = TP_OBJ();
    return tp_copy(tp, r);
}


tp_obj tpy_len(TP) {
    tp_obj e = TP_OBJ();
    return tp_len(tp,e);
}

tp_obj tpy_range(TP) {
    int a,b,c,i;
    tp_obj r = tp_list_t(tp);
    switch (tp->params.list.val->len) {
        case 1: a = 0; b = TP_NUM(); c = 1; break;
        case 2:
        case 3: a = TP_NUM(); b = TP_NUM(); c = TP_DEFAULT(tp_number(1)).number.val; break;
        default: return r;
    }
    if (c != 0) {
        for (i=a; (c>0) ? i<b : i>b; i+=c) {
            tpd_list_append(tp, r.list.val, tp_number(i));
        }
    }
    return r;
}

tp_obj tpy_istype(TP) {
    tp_obj v = TP_OBJ();
    tp_obj t = TP_STR();
    if (tp_string_equal_atom(t, "string")) { return tp_number(v.type.typeid == TP_STRING); }
    if (tp_string_equal_atom(t, "list")) { return tp_number(v.type.typeid == TP_LIST); }
    if (tp_string_equal_atom(t, "dict")) { return tp_number(v.type.typeid == TP_DICT); }
    if (tp_string_equal_atom(t, "number")) { return tp_number(v.type.typeid == TP_NUMBER); }
    if (tp_string_equal_atom(t, "func")) { return tp_number(v.type.typeid == TP_FUNC && (v.type.mask & TP_FUNC_MASK_METHOD) == 0); }
    if (tp_string_equal_atom(t, "method")) { return tp_number(v.type.typeid == TP_FUNC && (v.type.mask & TP_FUNC_MASK_METHOD) != 0); }
    tp_raise(tp_None,tp_string_atom(tp, "(is_type) TypeError: ?"));
}

tp_obj tpy_isinstance(TP) {
    tp_obj v = TP_OBJ();
    tp_obj t = TP_OBJ();

    if(t.type.typeid != TP_DICT) {
        tp_raise(tp_None, tp_string_atom(tp, "isinstance TypeError: class argument must be a dictionary."));
    }

    tp_obj p = v;
    tp_obj meta;
    while(1) {
        meta = tp_get_meta(tp, p);
        if(tp_none(meta)) break;

        if (meta.dict.val == t.dict.val) {
            return tp_number(1);
        }
        p = meta;
    }
    return tp_number(0);
}


tp_obj tpy_float(TP) {
    tp_obj v = TP_OBJ();
    int ord = TP_DEFAULT(tp_number(0)).number.val;
    int type = v.type.typeid;
    if (type == TP_NUMBER) { return v; }
    if (type == TP_STRING && tp_string_len(v) < 32) {
        char s[32]; memset(s,0,tp_string_len(v)+1);
        memcpy(s, tp_string_getptr(v), tp_string_len(v));
        if (strchr(s,'.')) { return tp_number(atof(s)); }
        return(tp_number(strtol(s,0,ord)));
    }
    tp_raise(tp_None,tp_string_atom(tp, "(tpy_float) TypeError: ?"));
}

tp_obj tpy_fpack(TP) {
    tp_num v = TP_NUM();
    tp_obj r = tp_string_t(tp,sizeof(tp_num));
    *(tp_num*) tp_string_getptr(r) = v;
    return r;
}

tp_obj tpy_funpack(TP) {
    tp_obj v = TP_STR();
    if (tp_string_len(v) != sizeof(tp_num)) {
        tp_raise(tp_None, tp_string_atom(tp, "funpack ValueError: length of string is incorrect."));
    }
    tp_num r = *((tp_num*) tp_string_getptr(v));
    return tp_number(r);
}

tp_obj tpy_abs(TP) {
    return tp_number(fabs(tpy_float(tp).number.val));
}
tp_obj tpy_int(TP) {
    return tp_number((long)tpy_float(tp).number.val);
}
tp_num _roundf(tp_num v) {
    tp_num av = fabs(v); tp_num iv = (long)av;
    av = (av-iv < 0.5?iv:iv+1);
    return (v<0?-av:av);
}
tp_obj tpy_round(TP) {
    return tp_number(_roundf(tpy_float(tp).number.val));
}

/* Function: tp_setmeta
 * Set a "dict's meta".
 *
 * This is a builtin function, so you need to use <tp_params> to provide the
 * parameters.
 *
 * In tinypy, each dictionary can have a so-called "meta" dictionary attached
 * to it. When dictionary attributes are accessed, but not present in the
 * dictionary, they instead are looked up in the meta dictionary. To get the
 * raw dictionary, you can use <tp_getraw>.
 *
 * This function is particulary useful for objects and classes, which are just
 * special dictionaries created with <tp_object> and <tp_class>. There you can
 * use tp_setmeta to change the class of the object or parent class of a class.
 *
 * Parameters:
 * self - The dictionary for which to set a meta.
 * meta - The meta dictionary.
 *
 * Returns:
 * None
 */
tp_obj tpy_setmeta(TP) {
    tp_obj self = TP_OBJ();
    tp_obj meta = TP_TYPE(TP_DICT);
    tp_set_meta(tp, self, meta);
    return tp_None;
}

tp_obj tpy_getmeta(TP) {
    tp_obj self = TP_OBJ();
    return tp_get_meta(tp, self);
}

tp_obj tpy_object_new(TP) {
    tp_obj klass = TP_TYPE(TP_DICT);
    tp_obj self = tp_object(tp);
    self.dict.val->meta = klass;
    TP_META_BEGIN(self, __init__);
        tp_call(tp, __init__, tp->params);
    TP_META_END;
    return self;
}

/* Function: tp_getraw
 * Retrieve the raw dict of a dict.
 *
 * This builtin retrieves one dict parameter from tinypy, and returns its raw
 * dict. This is very useful when implementing your own __get__ and __set__
 * functions, as it allows you to directly access the attributes stored in the
 * dict.
 */
tp_obj tpy_getraw(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    return tp_getraw(tp, self);
}

/* Function: tpy_bool
 * Coerces any value to a boolean.
 */
tp_obj tpy_bool(TP) {
    tp_obj v = TP_OBJ();
    return (tp_number(tp_true(tp, v)));
}

/* import a module or members of a module
 * compiler is initialized; 
 * it only loads existing modules from the module, and returns None
 * on failure. 
 * 
 * Python : 
 *
 * __import__(modname, members)
 *
 * Parameters
 * ----------
 * modname : string; name of the module
 * members : None, list or '*'. names to import.
 *
 * Returns
 * -------
 *  module, if members is None
 *  dict constructed from names on the list and members of the module.
 *
 *  If members is '*', the list includes every member that does not start with '_'.
 *
 * */
tp_obj tpy_import(TP) {
    tp_obj modname = TP_OBJ();
    tp_obj member = TP_OBJ();

    if (!tp_has(tp, tp->modules, modname).number.val) {
        tp_raise(tp_None, tp_string_atom(tp, "(tpy_import) cannot import module"));
    }

    tp_obj mod = tp_get(tp, tp->modules, modname);
    if (member.type.typeid == TP_NONE) {
        /* the entire module */
        return mod;
    }

    /* import members as a dict */
    tp_obj ret = tp_object(tp);

    if (member.type.typeid == TP_STRING) {

        /* from ... import * */
        if(0 == tp_string_cmp(member, tp_string_atom(tp, "*"))) {
            int i;
            for(i = 0; i < mod.dict.val->len; i ++) {
                tpd_item item = mod.dict.val->items[
                    tpd_dict_next(tp, mod.dict.val)
                    ];
                tp_obj k = item.key;
                tp_obj v = item.val;
                if (k.type.typeid != TP_STRING) continue;
                if (tp_string_len(k) == 0) continue;
                char * p = tp_string_getptr(k);
                if (p[0] == '_') continue;
                tp_dict_set(tp, ret, k, v);
            }
            return ret;
        }
    }

    /* from ... import a, b, c */
    if (member.type.typeid == TP_LIST) {
        int i;
        for (i = 0; i < member.list.val->len; i ++) {
            tp_obj k = member.list.val->items[i];
            tp_obj v = tp_mget(tp, mod, k);
            tp_dict_set(tp, ret, k, v);
        }
        return ret;
    }

    tp_raise(tp_None, tp_string_atom(tp, "(tpy_import) cannot import the given member "));
    return tp_None;
}

tp_obj tpy_exec(TP) {
    tp_obj code = TP_OBJ();
    tp_obj globals = TP_OBJ();
    tp_obj r = tp_None;

    return tp_exec(tp, code, globals);
}

tp_obj tpy_eval(TP) {
    tp_obj text = TP_STR();
    tp_obj globals = TP_TYPE(TP_DICT);

    tp_obj code = tp_compile(tp, text, tp_string_atom(tp, "<eval>"));

    tp_exec(tp, code, globals);

    return *tp->last_result;
}

tp_obj tpy_globals(TP) {
    return tp->frames[tp->cur].frame.info->globals;
}


tp_obj tpy_print(TP) {
    int n = 0;
    tp_obj e;
    TP_LOOP(e)
        if (n) { tp->echo(" ", -1); }
        tp_echo(tp,e);
        n += 1;
    TP_END;
    tp->echo("\n", -1);
    return tp_None;
}

tp_obj tpy_str(TP) {
    tp_obj v = TP_OBJ();
    return tp_str(tp, v);
}

tp_obj tpy_repr(TP) {
    tp_obj v = TP_OBJ();
    return tp_repr(tp, v);
}

tp_obj tpy_compile(TP) {
    tp_obj text = TP_OBJ();
    tp_obj fname = TP_OBJ();
    return tp_compile(tp, text, fname);
}

tp_obj tpy_module(TP) {
    tp_obj name = TP_OBJ();
    tp_obj code = TP_OBJ();
    tp_obj fname = TP_OBJ();

    return tp_import(tp, name, code, fname);
}

tp_obj tpy_dict(TP) {
    return tp_dict_copy(tp, TP_TYPE(TP_DICT));
}

tp_obj tpy_dict_update(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    tp_dict_update(tp, self, v);
    return tp_None;
}

void tp_module_builtins_init(TP) {
    tp_obj builtins = tp_object(tp);
    tp_set(tp, builtins, tp_string_atom(tp, "MODULES"), tp->modules);
    tp_set(tp, builtins, tp_string_atom(tp, "__dict__"), tp->builtins);

    tp_obj o;
    struct {const char *s;void *f;} b[] = {
    {"compile", tpy_compile},
    {"print",tpy_print},
    {"range",tpy_range},
    {"min",tpy_min},
    {"max",tpy_max},
    {"bind", tpy_bind},
    {"staticmethod", tpy_staticmethod},
    {"copy",tpy_copy},
    {"__import__",tpy_import},
    {"len",tpy_len},
    {"str", tpy_str},
    {"float",tpy_float}, 
    {"istype",tpy_istype},
    {"isinstance",tpy_isinstance}, 
    {"chr",tpy_chr}, 
    {"fpack",tpy_fpack},
    {"funpack", tpy_funpack},
    {"abs",tpy_abs},
    {"eval",tpy_eval},
    {"exec",tpy_exec},
    {"globals", tpy_globals},
    {"number",tpy_float},
    {"int",tpy_int},
    {"bool", tpy_bool},
    {"dict", tpy_dict},
    {"round",tpy_round},
    {"ord",tpy_ord},
    {"getraw",tpy_getraw},
    {"setmeta",tpy_setmeta},
    {"getmeta",tpy_getmeta},
    {"module", tpy_module},
    {"repr", tpy_repr},
    #ifdef TP_SANDBOX
    {"sandbox",tpy_sandbox_},
    #endif
    {0,0},
    };
    int i;
    for(i=0; b[i].s; i++) {
        tp_set(tp, builtins, tp_string_atom(tp, b[i].s), tp_function(tp,(tp_obj (*)(tp_vm *))b[i].f));
    }
    tp_set(tp, builtins, tp_string_atom(tp, "object"), tp->object_class);

    /* enable creating objects with object() */
    tp_set(tp, tp->object_class, tp_string_atom(tp, "__new__"), tp_function(tp, tpy_object_new));

    tp_set(tp, tp->modules, tp_string_atom(tp, "tinypy.runtime.builtins"), builtins);

    tp_set(tp, tp->list_class, tp_string_atom(tp, "append"), tp_function(tp, tpy_list_append));
    tp_set(tp, tp->list_class, tp_string_atom(tp, "pop"), tp_function(tp, tpy_list_pop));
    tp_set(tp, tp->list_class, tp_string_atom(tp, "index"), tp_function(tp, tpy_list_index));
    tp_set(tp, tp->list_class, tp_string_atom(tp, "sort"), tp_function(tp, tpy_list_sort));
    tp_set(tp, tp->list_class, tp_string_atom(tp, "extend"), tp_function(tp, tpy_list_extend));

    tp_set(tp, tp->string_class, tp_string_atom(tp, "encode"), tp_function(tp, tpy_str_encode));
    tp_set(tp, tp->string_class, tp_string_atom(tp, "join"), tp_function(tp, tpy_str_join));
    tp_set(tp, tp->string_class, tp_string_atom(tp, "split"), tp_function(tp, tpy_str_split));
    tp_set(tp, tp->string_class, tp_string_atom(tp, "index"), tp_function(tp, tpy_str_index));
    tp_set(tp, tp->string_class, tp_string_atom(tp, "find"), tp_function(tp, tpy_str_find));
    tp_set(tp, tp->string_class, tp_string_atom(tp, "strip"), tp_function(tp, tpy_str_strip));
    tp_set(tp, tp->string_class, tp_string_atom(tp, "replace"), tp_function(tp, tpy_str_replace));

    tp_set(tp, tp->dict_class, tp_string_atom(tp, "update"), tp_function(tp, tpy_dict_update));

    tp->builtins = builtins;
}


