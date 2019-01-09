/* File: Miscellaneous
 * Various functions to help interface tinypy.
 */

void tp_echo(TP, tp_obj e) {
    e = tp_str(tp, e);
    tp->echo(e.string.val->s, e.string.val->len);
}

