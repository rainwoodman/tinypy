void tpd_list_realloc(TP, tpd_list *self, int len) {
    if (!len) { len=1; }
    self->items = (tp_obj*)tp_realloc(tp, self->items,len*sizeof(tp_obj));
    self->alloc = len;
}

void tpd_list_set(TP, tpd_list *self,int k, tp_obj v, const char *error) {
    if (k >= self->len) {
        tp_raise(,tp_string("(tpd_list_set) KeyError"));
    }
    self->items[k] = v;
    tp_grey(tp,v);
}

tpd_list *tpd_list_new(TP) {
    return (tpd_list*) tp_malloc(tp, sizeof(tpd_list));
}

void tpd_list_free(TP, tpd_list *self) {
    tp_free(tp, self->items);
    tp_free(tp, self);
}

tp_obj tpd_list_get(TP, tpd_list *self, int k, const char *error) {
    if (k >= self->len) {
        tp_raise(tp_None, tp_string("(tpd_list_get) KeyError"));
    }
    return self->items[k];
}
void tpd_list_insertx(TP, tpd_list *self, int n, tp_obj v) {
    if (self->len >= self->alloc) {
        tpd_list_realloc(tp, self,self->alloc*2);
    }
    if (n < self->len) { memmove(&self->items[n+1],&self->items[n],sizeof(tp_obj)*(self->len-n)); }
    self->items[n] = v;
    self->len += 1;
}
void tpd_list_appendx(TP, tpd_list *self, tp_obj v) {
    tpd_list_insertx(tp, self, self->len, v);
}
void tpd_list_insert(TP,tpd_list *self, int n, tp_obj v) {
    tpd_list_insertx(tp,self,n,v);
    tp_grey(tp,v);
}
void tpd_list_append(TP,tpd_list *self, tp_obj v) {
    tpd_list_insert(tp,self,self->len,v);
}

void tpd_list_extend(TP, tpd_list * self, tpd_list * v) {
    int i;
    for (i = 0; i < v->len; i++) {
        tpd_list_append(tp, self, v->items[i]);
    }
}

tp_obj tpd_list_pop(TP,tpd_list *self, int n, const char *error) {
    tp_obj r = tpd_list_get(tp,self,n,error);
    if (n != self->len-1) {
        memmove(&self->items[n], &self->items[n+1], sizeof(tp_obj)*(self->len-(n+1)));
    }
    self->len -= 1;
    return r;
}

int tpd_list_find(TP,tpd_list *self, tp_obj v) {
    int n;
    for (n=0; n<self->len; n++) {
        if (tp_cmp(tp, v, self->items[n]) == 0) {
            return n;
        }
    }
    return -1;
}

int tpd_list_cmp(TP, tpd_list * a, tpd_list * b)
{
    int n, v;
    for(n=0; n<_tp_min(a->len, b->len); n++) {
        tp_obj aa = a->items[n];
        tp_obj bb = b->items[n];
        if (aa.type == TP_LIST && bb.type == TP_LIST) {
            v = tpd_list_cmp(tp, aa.list.val, bb.list.val);
        } else {
            v = tp_cmp(tp, aa, bb);
        }
        if (v) { return v; }
    }
    return a->len - b->len;
}


/* C API for lists */
tp_obj tp_list_copy(TP, tp_obj rr) {
    tp_check_type(tp, TP_LIST, rr);

    tp_obj val = {TP_LIST};
    tpd_list *o = rr.list.val;
    tpd_list *r = tpd_list_new(tp);
    *r = *o; r->gci = 0;
    r->alloc = o->len;
    r->items = (tp_obj*)tp_malloc(tp, sizeof(tp_obj)*o->len);
    memcpy(r->items,o->items,sizeof(tp_obj)*o->len);
    val.list.val = r;
    return tp_track(tp,val);
}

tp_obj tp_list_add(TP, tp_obj a, tp_obj b)
{
    tp_obj r;
    r = tp_list_copy(tp, a);
    tpd_list_extend(tp, r.list.val, b.list.val);
    return r;
}

tp_obj tp_list_mul(TP, tp_obj a, int n)
{
    tp_obj r;
    r = tp_list_copy(tp, a);
    int i;
    for (i = 1; i < n; i ++) {
        tpd_list_extend(tp, r.list.val, a.list.val);
    }
    return r;
}

tp_obj tp_list_n(TP, int n, tp_obj *argv) {
    int i;
    tp_obj r = tp_list(tp);
    tpd_list_realloc(tp, r.list.val,n);
    for (i=0; i<n; i++) {
        tpd_list_append(tp, r.list.val, argv[i]);
    }
    return r;
}

int _tp_list_sort_cmp(tp_obj *a, tp_obj *b) {
    return tp_cmp(0, *a, *b);
}


/******
 * Functions below take arguments from the current python scope.
 *  */

tp_obj tpy_list_index(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i = tpd_list_find(tp,self.list.val,v);
    if (i < 0) {
        tp_raise(tp_None,tp_string("(tp_index) ValueError: list.index(x): x not in list"));
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

tp_obj tpy_list_nt(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = tpd_list_new(tp);
    return r;
}

tp_obj tp_list(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = tpd_list_new(tp);
    return tp_track(tp,r);
}

tp_obj tpy_list_sort(TP) {
    tp_obj self = TP_OBJ();
    qsort(self.list.val->items, self.list.val->len, sizeof(tp_obj), (int(*)(const void*,const void*))_tp_list_sort_cmp);
    return tp_None;
}
