
/******
 * Functions below take arguments from the current python scope.
 *  */

tp_obj tpy_list_index(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i = tpd_list_find(tp, self.list.val, v, tp_equal);
    if (i < 0) {
        tp_raise(tp_None,tp_string_atom(tp, "(tp_index) ValueError: list.index(x): x not in list"));
    }
    return tp_number(i);
}

tp_obj tpy_list_append(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    tpd_list_append(tp, self.list.val, v);
    return tp_None;
}

tp_obj tpy_list_pop(TP) {
    tp_obj self = TP_OBJ();
    return tpd_list_pop(tp, self.list.val, self.list.val->len-1, "pop");
}

tp_obj tpy_list_insert(TP) {
    tp_obj self = TP_OBJ();
    int n = TP_NUM();
    tp_obj v = TP_OBJ();
    tpd_list_insert(tp, self.list.val, n, v);
    return tp_None;
}


tp_obj tpy_list_extend(TP) {
    tp_obj self = TP_TYPE(TP_LIST);
    tp_obj v = TP_TYPE(TP_LIST);
    tpd_list_extend(tp, self.list.val, v.list.val);
    return tp_None;
}


/* FIXME: add tpd interface. */
int _tp_list_sort_cmp(tp_obj *a, tp_obj *b) {
    if(tp_equal(0, *a, *b)) {
        return 0;
    }
    if (tp_lessthan(0, *a, *b)) {
        return -1;
    }
    return 1;
}

tp_obj tpy_list_sort(TP) {
    tp_obj self = TP_OBJ();
    qsort(self.list.val->items, self.list.val->len, sizeof(tp_obj), (int(*)(const void*,const void*))_tp_list_sort_cmp);
    return tp_None;
}

