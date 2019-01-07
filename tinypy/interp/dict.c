void tpi_dict_free(TP, tpi_dict *self) {
    tp_free(tp, self->items);
    tp_free(tp, self);
}

/* void tpi_dict_reset(tpi_dict *self) {
       memset(self->items,0,self->alloc*sizeof(tp_item));
       self->len = 0;
       self->used = 0;
       self->cur = 0;
   }*/

void tpi_dict_hashset(TP, tpi_dict *self, int hash, tp_obj k, tp_obj v) {
    tp_item item;
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
    tp_raise(,tp_string("(tpi_dict_hashset) RuntimeError: ?"));
}

void tpi_dict_realloc(TP, tpi_dict *self, int len) {
    tp_item *items = self->items;
    int i,alloc = self->alloc;
    len = _tp_max(8,len);

    self->items = (tp_item*)tp_malloc(tp, len*sizeof(tp_item));
    self->alloc = len; self->mask = len-1;
    self->len = 0; self->used = 0;

    for (i=0; i<alloc; i++) {
        if (items[i].used != 1) { continue; }
        tpi_dict_hashset(tp, self, items[i].hash, items[i].key, items[i].val);
    }
    tp_free(tp, items);
}

int tpi_dict_hashfind(TP, tpi_dict * self, int hash, tp_obj k) {
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
int tpi_dict_find(TP, tpi_dict * self, tp_obj k) {
    return tpi_dict_hashfind(tp, self, tp_hash(tp, k), k);
}

void tpi_dict_setx(TP, tpi_dict * self, tp_obj k, tp_obj v) {
    int hash = tp_hash(tp, k);
    int n = tpi_dict_hashfind(tp, self, hash, k);
    if (n == -1) {
        if (self->len >= (self->alloc/2)) {
            tpi_dict_realloc(tp, self, self->alloc*2);
        } else if (self->used >= (self->alloc*3/4)) {
            tpi_dict_realloc(tp, self, self->alloc);
        }
        tpi_dict_hashset(tp, self, hash, k, v);
    } else {
        self->items[n].val = v;
    }
}

void tpi_dict_set(TP, tpi_dict *self, tp_obj k, tp_obj v) {
    tpi_dict_setx(tp, self, k, v);
    tp_grey(tp,k);
    tp_grey(tp, v);
}

tp_obj tpi_dict_get(TP, tpi_dict *self, tp_obj k, const char *error) {
    int n = tpi_dict_find(tp, self, k);
    if (n < 0) {
        tp_raise(tp_None,
            tp_add(tp, tp_string("(tpi_dict_get) KeyError: "), tp_str(tp,k))
        );
    }
    return self->items[n].val;
}

void tpi_dict_del(TP, tpi_dict * self, tp_obj k, const char *error) {
    int n = tpi_dict_find(tp, self, k);
    if (n < 0) {
        tp_raise(, tp_add(tp, tp_string("(tpi_dict_del) KeyError: "), tp_str(tp, k)));
    }
    self->items[n].used = -1;
    self->len -= 1;
}

tpi_dict *tpi_dict_new(TP) {
    tpi_dict *self = (tpi_dict*) tp_malloc(tp, sizeof(tpi_dict));
    return self;
}

int tpi_dict_next(TP, tpi_dict *self) {
    if (!self->len) {
        tp_raise(0,tp_string("(tpi_dict_next) RuntimeError"));
    }
    while (1) {
        self->cur = ((self->cur + 1) & self->mask);
        if (self->items[self->cur].used > 0) {
            return self->cur;
        }
    }
}

tp_obj tp_dict_copy(TP, tp_obj rr) {
    tp_obj obj = {TP_DICT};
    tpi_dict *o = rr.dict.val;
    tpi_dict *r = tpi_dict_new(tp);
    *r = *o;
    r->gci = 0;
    r->items = (tp_item*) tp_malloc(tp, sizeof(tp_item)*o->alloc);
    memcpy(r->items, o->items, sizeof(tp_item)*o->alloc);
    obj.dict.val = r;
    obj.dict.dtype = 1;
    return tp_track(tp,obj);
}

/* Function: tp_dict
 *
 * Creates a new dictionary object.
 *
 * *Note* If you use <tp_setmeta> on the dictionary, you have to use <tp_getraw> to
 * access the "raw" dictionary again.
 *
 * Returns:
 * The newly created dictionary.
 */
tp_obj tp_dict(TP) {
    tp_obj r = {TP_DICT};
    r.dict.val = tpi_dict_new(tp);
    r.dict.dtype = 1;
    return tp ? tp_track(tp,r) : r;
}

tp_obj tp_dict_n(TP, int n, tp_obj* argv) {
    tp_obj r = tp_dict(tp);
    int i; for (i=0; i<n; i++) {
        tp_set(tp,r,argv[i*2],argv[i*2+1]);
    }
    return r;
}

tp_obj tpy_dict_merge(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i; for (i=0; i<v.dict.val->len; i++) {
        int n = tpi_dict_next(tp, v.dict.val);
        tpi_dict_set(tp,
                self.dict.val,
                v.dict.val->items[n].key,
                v.dict.val->items[n].val);
    }
    return tp_None;
}


