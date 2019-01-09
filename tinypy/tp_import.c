tp_obj tpy_save(TP) {
    char * fname = tp_cstr(tp, TP_STR());
    tp_obj v = TP_OBJ();
    FILE *f;
    f = fopen(fname,"wb");
    tp_free(tp, fname);
    if (!f) { tp_raise(tp_None, tp_string_const("(tp_save) IOError: ?")); }
    fwrite(v.string.val,v.string.len,1,f);
    fclose(f);
    return tp_None;
}

tp_obj tpy_load(TP) {
    FILE *f;
    long l;
    tp_obj r;
    char *s;
    char * fname = tp_cstr(tp, TP_STR());
    struct stat stbuf;
    stat(fname, &stbuf);
    l = stbuf.st_size;
    f = fopen(fname,"rb");
    tp_free(tp, fname);
    if (!f) {
        tp_raise(tp_None,tp_string_const("(tp_load) IOError: ?"));
    }
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    fread(s,1,l,f);
/*    if (rr !=l) { printf("hmmn: %d %d\n",rr,(int)l); }*/
    fclose(f);
    return r;
}

tp_obj tp_import(TP, tp_obj fname, tp_obj name, tp_obj code) {
    tp_obj g;

    if (!((fname.type != TP_NONE && tp_str_index(fname,tp_string_const(".tpc"))!=-1) || code.type != TP_NONE)) {
        return tp_ez_call(tp,"tinypy.compiler.py2bc","import_fname",tp_params_v(tp,2,fname,name));
    }

    if (code.type == TP_NONE) {
        tp_params_v(tp,1,fname);
        code = tpy_load(tp);
    }

    g = tp_dict_t(tp);
    tp_set(tp,g,tp_string_const("__name__"),name);
    tp_set(tp,g,tp_string_const("__code__"),code);
    tp_set(tp,g,tp_string_const("__dict__"),g);
    tp_frame(tp,g,code,0);
    tp_set(tp,tp->modules,name,g);

    if (!tp->jmp) { tp_run(tp, tp->cur); }

    return g;
}

/* Function: tp_import
 * Imports a module.
 * 
 * Parameters:
 * fname - The filename of a file containing the module's code.
 * name - The name of the module.
 * codes - The module's code.  If this is given, fname is ignored.
 * len - The length of the bytecode.
 *
 * Returns:
 * The module object.
 */
tp_obj tp_import_from_buffer(TP, const char * fname, const char * name, void *codes, int len) {
    tp_obj f = fname?tp_string_const(fname):tp_None;
    tp_obj bc = codes?tp_string_nt((const char*)codes,len):tp_None;
    return tp_import(tp, f, tp_string_const(name), bc);
}
