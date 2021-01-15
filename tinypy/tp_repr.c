void tp_str_(TP, tp_obj self, tpd_list * visited, StringBuilder * sb, int mode);

void tp_str_internal(TP, tp_obj self, StringBuilder * sb, int mode) {
    /* we only put unmanaged tp_data objects to the list.*/
    tpd_list * visited = tpd_list_new(tp);
    tp_str_(tp, self, visited, sb, mode);
    tpd_list_free(tp, visited);
}

tp_obj tp_str(TP, tp_obj self) {
    StringBuilder sb[1] = {tp};
    tp_str_internal(tp, self, sb, 1);
    return tp_string_steal_from_builder(tp, sb);
}
tp_obj tp_repr(TP, tp_obj self) {
    StringBuilder sb[1] = {tp};
    tp_str_internal(tp, self, sb, 0);
    return tp_string_steal_from_builder(tp, sb);
}

/* Function: tp_str
 * String representation of an object.
 * Checks for recursive data structures
 *
 * Returns a string object representating self.
 */
void tp_str_(TP, tp_obj self, tpd_list * visited, StringBuilder * sb, int mode) {
    /* if the class has __str__ or __repr__ use those */
    if(mode != 0) { /* str mode */
        TP_META_BEGIN(self, __str__);
            tp_obj obj = tp_call(tp, __str__, tp_list_t(tp), tp_None);
            string_builder_write(sb, tp_string_getptr(obj), tp_string_len(obj));
            return;
        TP_META_END;
    }
    TP_META_BEGIN(self, __repr__);
        tp_obj obj = tp_call(tp, __repr__, tp_list_t(tp), tp_None);
        string_builder_write(sb, tp_string_getptr(obj), tp_string_len(obj));
        return;
    TP_META_END;

    int type = self.type.typeid;
    if(type == TP_DICT) {
        tp_obj data = tp_data_nt(tp, 0, TPD_DICT(self));
        /* FIXME: use tp_data_cmp */
        if(tpd_list_find(tp, visited, data, tp_equal) >= 0) {
            string_builder_write(sb, "{...}", -1);
            return;
        }
        tpd_list_append(tp, visited, data);
    } else if(type == TP_LIST) {
        tp_obj data = tp_data_nt(tp, 0, TPD_LIST(self));
        if(tpd_list_find(tp, visited, data, tp_equal) >= 0) {
            string_builder_write(sb, "[...]", -1);
            return;
        }
        tpd_list_append(tp, visited, data);
    }

    if (type == TP_STRING) { 
        if(mode != 0) { /* str */
            string_builder_write(sb, tp_string_getptr(self), tp_string_len(self));
        } else { /* repr */
            int i;
            string_builder_write(sb, "'", 1);
            for (i = 0; i < tp_string_len(self); i ++) {
                const char * s = tp_string_getptr(self) + i;
                switch(s[0]) {
                    case '\n':
                        string_builder_write(sb, "\\n", 2);
                        break;
                    case '\r':
                        string_builder_write(sb, "\\r", 2);
                        break;
                    case '\t':
                        string_builder_write(sb, "\\t", 2);
                        break;
                    case '\'':
                    case '\"':
                        string_builder_write(sb, "\\", 1);
                    /* leak through */
                    default:
                        string_builder_write(sb, s, 1);
                }
            }
            string_builder_write(sb, "'", 1);
        } 
    } else if (type == TP_NUMBER) {
        char buf[128];
        switch (self.type.magic) {
            case TP_NUMBER_INT:
                snprintf(buf, 120, "%ld", TPN_AS_INT(self));
                break;
            case TP_NUMBER_FLOAT:
                snprintf(buf, 120, "%lf", TPN_AS_FLOAT(self));
                break;
            default:
                abort();
        }
        string_builder_write(sb, buf, -1);
    } else if(type == TP_DICT) {
        if(self.type.magic == TP_DICT_CLASS) {
            string_builder_write(sb, "C", -1);
        } else if(self.type.magic == TP_DICT_OBJECT) {
            string_builder_write(sb, "O", -1);
        } else if(self.type.magic == TP_DICT_RAW) {
            string_builder_write(sb, "R", -1);
        }
        string_builder_write(sb, "{", -1);
        int i, n = 0;
        for(i = 0; i < TPD_DICT(self)->alloc; i++) {
            if(TPD_DICT(self)->items[i].used > 0) {
                tp_str_(tp, TPD_DICT(self)->items[i].key, visited, sb, mode);
                string_builder_write(sb, ": ", -1);
                tp_str_(tp, TPD_DICT(self)->items[i].val, visited, sb, mode);
                if(n < TPD_DICT(self)->len - 1) {
                    string_builder_write(sb, ", ", -1);
                }
                n += 1;
            }
        }
        string_builder_write(sb, "}", -1);
    } else if(type == TP_LIST) {
        string_builder_write(sb, "[", -1);
        int i;
        for(i = 0; i < TPD_LIST(self)->len; i++) {
            tp_str_(tp, TPD_LIST(self)->items[i], visited, sb, mode);
            if(i < TPD_LIST(self)->len - 1) {
                string_builder_write(sb, ", ", -1);
            }
        }
        string_builder_write(sb, "]", -1);
    } else if (type == TP_NONE) {
        string_builder_write(sb, "None", -1);
    } else if (type == TP_DATA) {
        char buf[128];
        snprintf(buf, 120, "<data %p>", self.ptr);
        string_builder_write(sb, buf, -1);
    } else if (type == TP_FUNC) {
        char buf[128];
        snprintf(buf, 120, "<func %p>", TPD_FUNC(self));
        string_builder_write(sb, buf, -1);
    } else {
        string_builder_write(sb, "<?>", -1);
    }
    if(type == TP_DICT || type == TP_LIST) {
        tpd_list_pop(tp, visited, visited->len - 1, "visited list is empty");
    }
    return;
}

