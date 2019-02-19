tp_obj tp_import(TP, tp_obj name, tp_obj code, tp_obj fname) {
    tp_obj g;

    g = tp_dict_t(tp);
    tp_set(tp, g, tp_string_atom(tp, "__name__"), name);
    tp_set(tp, g, tp_string_atom(tp, "__file__"), fname);
    tp_set(tp, g, tp_string_atom(tp, "__code__"), code);
    tp_set(tp, g, tp_string_atom(tp, "__dict__"), g);

    tp_set(tp, tp->modules, name, g);

    tp_exec(tp, code, g);

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
    tp_obj bc = codes?tp_string_t_from_const(tp, (const char*)codes, len):tp_None;
    return tp_import(tp, tp_string_atom(tp, name), bc, f);
}
