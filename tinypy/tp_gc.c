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
 *
 **/

/* tp_obj tp_track(TP,tp_obj v) { return v; }
   void tp_grey(TP,tp_obj v) { }
   void tp_full(TP) { }
   void tp_gc_init(TP) { }
   void tp_gc_deinit(TP) { }
   void tp_delete(TP,tp_obj v) { }*/

/* tp_grey: ensure an object to the grey list, if the object is already
 * marked grey, then do nothing. */
void tp_grey(TP, tp_obj v) {
    if (v.type.typeid < TP_GC_TRACKED || (!v.gc.gci)) { return; }
    if (v.gc.gci->grey) { return; }
    if (v.type.typeid == TP_STRING && v.type.magic == TP_STRING_ATOM) { return; }
    if (v.type.typeid == TP_STRING && v.type.magic == TP_STRING_EXTERN) { return; }
    int i;
    v.gc.gci->grey = 1;
    v.gc.gci->black = 0;
    /* terminal types, no need to follow */
    if (v.type.typeid == TP_DATA) {
        v.gc.gci->black = 1;
        tpd_list_appendx(tp, tp->black, v);
        return;
    }
    if (v.type.typeid == TP_STRING && v.type.magic != TP_STRING_VIEW) {
        v.gc.gci->black = 1;
        tpd_list_appendx(tp, tp->black, v);
        return;
    }

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
    tp_grey(tp, tp_get_meta(tp, v));

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
    #ifdef TPVM_DEBUG
    tp->gcmax = 0;
    #else
    tp->gcmax = 8;
    #endif
}

/* Add a reachable object to the gc root. */
void tp_gc_set_reachable(TP, tp_obj v) {
    tp_set(tp, tp->root, tp_None, v);
}

void tp_delete(TP, tp_obj v) {
    #if 0 && defined(TPVM_DEBUG)
    printf("deleting object %p: black %d grey %d\n",
        v.gc.gci,
        v.gc.gci->black,
        v.gc.gci->grey
        );
    #endif
    int type = v.type.typeid;
    if (type == TP_LIST) {
        tpd_list_free(tp, v.list.val);
        return;
    } else if (type == TP_DICT) {
        tpd_dict_free(tp, v.dict.val);
        return;
    } else if (type == TP_STRING) {
        if(v.type.magic == TP_STRING_NORMAL) {
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
    } else if (type == TP_FRAME) {
        tp_free(tp, v.frame.info);
    }
    tp_raise(, tp_string_atom(tp, "(tp_delete) TypeError: ?"));
}

void tp_collect(TP) {
    int n;

    for (n=0; n<tp->white->len; n++) {
        tp_obj r = tp->white->items[n];
        if (r.gc.gci->black) { continue; }
        tp_delete(tp,r);
    }
    tp->white->len = 0;

    /* put all objects to the white list, without duplicates. */
    for (n=0; n<tp->black->len; n++) {
        tp->black->items[n].gc.gci->black = 1;
    }
    for (n=0; n<tp->black->len; n++) {
        tp_obj v = tp->black->items[n];
        if(v.gc.gci->black == 0) {
            abort();
        }
        tpd_list_appendx(tp, tp->white, v);
        v.gc.gci->black = 0;
        v.gc.gci->grey = 0;
    }
    tp->black->len = 0;
}

void tp_mark(TP, int max) {
    while (tp->grey->len && max > 0) {
        tp_obj v;
        /* pick a grey object */
        v = tpd_list_pop(tp, tp->grey, tp->grey->len-1, "_tp_gcinc");
        if(v.gc.gci->black) {
            abort();
        }
        /* color it as black. */
        v.gc.gci->black = 1;
        v.gc.gci->grey = 1;
        tpd_list_appendx(tp, tp->black, v);

        /* put children to grey. */
        tp_follow(tp,v);
        if(max > 0) max--;
    }
}

void tp_gc_dump(TP, tpd_list * l, int name, int mark) {
    /* FIXME: add tp_string_builder_printf, and write to a string builder. */
    #if ! (0 && defined(TPVM_DEBUG))
        return;
    #endif
    int i;
    char step[20];
    sprintf(step, "%c[%06d]%c", mark, tp->steps, name);
    for(i = 0; i < l->len; i ++) {
        tp_obj v = l->items[i];
        printf("%s%p:%d%d%c",
        i % 6 == 0?step:"",
        v.gc.gci,
        v.gc.gci->black,
        v.gc.gci->grey,
        (i + 1) % 6 == 0?'\n':' '
        );
    }
    printf("\n");
    fflush(stdout);
}

void tp_gc_run(TP, int full) {
    if (full || tp->gcmax == 0 || (tp->steps % tp->gcmax == 0)) {
        tp_mark(tp, -1);
    } else {
        /* mark 2 items from the grey list every step */
        tp_mark(tp, 2);
    }

    /* grey list is empty, we can run a collection */
    if(tp->grey->len == 0) {
        tp_gc_dump(tp, tp->white, 'W', 'M');
        /*FIXME: should we do this every time grey drops to zero? */
        tp_collect(tp);
        tp_gc_dump(tp, tp->white, 'W', 'C');
        tp_follow(tp, tp->root);
        tp_gc_dump(tp, tp->grey, 'G', 'F');
    }
    tp->steps += 1;
}

/* tp_track: put an object to the grey list.
 * Use tp_track if the object is definitely new.*/
tp_obj tp_track(TP,tp_obj v) {
    /* force greying the object */
    if (v.type.typeid >= TP_GC_TRACKED && v.gc.gci) {
        v.gc.gci->grey = 0;
        /* NOTE(rainwoodman): I don't think we need to set the following flags */
        v.gc.gci->black = 0;
    }
    tp_grey(tp,v);
    return v;
}

void tp_gc_deinit(TP) {
    while (tp->root.list.val->len) {
        tpd_list_pop(tp, tp->root.list.val, 0, "tp_deinit");
    }
    tp_gc_run(tp, 1);
    tp_delete(tp, tp->root);
    tpd_list_free(tp, tp->white);
    tpd_list_free(tp, tp->grey);
    tpd_list_free(tp, tp->black);
}


/**/

