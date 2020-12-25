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
    v.gc.gci->visited = 0;
    /* terminal types, no need to follow */
    if (v.type.typeid == TP_DATA) {
        v.gc.gci->black = 1;
        tpd_list_appendx(tp, tp->black, v);
        return;
    }
    if (v.type.typeid == TP_STRING) {
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
    tp->gcmax = 16384;
    #endif
}

/* Add a reachable object to the gc root. */
void tp_gc_set_reachable(TP, tp_obj v) {
    tp_set(tp, tp->root, tp_None, v);
}

void tp_delete(TP, tp_obj v) {
    printf("deleting object %p: black %d grey %d visited %d\n",
        v.gc.gci,
        v.gc.gci->black,
        v.gc.gci->grey,
        v.gc.gci->visited);

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
        tp->black->items[n].gc.gci->visited = 0;
    }
    for (n=0; n<tp->black->len; n++) {
        tp_obj v = tp->black->items[n];
        if(v.gc.gci->visited) {
            continue;
        }
        tpd_list_appendx(tp, tp->white, v);
        v.gc.gci->black = 0;
        v.gc.gci->grey = 0;
        v.gc.gci->visited = 1;
    }
    tp->black->len = 0;
}

void tp_scan_grey(TP) {
    while (tp->grey->len) {
        tp_obj v;
        /* pick a grey object */
        v = tpd_list_pop(tp, tp->grey, tp->grey->len-1, "_tp_gcinc");
        if(v.gc.gci->black) {
            abort();
        }
        /* color it as black. */
        v.gc.gci->black = 1;
        v.gc.gci->grey = 1;
        v.gc.gci->visited = 0;
        tpd_list_appendx(tp, tp->black, v);

        /* put children to grey. */
        tp_follow(tp,v);
    }
}

void tp_gcdump(TP) {
    int i;
    printf("====== black %d ======\n", tp->black->len);
    for(i = 0; i < tp->black->len; i ++) {
        tp_obj v = tp->black->items[i];
        printf("%08p : %d%d%d%c", v.gc.gci,
        v.gc.gci->black,
        v.gc.gci->grey,
        v.gc.gci->visited,
        (i + 1) % 8 == 0?'\n':' '
        );
    }
    printf("\n");
    printf("====== white %d ======\n", tp->white->len);
    for(i = 0; i < tp->white->len; i ++) {
        tp_obj v = tp->white->items[i];
        printf("%08p : %d%d%d%c", v.gc.gci,
        v.gc.gci->black,
        v.gc.gci->grey,
        v.gc.gci->visited,
        (i + 1) % 8 == 0?'\n':' '
        );
    }
    printf("\n");
    printf("====== grey %d ======\n", tp->grey->len);
    for(i = 0; i < tp->grey->len; i ++) {
        tp_obj v = tp->grey->items[i];
        printf("%08p : %d%d%d%c", v.gc.gci,
        v.gc.gci->black,
        v.gc.gci->grey,
        v.gc.gci->visited,
        (i + 1) % 8 == 0?'\n':' '
        );
    }
    printf("\n");
}

void tp_full(TP) {
    #ifdef TPVM_DEBUG
    printf("running full gc %d %d\n", tp->steps, tp->gcmax);
    #endif
    tp_scan_grey(tp);
    #ifdef TPVM_DEBUG
    printf("after grey\n"); tp_gcdump(tp);
    #endif
    tp_collect(tp);
    #ifdef TPVM_DEBUG
    printf("after collect\n"); tp_gcdump(tp);
    #endif
    tp_follow(tp, tp->root);
    #ifdef TPVM_DEBUG
    printf("after follow\n"); tp_gcdump(tp);
    #endif
}

void tp_gcinc(TP) {
    if (tp->steps >= tp->gcmax) {
        tp_full(tp);
        tp->steps = 0;
        return;
    }
    tp->steps += 1;
    tp_scan_grey(tp);
}

/* tp_track: put an object to the grey list.
 * Use tp_track if the object is definitely new.*/
tp_obj tp_track(TP,tp_obj v) {
    /* force greying the object */
    if (v.type.typeid >= TP_GC_TRACKED && v.gc.gci) {
        v.gc.gci->grey = 0;
        v.gc.gci->black = 0;
        v.gc.gci->visited = 0;
    }
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

