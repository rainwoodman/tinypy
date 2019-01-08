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
    tp_obj math_mod = tpy_dict(tp);

    /*
     * initialize pi and e
     */
    math_pi = tp_number(M_PI);
    math_e  = tp_number(M_E);

    /*
     * bind math functions to math module
     */
    tp_set(tp, math_mod, tp_string("pi"), math_pi);
    tp_set(tp, math_mod, tp_string("e"), math_e);
    tp_set(tp, math_mod, tp_string("acos"), tpy_fnc(tp, math_acos));
    tp_set(tp, math_mod, tp_string("asin"), tpy_fnc(tp, math_asin));
    tp_set(tp, math_mod, tp_string("atan"), tpy_fnc(tp, math_atan));
    tp_set(tp, math_mod, tp_string("atan2"), tpy_fnc(tp, math_atan2));
    tp_set(tp, math_mod, tp_string("ceil"), tpy_fnc(tp, math_ceil));
    tp_set(tp, math_mod, tp_string("cos"), tpy_fnc(tp, math_cos));
    tp_set(tp, math_mod, tp_string("cosh"), tpy_fnc(tp, math_cosh));
    tp_set(tp, math_mod, tp_string("degrees"), tpy_fnc(tp, math_degrees));
    tp_set(tp, math_mod, tp_string("exp"), tpy_fnc(tp, math_exp));
    tp_set(tp, math_mod, tp_string("fabs"), tpy_fnc(tp, math_fabs));
    tp_set(tp, math_mod, tp_string("floor"), tpy_fnc(tp, math_floor));
    tp_set(tp, math_mod, tp_string("fmod"), tpy_fnc(tp, math_fmod));
    tp_set(tp, math_mod, tp_string("frexp"), tpy_fnc(tp, math_frexp));
    tp_set(tp, math_mod, tp_string("hypot"), tpy_fnc(tp, math_hypot));
    tp_set(tp, math_mod, tp_string("ldexp"), tpy_fnc(tp, math_ldexp));
    tp_set(tp, math_mod, tp_string("log"), tpy_fnc(tp, math_log));
    tp_set(tp, math_mod, tp_string("log10"), tpy_fnc(tp, math_log10));
    tp_set(tp, math_mod, tp_string("modf"), tpy_fnc(tp, math_modf));
    tp_set(tp, math_mod, tp_string("pow"), tpy_fnc(tp, math_pow));
    tp_set(tp, math_mod, tp_string("radians"), tpy_fnc(tp, math_radians));
    tp_set(tp, math_mod, tp_string("sin"), tpy_fnc(tp, math_sin));
    tp_set(tp, math_mod, tp_string("sinh"), tpy_fnc(tp, math_sinh));
    tp_set(tp, math_mod, tp_string("sqrt"), tpy_fnc(tp, math_sqrt));
    tp_set(tp, math_mod, tp_string("tan"), tpy_fnc(tp, math_tan));
    tp_set(tp, math_mod, tp_string("tanh"), tpy_fnc(tp, math_tanh));

    /*
     * bind special attributes to math module
     */
    tp_set(tp, math_mod, tp_string("__doc__"), 
            tp_string(
                "This module is always available.  It provides access to the\n"
                "mathematical functions defined by the C standard."));
    tp_set(tp, math_mod, tp_string("__name__"), tp_string("math"));
    tp_set(tp, math_mod, tp_string("__file__"), tp_string(__FILE__));

    /*
     * bind to tiny modules[]
     */
    tp_set(tp, tp->modules, tp_string("math"), math_mod);
}

