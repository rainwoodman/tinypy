/* File: Miscellaneous
 * Various functions to help interface tinypy.
 */

void tp_echo(TP, tp_obj e) {
    e = tp_str(tp, e);
    tp->echo(tp_string_getptr(e), tp_string_len(e));
}

