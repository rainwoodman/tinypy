tp_obj tp_import(TP, tp_obj fname, tp_obj name, tp_obj code) {
    tp_obj g;

    if (!((fname.type.typeid != TP_NONE && tp_str_index(fname,tp_string_atom(tp, ".tpc"))!=-1) || code.type.typeid != TP_NONE)) {
        return tp_ez_call(tp,"tinypy.compiler.py2bc","import_fname",tp_params_v(tp,2,fname,name));
    }

    if (code.type.typeid == TP_NONE) {
        tp_raise(tp_None, tp_string_atom(tp, "(tp_import) Must provide code string."));
/*
        char * fname1 = tp_cstr(tp, fname);
        code = tp_load(tp, fname1);
        tp_free(tp, fname1);
*/
    }

    g = tp_dict_t(tp);
    tp_set(tp, g, tp_string_atom(tp, "__name__"), name);
    tp_set(tp, g, tp_string_atom(tp, "__code__"), code);
    tp_set(tp, g, tp_string_atom(tp, "__dict__"), g);

    tp_set(tp, tp->modules, name, g);

    tp_enter_frame(tp, g, code, 0);

    if (!tp->jmp) {
        tp_run_frame(tp);
    }

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
    tp_obj f = fname?tp_string_atom(tp, fname):tp_None;
    tp_obj bc = codes?tp_string_from_const(tp, (const char*)codes, len):tp_None;
    return tp_import(tp, f, tp_string_atom(tp, name), bc);
}
