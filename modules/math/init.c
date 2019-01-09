#include <tinypy/tp.h>
#include "math.c"

/*
 * init math module, namely, set its dictionary
 */
void math_init(TP)
{
    /*
     * new a module dict for math
     */
    tp_obj math_mod = tp_dict_t(tp);

    /*
     * initialize pi and e
     */
    math_pi = tp_number(M_PI);
    math_e  = tp_number(M_E);

    /*
     * bind math functions to math module
     */
    tp_set(tp, math_mod, tp_string_atom(tp, "pi"), math_pi);
    tp_set(tp, math_mod, tp_string_atom(tp, "e"), math_e);
    tp_set(tp, math_mod, tp_string_atom(tp, "acos"), tp_function(tp, math_acos));
    tp_set(tp, math_mod, tp_string_atom(tp, "asin"), tp_function(tp, math_asin));
    tp_set(tp, math_mod, tp_string_atom(tp, "atan"), tp_function(tp, math_atan));
    tp_set(tp, math_mod, tp_string_atom(tp, "atan2"), tp_function(tp, math_atan2));
    tp_set(tp, math_mod, tp_string_atom(tp, "ceil"), tp_function(tp, math_ceil));
    tp_set(tp, math_mod, tp_string_atom(tp, "cos"), tp_function(tp, math_cos));
    tp_set(tp, math_mod, tp_string_atom(tp, "cosh"), tp_function(tp, math_cosh));
    tp_set(tp, math_mod, tp_string_atom(tp, "degrees"), tp_function(tp, math_degrees));
    tp_set(tp, math_mod, tp_string_atom(tp, "exp"), tp_function(tp, math_exp));
    tp_set(tp, math_mod, tp_string_atom(tp, "fabs"), tp_function(tp, math_fabs));
    tp_set(tp, math_mod, tp_string_atom(tp, "floor"), tp_function(tp, math_floor));
    tp_set(tp, math_mod, tp_string_atom(tp, "fmod"), tp_function(tp, math_fmod));
    tp_set(tp, math_mod, tp_string_atom(tp, "frexp"), tp_function(tp, math_frexp));
    tp_set(tp, math_mod, tp_string_atom(tp, "hypot"), tp_function(tp, math_hypot));
    tp_set(tp, math_mod, tp_string_atom(tp, "ldexp"), tp_function(tp, math_ldexp));
    tp_set(tp, math_mod, tp_string_atom(tp, "log"), tp_function(tp, math_log));
    tp_set(tp, math_mod, tp_string_atom(tp, "log10"), tp_function(tp, math_log10));
    tp_set(tp, math_mod, tp_string_atom(tp, "modf"), tp_function(tp, math_modf));
    tp_set(tp, math_mod, tp_string_atom(tp, "pow"), tp_function(tp, math_pow));
    tp_set(tp, math_mod, tp_string_atom(tp, "radians"), tp_function(tp, math_radians));
    tp_set(tp, math_mod, tp_string_atom(tp, "sin"), tp_function(tp, math_sin));
    tp_set(tp, math_mod, tp_string_atom(tp, "sinh"), tp_function(tp, math_sinh));
    tp_set(tp, math_mod, tp_string_atom(tp, "sqrt"), tp_function(tp, math_sqrt));
    tp_set(tp, math_mod, tp_string_atom(tp, "tan"), tp_function(tp, math_tan));
    tp_set(tp, math_mod, tp_string_atom(tp, "tanh"), tp_function(tp, math_tanh));

    /*
     * bind special attributes to math module
     */
    tp_set(tp, math_mod, tp_string_atom(tp, "__doc__"), 
            tp_string_atom(tp, 
                "This module is always available.  It provides access to the\n"
                "mathematical functions defined by the C standard."));
    tp_set(tp, math_mod, tp_string_atom(tp, "__name__"), tp_string_atom(tp, "math"));
    tp_set(tp, math_mod, tp_string_atom(tp, "__file__"), tp_string_atom(tp, __FILE__));

    /*
     * bind to tiny modules[]
     */
    tp_set(tp, tp->modules, tp_string_atom(tp, "math"), math_mod);
}

