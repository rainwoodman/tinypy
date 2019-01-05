tp_obj tp_save(TP) {
    char fname[256]; tp_cstr(tp,TP_STR(),fname,256);
    tp_obj v = TP_OBJ();
    FILE *f;
    f = fopen(fname,"wb");
    if (!f) { tp_raise(tp_None,tp_string("(tp_save) IOError: ?")); }
    fwrite(v.string.val,v.string.len,1,f);
    fclose(f);
    return tp_None;
}

tp_obj tp_load(TP) {
    FILE *f;
    long l;
    tp_obj r;
    char *s;
    char fname[256]; tp_cstr(tp,TP_STR(),fname,256);
    struct stat stbuf;
    stat(fname, &stbuf);
    l = stbuf.st_size;
    f = fopen(fname,"rb");
    if (!f) {
        tp_raise(tp_None,tp_string("(tp_load) IOError: ?"));
    }
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    fread(s,1,l,f);
/*    if (rr !=l) { printf("hmmn: %d %d\n",rr,(int)l); }*/
    fclose(f);
    return tp_track(tp,r);
}

tp_obj _tp_import(TP, tp_obj fname, tp_obj name, tp_obj code) {
    tp_obj g;

    if (!((fname.type != TP_NONE && _tp_str_index(fname,tp_string(".tpc"))!=-1) || code.type != TP_NONE)) {
        return tp_ez_call(tp,"tinypy.compiler.py2bc","import_fname",tp_params_v(tp,2,fname,name));
    }

    if (code.type == TP_NONE) {
        tp_params_v(tp,1,fname);
        code = tp_load(tp);
    }

    g = tp_dict(tp);
    tp_set(tp,g,tp_string("__name__"),name);
    tp_set(tp,g,tp_string("__code__"),code);
    tp_set(tp,g,tp_string("__dict__"),g);
    tp_frame(tp,g,code,0);
    tp_set(tp,tp->modules,name,g);

    if (!tp->jmp) { tp_run(tp,tp->cur); }

    return g;
}


