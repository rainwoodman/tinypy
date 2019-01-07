/* File: Builtins
 * Builtin tinypy functions.
 */

tp_obj tp_builtins_print(TP) {
    tp_print(tp);
    return tp_None;
}

tp_obj tp_bind(TP) {
    tp_obj r = TP_TYPE(TP_FNC);
    tp_obj self = TP_OBJ();
    return tp_fnc_new(tp,
        r.fnc.ftype|2,r.fnc.cfnc,r.fnc.info->code,
        self,r.fnc.info->globals);
}

tp_obj tp_min(TP) {
    tp_obj r = TP_OBJ();
    tp_obj e;
    TP_LOOP(e)
        if (tp_cmp(tp,r,e) > 0) { r = e; }
    TP_END;
    return r;
}

tp_obj tp_max(TP) {
    tp_obj r = TP_OBJ();
    tp_obj e;
    TP_LOOP(e)
        if (tp_cmp(tp,r,e) < 0) { r = e; }
    TP_END;
    return r;
}

tp_obj tp_copy(TP) {
    tp_obj r = TP_OBJ();
    int type = r.type;
    if (type == TP_LIST) {
        return _tp_list_copy(tp,r);
    } else if (type == TP_DICT) {
        return _tp_dict_copy(tp,r);
    }
    tp_raise(tp_None,tp_string("(tp_copy) TypeError: ?"));
}


tp_obj tp_builtins_len(TP) {
    tp_obj e = TP_OBJ();
    return tp_len(tp,e);
}

tp_obj tp_assert(TP) {
    int a = TP_NUM();
    if (a) { return tp_None; }
    tp_raise(tp_None,tp_string("(tp_assert) AssertionError"));
}

tp_obj tp_range(TP) {
    int a,b,c,i;
    tp_obj r = tp_list(tp);
    switch (tp->params.list.val->len) {
        case 1: a = 0; b = TP_NUM(); c = 1; break;
        case 2:
        case 3: a = TP_NUM(); b = TP_NUM(); c = TP_DEFAULT(tp_number(1)).number.val; break;
        default: return r;
    }
    if (c != 0) {
        for (i=a; (c>0) ? i<b : i>b; i+=c) {
            _tpi_list_append(tp, r.list.val, tp_number(i));
        }
    }
    return r;
}

/* Function: tp_system
 *
 * The system builtin. A grave security flaw. If your version of tinypy
 * enables this, you better remove it before deploying your app :P
 */
tp_obj tp_system(TP) {
    char s[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),s,TP_CSTR_LEN);
    int r = system(s);
    return tp_number(r);
}

tp_obj tp_istype(TP) {
    tp_obj v = TP_OBJ();
    tp_obj t = TP_STR();
    if (tp_cmp(tp,t,tp_string("string")) == 0) { return tp_number(v.type == TP_STRING); }
    if (tp_cmp(tp,t,tp_string("list")) == 0) { return tp_number(v.type == TP_LIST); }
    if (tp_cmp(tp,t,tp_string("dict")) == 0) { return tp_number(v.type == TP_DICT); }
    if (tp_cmp(tp,t,tp_string("number")) == 0) { return tp_number(v.type == TP_NUMBER); }
    if (tp_cmp(tp,t,tp_string("fnc")) == 0) { return tp_number(v.type == TP_FNC && (v.fnc.ftype&2) == 0); }
    if (tp_cmp(tp,t,tp_string("method")) == 0) { return tp_number(v.type == TP_FNC && (v.fnc.ftype&2) != 0); }
    tp_raise(tp_None,tp_string("(is_type) TypeError: ?"));
}

tp_obj tp_isinstance(TP) {
    tp_obj v = TP_OBJ();
    tp_obj t = TP_OBJ();
    
    tp_obj * pv = &v;
    if(t.type != TP_DICT) {
        tp_raise(tp_None, tp_string("isinstance TypeError: class argument must be a dictionary."));
    }

    while(pv->type == TP_DICT) {
        if (pv->dict.val->meta.dict.val == t.dict.val) {
            return tp_number(1);
        }
        pv = &(pv->dict.val->meta);
    }
    return tp_number(0);
}


tp_obj tp_float(TP) {
    tp_obj v = TP_OBJ();
    int ord = TP_DEFAULT(tp_number(0)).number.val;
    int type = v.type;
    if (type == TP_NUMBER) { return v; }
    if (type == TP_STRING && v.string.len < 32) {
        char s[32]; memset(s,0,v.string.len+1);
        memcpy(s,v.string.val,v.string.len);
        if (strchr(s,'.')) { return tp_number(atof(s)); }
        return(tp_number(strtol(s,0,ord)));
    }
    tp_raise(tp_None,tp_string("(tp_float) TypeError: ?"));
}

tp_obj tp_builtins_join(TP) {
    tp_obj val = TP_OBJ();
    int l=0,i;
    tp_obj r;
    char *s;
    for (i=0; i<val.list.val->len; i++) {
        l += tp_str(tp,val.list.val->items[i]).string.len;
    }
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    l = 0;
    for (i=0; i<val.list.val->len; i++) {
        tp_obj e;
        e = tp_str(tp,val.list.val->items[i]);
        memcpy(s+l,e.string.val,e.string.len); l += e.string.len;
    }
    return tp_track(tp,r);
}

tp_obj tp_fpack(TP) {
    tp_num v = TP_NUM();
    tp_obj r = tp_string_t(tp,sizeof(tp_num));
    *(tp_num*)r.string.val = v;
    return tp_track(tp,r);
}

tp_obj tp_funpack(TP) {
    tp_obj v = TP_STR();
    if (v.string.len != sizeof(tp_num)) {
        tp_raise(tp_None, tp_string("funpack ValueError: length of string is incorrect."));
    }
    tp_num r = *((tp_num*) v.string.val);
    return tp_number(r);
}

