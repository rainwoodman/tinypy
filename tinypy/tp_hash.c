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
        case TP_NUMBER: {
            switch(v.type.magic) {
                case TP_NUMBER_INT:
                    return tpd_lua_hash(&v.nint, sizeof(v.nint));
                case TP_NUMBER_FLOAT:
                    return tpd_lua_hash(&v.nfloat, sizeof(v.nfloat));
                default:
                    abort();
            }
        }
        case TP_STRING: return tpd_lua_hash(tp_string_getptr(v), tp_string_len(v));
        case TP_DICT: return tpd_lua_hash(&v.info, sizeof(void*));
        case TP_LIST: {
            int r = TPD_LIST(v)->len;
            int n;
            for(n=0; n<TPD_LIST(v)->len; n++) {
                tp_obj vv = TPD_LIST(v)->items[n];
                r += (vv.type.typeid != TP_LIST)?
                      tp_hash(tp, vv)
                    : tpd_lua_hash(&vv.info, sizeof(void*));
            }
            return r;
        }
        case TP_FUNC: return tpd_lua_hash(&v.info, sizeof(void*));
        case TP_DATA: return tpd_lua_hash(&v.ptr, sizeof(void*));
    }
    tp_raise(0, tp_string_atom(tp, "(tp_hash) TypeError: value unhashable"));
}

