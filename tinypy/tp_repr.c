
tp_obj tp_str_(TP, tp_obj self, tpd_list * visited);
tp_obj tp_repr_(TP, tp_obj self, tpd_list * visited);

tp_obj tp_repr(TP, tp_obj self) {
    /* we only put unmanaged tp_data objects to the list.*/
    tpd_list * visited = tpd_list_new(tp);
    tp_obj result = tp_repr_(tp, self, visited);
    tpd_list_free(tp, visited);
    return result;
}

tp_obj tp_str(TP, tp_obj self) {
    /* we only put unmanaged tp_data objects to the list.*/
    tpd_list * visited = tpd_list_new(tp);
    tp_obj result = tp_str_(tp, self, visited);
    tpd_list_free(tp, visited);
    return result;
}

tp_obj tp_repr_(TP, tp_obj self, tpd_list *visited) {
    TP_META_BEGIN(self,"__repr__");
        return tp_call(tp,meta,tp_params(tp));
    TP_META_END;

    if(self.type == TP_STRING) {
        tp_params_v(tp, 3, self, tp_string_const("'"), tp_string_const("\\'"));
        tp_obj replaced = tpy_str_replace(tp);
        return tp_printf(tp, "\'%s\'", replaced.string.val);
    }

    return tp_str_(tp, self, visited);
}

/* Function: tp_str
 * String representation of an object.
 * Checks for recursive data structures
 *
 * Returns a string object representating self.
 */
tp_obj tp_str_(TP, tp_obj self, tpd_list * visited) {
    /* if the class has __str__ or __repr__ use those */
    TP_META_BEGIN(self,"__str__");
        return tp_call(tp, meta, tp_params(tp));
    TP_META_END;
    TP_META_BEGIN(self,"__repr___");
        return tp_call(tp, meta, tp_params(tp));
    TP_META_END;
    int type = self.type;
    if(type == TP_DICT) {
        tp_obj data = tp_data_nt(tp, 0, self.dict.val);
        /* FIXME: use tp_data_cmp */
        if(tpd_list_find(tp, visited, data, tp_cmp) >= 0)
            return tp_string_const("{...}");
        tpd_list_append(tp, visited, data);

    } else if(type == TP_LIST) {
        tp_obj data = tp_data_nt(tp, 0, self.list.val);
        if(tpd_list_find(tp, visited, data, tp_cmp) >= 0)
            return tp_string_const("[...]");
        tpd_list_append(tp, visited, data);
    }
    tp_obj result = tp_None;
    if (type == TP_STRING) { 
        result = self; 
    } else if (type == TP_NUMBER) {
        tp_num v = self.number.val;
        if ((fabs(v-(long)v)) < 0.000001) {
            return tp_printf(tp,"%ld",(long)v);
        }
        result = tp_printf(tp, "%f", v);
    } else if(type == TP_DICT) {
        result = tp_string_const("{");
        int i, n = 0;
        for(i = 0; i < self.dict.val->alloc; i++) {
            if(self.dict.val->items[i].used > 0) {
                result = tp_add(tp, result, tp_repr_(tp, self.dict.val->items[i].key, visited));
                result = tp_add(tp, result, tp_string_const(": "));
                result = tp_add(tp, result, tp_repr_(tp, self.dict.val->items[i].val, visited));
                if(n < self.dict.val->len - 1) result = tp_add(tp, result, tp_string_const(", "));
                n += 1;
            }
        }
        result = tp_add(tp, result, tp_string_const("}"));
        /*result = tp_printf(tp,"<dict 0x%x>",self.dict.val);*/
    } else if(type == TP_LIST) {
        result = tp_string_const("[");
        int i;
        for(i = 0; i < self.list.val->len; i++) {
            result = tp_add(tp, result, tp_repr_(tp, self.list.val->items[i], visited));
            if(i < self.list.val->len - 1) result = tp_add(tp, result, tp_string_const(", "));
        }
        result = tp_add(tp, result, tp_string_const("]"));
        /*result = tp_printf(tp,"<list 0x%x>",self.list.val);*/
    } else if (type == TP_NONE) {
        result = tp_string_const("None");
    } else if (type == TP_DATA) {
        result = tp_printf(tp,"<data 0x%x>",self.data.val);
    } else if (type == TP_FNC) {
        result = tp_printf(tp,"<fnc 0x%x>",self.fnc.info);
    } else {
        result = tp_string_const("<?>");
    }
    if(type == TP_DICT || type == TP_LIST) {
        tpd_list_pop(tp, visited, visited->len - 1, "visited list is empty");
    }
    return result;
}

tp_obj tpy_str(TP) {
    tp_obj v = TP_OBJ();
    return tp_str(tp, v);
}

tp_obj tpy_repr(TP) {
    tp_obj v = TP_OBJ();
    return tp_repr(tp, v);
}