tp_obj tp_abs(TP) {
    return tp_number(fabs(tp_float(tp).number.val));
}
tp_obj tp_int(TP) {
    return tp_number((long)tp_float(tp).number.val);
}
tp_num _roundf(tp_num v) {
    tp_num av = fabs(v); tp_num iv = (long)av;
    av = (av-iv < 0.5?iv:iv+1);
    return (v<0?-av:av);
}
tp_obj tp_round(TP) {
    return tp_number(_roundf(tp_float(tp).number.val));
}

tp_obj tp_exists(TP) {
    char fname[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),fname,TP_CSTR_LEN);
    struct stat stbuf;
    return tp_number(!stat(fname,&stbuf));
}
tp_obj tp_mtime(TP) {
    char fname[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),fname,TP_CSTR_LEN);
    struct stat stbuf;
    if (!stat(fname,&stbuf)) { return tp_number(stbuf.st_mtime); }
    tp_raise(tp_None,tp_string("(tp_mtime) IOError: ?"));
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
tp_obj tp_setmeta(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj meta = TP_TYPE(TP_DICT);
    self.dict.val->meta = meta;
    return tp_None;
}

tp_obj tp_getmeta(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    return self.dict.val->meta;
}

/* Function: tp_object
 * Creates a new object.
 *
 * Returns:
 * The newly created object. The object initially has no parent class, use
 * <tp_setmeta> to set a class. Also see <tp_object_new>.
 */
tp_obj tp_object(TP) {
    tp_obj self = tp_dict(tp);
    self.dict.dtype = 2;
    return self;
}

tp_obj tp_object_new(TP) {
    tp_obj klass = TP_TYPE(TP_DICT);
    tp_obj self = tp_object(tp);
    self.dict.val->meta = klass;
    TP_META_BEGIN(self,"__init__");
        tp_call(tp,meta,tp->params);
    TP_META_END;
    return self;
}

tp_obj tp_object_call(TP) {
    tp_obj self;
    if (tp->params.list.val->len) {
        self = TP_TYPE(TP_DICT);
        self.dict.dtype = 2;
    } else {
        self = tp_object(tp);
    }
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
tp_obj tp_getraw(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    self.dict.dtype = 0;
    return self;
}

/* Function: tp_class
 * Creates a new base class.
 *
 * Parameters:
 * none
 *
 * Returns:
 * A new, empty class (derived from tinypy's builtin "object" class).
 */
tp_obj tp_class(TP) {
    tp_obj klass = tp_dict(tp);
    klass.dict.val->meta = tp_get(tp,tp->builtins,tp_string("object")); 
    return klass;
}

/* Function: tp_builtins_bool
 * Coerces any value to a boolean.
 */
tp_obj tp_builtins_bool(TP) {
    tp_obj v = TP_OBJ();
    return (tp_number(tp_true(tp, v)));
}

/* the temporary import function used before the
 * compiler is initialized; 
 * it only loads existing modules from the module, and returns None
 * on failure. */
tp_obj tp_builtins_import(TP) {
    tp_obj mod = TP_OBJ();

    if (tp_has(tp,tp->modules,mod).number.val) {
        return tp_get(tp,tp->modules,mod);
    }
    
    /* r = _tp_import(tp,tp_add(tp,mod,tp_string(".tpc")),mod,tp_None); */
    return tp_None;
}

void _tp_import_builtins(TP) {
    tp_obj builtins = tp_dict(tp);
    tp_set(tp, builtins, tp_string("MODULES"), tp->modules);
    tp_set(tp,builtins, tp_string("__dict__"), tp->builtins);

    tp_obj o;
    struct {const char *s;void *f;} b[] = {
    {"print",tp_builtins_print}, {"range",tp_range}, {"min",tp_min},
    {"max",tp_max}, {"bind",tp_bind}, {"copy",tp_copy},
    {"__import__",tp_builtins_import}, {"len",tp_builtins_len}, {"assert",tp_assert},
    {"str",tp_str2}, {"float",tp_float}, {"system",tp_system},
    {"istype",tp_istype}, {"isinstance",tp_isinstance}, 
    {"chr",tp_chr}, {"save",tp_save},
    {"load",tp_load}, {"read",tp_load},
    {"fpack",tp_fpack}, {"funpack", tp_funpack},
    {"abs",tp_abs},
    {"int",tp_int}, {"eval",tp_eval_}, {"exec",tp_exec_}, {"exists",tp_exists},
    {"mtime",tp_mtime}, {"number",tp_float}, {"round",tp_round},
    {"ord",tp_ord}, {"merge",tp_merge}, {"getraw",tp_getraw},
    {"setmeta",tp_setmeta}, {"getmeta",tp_getmeta},
    {"bool", tp_builtins_bool}, {"join", tp_builtins_join}, {"repr", tp_repr2},
    #ifdef TP_SANDBOX
    {"sandbox",tp_sandbox_},
    #endif
    {0,0},
    };
    int i; for(i=0; b[i].s; i++) {
        tp_set(tp, builtins, tp_string(b[i].s), tp_fnc(tp,(tp_obj (*)(tp_vm *))b[i].f));
    }
    
    o = tp_object(tp);
    tp_set(tp,o,tp_string("__call__"),tp_fnc(tp,tp_object_call));
    tp_set(tp,o,tp_string("__new__"),tp_fnc(tp,tp_object_new));
    tp_set(tp,builtins,tp_string("object"),o);

    tp_set(tp,tp->modules, tp_string("tinypy.language.builtins"), builtins);
    tp->builtins = builtins;
}


