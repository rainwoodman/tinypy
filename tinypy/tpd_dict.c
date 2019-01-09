void tpd_dict_free(TP, tpd_dict *self) {
    tp_free(tp, self->items);
    tp_free(tp, self);
}

/* void tpd_dict_reset(tpd_dict *self) {
       memset(self->items,0,self->alloc*sizeof(tpd_item));
       self->len = 0;
       self->used = 0;
       self->cur = 0;
   }*/

void tpd_dict_hashset(TP, tpd_dict *self, int hash, tp_obj k, tp_obj v) {
    tpd_item item;
    int i,idx = hash&self->mask;
    for (i=idx; i<idx+self->alloc; i++) {
        int n = i&self->mask;
        if (self->items[n].used > 0) { continue; }
        if (self->items[n].used == 0) { self->used += 1; }
        item.used = 1;
        item.hash = hash;
        item.key = k;
        item.val = v;
        self->items[n] = item;
        self->len += 1;
        return;
    }
}

void tpd_dict_realloc(TP, tpd_dict *self, int len) {
    tpd_item *items = self->items;
    int i,alloc = self->alloc;
    len = _tp_max(8,len);

    self->items = (tpd_item*)tp_malloc(tp, len*sizeof(tpd_item));
    self->alloc = len; self->mask = len-1;
    self->len = 0; self->used = 0;

    for (i=0; i<alloc; i++) {
        if (items[i].used != 1) { continue; }
        tpd_dict_hashset(tp, self, items[i].hash, items[i].key, items[i].val);
    }
    tp_free(tp, items);
}

int tpd_dict_hashfind(TP, tpd_dict * self, int hash, tp_obj k) {
    int i,idx = hash&self->mask;
    for (i=idx; i<idx+self->alloc; i++) {
        int n = i&self->mask;
        if (self->items[n].used == 0) { break; }
        if (self->items[n].used < 0) { continue; }
        if (self->items[n].hash != hash) { continue; }
        if (tp_cmp(tp, self->items[n].key, k) != 0) { continue; }
        return n;
    }
    return -1;
}

tpd_dict *tpd_dict_new(TP) {
    tpd_dict *self = (tpd_dict*) tp_malloc(tp, sizeof(tpd_dict));
    return self;
}

void tpd_dict_hashsetx(TP, tpd_dict * self, int hash, tp_obj k, tp_obj v) {
    int n = tpd_dict_hashfind(tp, self, hash, k);
    if (n == -1) {
        if (self->len >= (self->alloc/2)) {
            tpd_dict_realloc(tp, self, self->alloc*2);
        } else if (self->used >= (self->alloc*3/4)) {
            tpd_dict_realloc(tp, self, self->alloc);
        }
        tpd_dict_hashset(tp, self, hash, k, v);
    } else {
        self->items[n].val = v;
    }
}

int tpd_dict_next(TP, tpd_dict *self) {
    if (!self->len) {
        return -1;
    }
    while (1) {
        self->cur = ((self->cur + 1) & self->mask);
        if (self->items[self->cur].used > 0) {
            return self->cur;
        }
    }
}

tp_obj tpd_dict_get(TP, tpd_dict * self, int n) {
    return self->items[n].val;
}

void tpd_dict_del(TP, tpd_dict * self, int n) {
    self->items[n].used = -1;
    self->len -= 1;
}
