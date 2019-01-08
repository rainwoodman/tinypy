#include <tinypy/interp.h>

#include "random.c"

/*
 * random_mod_init()
 *
 * random module initialization function
 */
void random_init(TP)
{
    /*
     * module dict for random
     */
    tp_obj random_mod = tpy_dict(tp);

    /*
     * bind functions to random module
     */
    tp_set(tp, random_mod, tp_string("seed"),       tpy_fnc(tp, random_seed));
    tp_set(tp, random_mod, tp_string("getstate"),   tpy_fnc(tp, random_getstate));
    tp_set(tp, random_mod, tp_string("setstate"),   tpy_fnc(tp, random_setstate));
    tp_set(tp, random_mod, tp_string("jumpahead"),  tpy_fnc(tp, random_jumpahead));
    tp_set(tp, random_mod, tp_string("random"),     tpy_fnc(tp, random_random));

    /*
     * bind usual distribution random variable generator
     */
    tp_set(tp, random_mod, tp_string("uniform"),        tpy_fnc(tp, random_uniform));
    tp_set(tp, random_mod, tp_string("normalvariate"),  tpy_fnc(tp, random_normalvariate));
    tp_set(tp, random_mod, tp_string("lognormvariate"), tpy_fnc(tp, random_lognormvariate));
    tp_set(tp, random_mod, tp_string("expovariate"),    tpy_fnc(tp, random_expovariate));
    tp_set(tp, random_mod, tp_string("vonmisesvariate"), tpy_fnc(tp, random_vonmisesvariate));
    tp_set(tp, random_mod, tp_string("gammavariate"),   tpy_fnc(tp, random_gammavariate));
    tp_set(tp, random_mod, tp_string("betavariate"),    tpy_fnc(tp, random_betavariate));
    tp_set(tp, random_mod, tp_string("paretovariate"),  tpy_fnc(tp, random_paretovariate));
    tp_set(tp, random_mod, tp_string("weibullvariate"), tpy_fnc(tp, random_weibullvariate));
    tp_set(tp, random_mod, tp_string("randrange"),      tpy_fnc(tp, random_randrange));
    tp_set(tp, random_mod, tp_string("randint"),        tpy_fnc(tp, random_randint));
    tp_set(tp, random_mod, tp_string("choice"),         tpy_fnc(tp, random_choice));
    tp_set(tp, random_mod, tp_string("shuffle"),        tpy_fnc(tp, random_shuffle));

    /*
     * bind special attributes to random module
     */
    tp_set(tp, random_mod, tp_string("__doc__"),  tp_string("Random variable generators."));
    tp_set(tp, random_mod, tp_string("__name__"), tp_string("random"));
    tp_set(tp, random_mod, tp_string("__file__"), tp_string(__FILE__));

    /*
     * bind random module to tinypy modules[]
     */
    tp_set(tp, tp->modules, tp_string("random"), random_mod);
}
