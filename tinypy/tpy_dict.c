tp_obj tpy_dict_update(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    tp_dict_update(tp, self, v);
    return tp_None;
}

