void tpd_list_realloc(TP, tpd_list *self, int len) {
    if (!len) { len=1; }
    self->items = (tp_obj*)tp_realloc(tp, self->items,len*sizeof(tp_obj));
    self->alloc = len;
}

void tpd_list_set(TP, tpd_list *self,int k, tp_obj v, const char *error) {
    if (k >= self->len) {
        tp_raise(,tp_string_const("(tpd_list_set) KeyError"));
    }
    self->items[k] = v;
    tp_grey(tp, v);
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
        tp_raise(tp_None, tp_string_const("(tpd_list_get) KeyError"));
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
    tp_grey(tp, v);
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

int tpd_list_find(TP, tpd_list * self, tp_obj v, int (*cmp)(TP, tp_obj self, tp_obj v)) {
    int n;
    for (n=0; n<self->len; n++) {
        if (cmp(tp, v, self->items[n]) == 0) {
            return n;
        }
    }
    return -1;
}
