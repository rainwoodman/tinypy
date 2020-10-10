/**
 * Definition of GC marks from wikipedia on mark and sweep:
 *
 * - The white set, or condemned set, is the set of objects that are candidates
 *   for having their memory recycled.
 *
 * - The black set is the set of objects that can be shown to have no outgoing
 *   references to objects in the white set, and to be reachable from the roots.
 *   Objects in the black set are not candidates for collection.
 *
 * - The gray set contains all objects reachable from the roots but yet to be
 *   scanned for references to "white" objects. Since they are known to be
 *   reachable from the roots, they cannot be garbage-collected and will end up
 *   in the black set after being scanned.
 *
 * Invariant: No black object reference white objects.
 * Therefore if grey is empty, white objects are all unreachable.
 **/

/* tp_obj tp_track(TP,tp_obj v) { return v; }
   void tp_grey(TP,tp_obj v) { }
   void tp_full(TP) { }
   void tp_gc_init(TP) { }
   void tp_gc_deinit(TP) { }
   void tp_delete(TP,tp_obj v) { }*/

void tp_grey(TP, tp_obj v) {
    if (v.type.typeid < TP_GC_TRACKED || (!v.gc.gci) || *v.gc.gci) { return; }
    if (v.type.typeid == TP_STRING && v.type.magic == TP_STRING_ATOM) { return; }
    *v.gc.gci = 1; /* keep-alive */
    if (v.type.typeid == TP_DATA) {
        /* terminal types, no need to follow */
        tpd_list_appendx(tp, tp->black, v);
        return;
    }
    /* need to follow the referents */
    tpd_list_appendx(tp, tp->grey, v);
}

void tp_follow(TP,tp_obj v) {
    int type = v.type.typeid;
    if (type == TP_STRING && v.type.magic == TP_STRING_VIEW) {
        tp_grey(tp, v.string.info->base); 
    }
    if (type == TP_LIST) {
        int n;
        for (n=0; n<v.list.val->len; n++) {
            tp_grey(tp,v.list.val->items[n]);
        }
    }
    if (type == TP_DICT) {
        int i;
        for (i=0; i<v.dict.val->len; i++) {
            int n = tpd_dict_next(tp,v.dict.val);
            tp_grey(tp,v.dict.val->items[n].key);
            tp_grey(tp,v.dict.val->items[n].val);
        }
    }
    if (type >= TP_HAS_META) {
        tp_grey(tp, v.obj.info->meta); 
    }

    if (type == TP_FUNC) {
        tp_grey(tp,v.func.info->instance);
        tp_grey(tp,v.func.info->globals);
        tp_grey(tp,v.func.info->code);
    }
}

void tp_gc_init(TP) {
    tp->root = tp_list_nt(tp);
    tp->white = tpd_list_new(tp);
    tp->grey = tpd_list_new(tp);
    tp->black = tpd_list_new(tp);
    tp->steps = 0;
    tp->gcmax = 16384;
}

/* Add a reachable object to the gc root. */
void tp_gc_set_reachable(TP, tp_obj v) {
    tp_set(tp, tp->root, tp_None, v);
}

void tp_delete(TP, tp_obj v) {
    int type = v.type.typeid;
    if (type == TP_LIST) {
        tpd_list_free(tp, v.list.val);
        return;
    } else if (type == TP_DICT) {
        tpd_dict_free(tp, v.dict.val);
        return;
    } else if (type == TP_STRING) {
        if(v.type.magic == TP_STRING_NONE) {
            tp_free(tp, v.string.info->s); 
        }
        tp_free(tp, v.string.info);
        return;
    } else if (type == TP_DATA) {
        if (v.data.info->free) {
            v.data.info->free(tp,v);
        }
        tp_free(tp, v.data.info);
        return;
    } else if (type == TP_FUNC) {
        tp_free(tp, v.func.info);
        return;
    }
    tp_raise(, tp_string_atom(tp, "(tp_delete) TypeError: ?"));
}

void tp_collect(TP) {
    int n;
    tpd_list *tmp;
    for (n=0; n<tp->white->len; n++) {
        tp_obj r = tp->white->items[n];
        if (*r.gc.gci) { continue; }
        tp_delete(tp,r);
    }
    tp->white->len = 0;

    for (n=0; n<tp->black->len; n++) {
        *tp->black->items[n].gc.gci = 0;
    }
    tmp = tp->white;
    tp->white = tp->black;
    tp->black = tmp;
    /* if called after a sweep, at this point */
    /* white is the list of reachable objects;
     * black shall be empty and will capture newly produced reachable objects */
}

void _tp_gcinc(TP) {
    tp_obj v;
    if (!tp->grey->len) {
        return;
    }
    v = tpd_list_pop(tp, tp->grey, tp->grey->len-1, "_tp_gcinc");
    tp_follow(tp,v);
    tpd_list_appendx(tp, tp->black, v);
}

void tp_full(TP) {
    while (tp->grey->len) {
        _tp_gcinc(tp);
    }
    tp_collect(tp);
    tp_follow(tp, tp->root);
}

void tp_gcinc(TP) {
    tp->steps += 1;

    if (tp->steps < tp->gcmax && tp->grey->len > 0) {
        /* follow two objects every one new object is tracked */
        _tp_gcinc(tp);
        _tp_gcinc(tp);
        return;
    }
    tp->steps = 0;
    tp_full(tp);
}

tp_obj tp_track(TP,tp_obj v) {
    tp_grey(tp,v);
    return v;
}

void tp_gc_deinit(TP) {
    while (tp->root.list.val->len) {
        tpd_list_pop(tp, tp->root.list.val, 0, "tp_deinit");
    }
    tp_full(tp); tp_full(tp);
    tp_delete(tp, tp->root);
    tpd_list_free(tp, tp->white);
    tpd_list_free(tp, tp->grey);
    tpd_list_free(tp, tp->black);
}


/**/

