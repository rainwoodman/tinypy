/* File: Dict
 * Functions for dealing with dictionaries.
 */
int tpd_lua_hash(void const *v,int l) {
    int i,step = (l>>5)+1;
    int h = l + (l >= 4?*(int*)v:0);
    for (i=l; i>=step; i-=step) {
        h = h^((h<<5)+(h>>2)+((unsigned char *)v)[i-1]);
    }
    return h;
}

int tp_hash(TP, tp_obj v) {
    switch (v.type.typeid) {
        case TP_NONE: return 0;
        case TP_NUMBER: return tpd_lua_hash(&v.number.val, sizeof(tp_num));
        case TP_STRING: return tpd_lua_hash(tp_string_getptr(v), tp_string_len(v));
        case TP_DICT: return tpd_lua_hash(&v.dict.val, sizeof(void*));
        case TP_LIST: {
            int r = v.list.val->len;
            int n;
            for(n=0; n<v.list.val->len; n++) {
                tp_obj vv = v.list.val->items[n];
                r += (vv.type.typeid != TP_LIST)?
                      tp_hash(tp, v.list.val->items[n])
                    : tpd_lua_hash(&vv.list.val, sizeof(void*));
            }
            return r;
        }
        case TP_FUNC: return tpd_lua_hash(&v.func.info, sizeof(void*));
        case TP_DATA: return tpd_lua_hash(&v.data.val, sizeof(void*));
    }
    tp_raise(0, tp_string_atom(tp, "(tp_hash) TypeError: value unhashable"));
}

