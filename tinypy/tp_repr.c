typedef struct {
    char * buffer;
    int size;
    int len;
} StringBuilder;

void string_builder_write (TP, StringBuilder * sb, const char * s, int len)
{
    if(len < 0) len = strlen(s);
    if(sb->len + len + 1 >= sb->size) {
        sb->size = (sb->len + len + 1) + sb->len / 2;
        sb->buffer = tp_realloc(tp, sb->buffer, sb->size);
    }
    memcpy(sb->buffer + sb->len, s, len);
    sb->len += len;
    sb->buffer[sb->len] = 0;
}

void tp_str_(TP, tp_obj self, tpd_list * visited, StringBuilder * sb, int mode);

tp_obj tp_repr(TP, tp_obj self) {
    /* we only put unmanaged tp_data objects to the list.*/
    tpd_list * visited = tpd_list_new(tp);
    StringBuilder sb[1];
    sb->buffer = tp_malloc(tp, 128);
    sb->len = 0;
    sb->size = 128;
    tp_str_(tp, self, visited, sb, 0);

    tpd_list_free(tp, visited);
    tp_obj r = tp_string_from_buffer(tp, sb->buffer, sb->len);

    return r;
}

tp_obj tp_str(TP, tp_obj self) {
    /* we only put unmanaged tp_data objects to the list.*/
    tpd_list * visited = tpd_list_new(tp);
    StringBuilder sb[1];
    sb->buffer = tp_malloc(tp, 128);
    sb->len = 0;
    sb->size = 128;

    tp_str_(tp, self, visited, sb, 1);

    tpd_list_free(tp, visited);
//    printf("tp-str: %d %s\n", sb->len, sb->buffer);
    tp_obj r = tp_string_from_buffer(tp, sb->buffer, sb->len);

    return r;
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
        TP_META_BEGIN(self,"__str__");
            tp_obj obj = tp_call(tp, meta, tp_params(tp));
            string_builder_write(tp, sb, obj.string.val->s, obj.string.val->len);
            return;
        TP_META_END;
    }
    TP_META_BEGIN(self,"__repr___");
        tp_obj obj = tp_call(tp, meta, tp_params(tp));
        string_builder_write(tp, sb, obj.string.val->s, obj.string.val->len);
        return;
    TP_META_END;

    int type = self.type.typeid;
    if(type == TP_DICT) {
        tp_obj data = tp_data_nt(tp, 0, self.dict.val);
        /* FIXME: use tp_data_cmp */
        if(tpd_list_find(tp, visited, data, tp_cmp) >= 0) {
            string_builder_write(tp, sb, "{...}", -1);
            return;
        }
        tpd_list_append(tp, visited, data);
    } else if(type == TP_LIST) {
        tp_obj data = tp_data_nt(tp, 0, self.list.val);
        if(tpd_list_find(tp, visited, data, tp_cmp) >= 0) {
            string_builder_write(tp, sb, "[...]", -1);
            return;
        }
        tpd_list_append(tp, visited, data);
    }

    if (type == TP_STRING) { 
        if(mode != 0) { /* str */
            string_builder_write(tp, sb, self.string.val->s, self.string.val->len);
        } else { /* repr */
            int i;
            string_builder_write(tp, sb, "'", 1);
            for (i = 0; i < self.string.val->len; i ++) {
                const char * s = self.string.val->s + i;
                switch(s[0]) {
                    case '\n':
                        string_builder_write(tp, sb, "\\n", 2);
                        break;
                    case '\r':
                        string_builder_write(tp, sb, "\\r", 2);
                        break;
                    case '\t':
                        string_builder_write(tp, sb, "\\t", 2);
                        break;
                    case '\'':
                    case '\"':
                        string_builder_write(tp, sb, "\\", 1);
                    /* leak through */
                    default:
                        string_builder_write(tp, sb, s, 1);
                }
            }
            string_builder_write(tp, sb, "'", 1);
        } 
    } else if (type == TP_NUMBER) {
        char buf[128];
        tp_num v = self.number.val;
        if ((fabs(v-(long)v)) < 0.000001) {
            snprintf(buf, 120, "%ld", (long)v);
            string_builder_write(tp, sb, buf, -1);
        } else {
            snprintf(buf, 120, "%f", v);
        }
    } else if(type == TP_DICT) {
        string_builder_write(tp, sb, "{", -1);
        int i, n = 0;
        for(i = 0; i < self.dict.val->alloc; i++) {
            if(self.dict.val->items[i].used > 0) {
                tp_str_(tp, self.dict.val->items[i].key, visited, sb, mode);
                string_builder_write(tp, sb, ": ", -1);
                tp_str_(tp, self.dict.val->items[i].val, visited, sb, mode);
                if(n < self.dict.val->len - 1) {
                    string_builder_write(tp, sb, ", ", -1);
                }
                n += 1;
            }
        }
        string_builder_write(tp, sb, "}", -1);
    } else if(type == TP_LIST) {
        string_builder_write(tp, sb, "[", -1);
        int i;
        for(i = 0; i < self.list.val->len; i++) {
            tp_str_(tp, self.list.val->items[i], visited, sb, mode);
            if(i < self.list.val->len - 1) {
                string_builder_write(tp, sb, ", ", -1);
            }
        }
        string_builder_write(tp, sb, "]", -1);
    } else if (type == TP_NONE) {
        string_builder_write(tp, sb, "None", -1);
    } else if (type == TP_DATA) {
        char buf[128];
        snprintf(buf, 120, "<data 0x%x>", self.data.val);
        string_builder_write(tp, sb, buf, -1);
    } else if (type == TP_FUNC) {
        char buf[128];
        snprintf(buf, 120, "<func 0x%x>", self.func.info);
        string_builder_write(tp, sb, buf, -1);
    } else {
        string_builder_write(tp, sb, "<?>", -1);
    }
    if(type == TP_DICT || type == TP_LIST) {
        tpd_list_pop(tp, visited, visited->len - 1, "visited list is empty");
    }
    return;
}

tp_obj tpy_str(TP) {
    tp_obj v = TP_OBJ();
    return tp_str(tp, v);
}

tp_obj tpy_repr(TP) {
    tp_obj v = TP_OBJ();
    return tp_repr(tp, v);
}

