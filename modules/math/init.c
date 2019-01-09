#include <tinypy/interp.h>
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
    tp_set(tp, math_mod, tp_string_const("pi"), math_pi);
    tp_set(tp, math_mod, tp_string_const("e"), math_e);
    tp_set(tp, math_mod, tp_string_const("acos"), tp_function(tp, math_acos));
    tp_set(tp, math_mod, tp_string_const("asin"), tp_function(tp, math_asin));
    tp_set(tp, math_mod, tp_string_const("atan"), tp_function(tp, math_atan));
    tp_set(tp, math_mod, tp_string_const("atan2"), tp_function(tp, math_atan2));
    tp_set(tp, math_mod, tp_string_const("ceil"), tp_function(tp, math_ceil));
    tp_set(tp, math_mod, tp_string_const("cos"), tp_function(tp, math_cos));
    tp_set(tp, math_mod, tp_string_const("cosh"), tp_function(tp, math_cosh));
    tp_set(tp, math_mod, tp_string_const("degrees"), tp_function(tp, math_degrees));
    tp_set(tp, math_mod, tp_string_const("exp"), tp_function(tp, math_exp));
    tp_set(tp, math_mod, tp_string_const("fabs"), tp_function(tp, math_fabs));
    tp_set(tp, math_mod, tp_string_const("floor"), tp_function(tp, math_floor));
    tp_set(tp, math_mod, tp_string_const("fmod"), tp_function(tp, math_fmod));
    tp_set(tp, math_mod, tp_string_const("frexp"), tp_function(tp, math_frexp));
    tp_set(tp, math_mod, tp_string_const("hypot"), tp_function(tp, math_hypot));
    tp_set(tp, math_mod, tp_string_const("ldexp"), tp_function(tp, math_ldexp));
    tp_set(tp, math_mod, tp_string_const("log"), tp_function(tp, math_log));
    tp_set(tp, math_mod, tp_string_const("log10"), tp_function(tp, math_log10));
    tp_set(tp, math_mod, tp_string_const("modf"), tp_function(tp, math_modf));
    tp_set(tp, math_mod, tp_string_const("pow"), tp_function(tp, math_pow));
    tp_set(tp, math_mod, tp_string_const("radians"), tp_function(tp, math_radians));
    tp_set(tp, math_mod, tp_string_const("sin"), tp_function(tp, math_sin));
    tp_set(tp, math_mod, tp_string_const("sinh"), tp_function(tp, math_sinh));
    tp_set(tp, math_mod, tp_string_const("sqrt"), tp_function(tp, math_sqrt));
    tp_set(tp, math_mod, tp_string_const("tan"), tp_function(tp, math_tan));
    tp_set(tp, math_mod, tp_string_const("tanh"), tp_function(tp, math_tanh));

    /*
     * bind special attributes to math module
     */
    tp_set(tp, math_mod, tp_string_const("__doc__"), 
            tp_string_const(
                "This module is always available.  It provides access to the\n"
                "mathematical functions defined by the C standard."));
    tp_set(tp, math_mod, tp_string_const("__name__"), tp_string_const("math"));
    tp_set(tp, math_mod, tp_string_const("__file__"), tp_string_const(__FILE__));

    /*
     * bind to tiny modules[]
     */
    tp_set(tp, tp->modules, tp_string_const("math"), math_mod);
}

