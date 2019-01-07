void _tpi_list_realloc(TP, tpi_list *self, int len) {
    if (!len) { len=1; }
    self->items = (tp_obj*)tp_realloc(tp, self->items,len*sizeof(tp_obj));
    self->alloc = len;
}

void _tpi_list_set(TP, tpi_list *self,int k, tp_obj v, const char *error) {
    if (k >= self->len) {
        tp_raise(,tp_string("(_tpi_list_set) KeyError"));
    }
    self->items[k] = v;
    tp_grey(tp,v);
}

tpi_list *_tpi_list_new(TP) {
    return (tpi_list*) tp_malloc(tp, sizeof(tpi_list));
}

void _tpi_list_free(TP, tpi_list *self) {
    tp_free(tp, self->items);
    tp_free(tp, self);
}

tp_obj _tpi_list_get(TP, tpi_list *self, int k, const char *error) {
    if (k >= self->len) {
        tp_raise(tp_None, tp_string("(_tpi_list_get) KeyError"));
    }
    return self->items[k];
}
void _tpi_list_insertx(TP, tpi_list *self, int n, tp_obj v) {
    if (self->len >= self->alloc) {
        _tpi_list_realloc(tp, self,self->alloc*2);
    }
    if (n < self->len) { memmove(&self->items[n+1],&self->items[n],sizeof(tp_obj)*(self->len-n)); }
    self->items[n] = v;
    self->len += 1;
}
void _tpi_list_appendx(TP, tpi_list *self, tp_obj v) {
    _tpi_list_insertx(tp, self, self->len, v);
}
void _tpi_list_insert(TP,tpi_list *self, int n, tp_obj v) {
    _tpi_list_insertx(tp,self,n,v);
    tp_grey(tp,v);
}
void _tpi_list_append(TP,tpi_list *self, tp_obj v) {
    _tpi_list_insert(tp,self,self->len,v);
}

void _tpi_list_extend(TP, tpi_list * self, tpi_list * v) {
    int i;
    for (i = 0; i < v->len; i++) {
        _tpi_list_append(tp, self, v->items[i]);
    }
}

tp_obj _tpi_list_pop(TP,tpi_list *self, int n, const char *error) {
    tp_obj r = _tpi_list_get(tp,self,n,error);
    if (n != self->len-1) {
        memmove(&self->items[n], &self->items[n+1], sizeof(tp_obj)*(self->len-(n+1)));
    }
    self->len -= 1;
    return r;
}

int _tpi_list_find(TP,tpi_list *self, tp_obj v) {
    int n;
    for (n=0; n<self->len; n++) {
        if (tp_cmp(tp, v, self->items[n]) == 0) {
            return n;
        }
    }
    return -1;
}

int _tpi_list_cmp(TP, tpi_list * a, tpi_list * b)
{
    int n, v;
    for(n=0; n<_tp_min(a->len, b->len); n++) {
        tp_obj aa = a->items[n];
        tp_obj bb = b->items[n];
        if (aa.type == TP_LIST && bb.type == TP_LIST) {
            v = _tpi_list_cmp(tp, aa.list.val, bb.list.val);
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
    tpi_list *o = rr.list.val;
    tpi_list *r = _tpi_list_new(tp);
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
    _tpi_list_extend(tp, r.list.val, b.list.val);
    return r;
}

tp_obj tp_list_mul(TP, tp_obj a, int n)
{
    tp_obj r;
    r = tp_list_copy(tp, a);
    int i;
    for (i = 1; i < n; i ++) {
        _tpi_list_extend(tp, r.list.val, a.list.val);
    }
    return r;
}

tp_obj tp_list_n(TP, int n, tp_obj *argv) {
    int i;
    tp_obj r = tp_list(tp);
    _tpi_list_realloc(tp, r.list.val,n);
    for (i=0; i<n; i++) {
        _tpi_list_append(tp, r.list.val, argv[i]);
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
    int i = _tpi_list_find(tp,self.list.val,v);
    if (i < 0) {
        tp_raise(tp_None,tp_string("(tp_index) ValueError: list.index(x): x not in list"));
    }
    return tp_number(i);
}

tp_obj tpy_list_append(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    _tpi_list_append(tp, self.list.val, v);
    return tp_None;
}

tp_obj tpy_list_pop(TP) {
    tp_obj self = TP_OBJ();
    return _tpi_list_pop(tp, self.list.val, self.list.val->len-1, "pop");
}

tp_obj tpy_list_insert(TP) {
    tp_obj self = TP_OBJ();
    int n = TP_NUM();
    tp_obj v = TP_OBJ();
    _tpi_list_insert(tp, self.list.val, n, v);
    return tp_None;
}

tp_obj tpy_list_extend(TP) {
    tp_obj self = TP_TYPE(TP_LIST);
    tp_obj v = TP_TYPE(TP_LIST);
    _tpi_list_extend(tp, self.list.val, v.list.val);
    return tp_None;
}

tp_obj tpy_list_nt(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = _tpi_list_new(tp);
    return r;
}

tp_obj tp_list(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = _tpi_list_new(tp);
    return tp_track(tp,r);
}

tp_obj tpy_list_sort(TP) {
    tp_obj self = TP_OBJ();
    qsort(self.list.val->items, self.list.val->len, sizeof(tp_obj), (int(*)(const void*,const void*))_tp_list_sort_cmp);
    return tp_None;
}
