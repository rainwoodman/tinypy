#include <tinypy/tp.h>

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
    tp_obj random_mod = tp_dict_t(tp);

    /*
     * bind functions to random module
     */
    tp_set(tp, random_mod, tp_string_atom(tp, "seed"),       tp_function(tp, random_seed));
    tp_set(tp, random_mod, tp_string_atom(tp, "getstate"),   tp_function(tp, random_getstate));
    tp_set(tp, random_mod, tp_string_atom(tp, "setstate"),   tp_function(tp, random_setstate));
    tp_set(tp, random_mod, tp_string_atom(tp, "jumpahead"),  tp_function(tp, random_jumpahead));
    tp_set(tp, random_mod, tp_string_atom(tp, "random"),     tp_function(tp, random_random));

    /*
     * bind usual distribution random variable generator
     */
    tp_set(tp, random_mod, tp_string_atom(tp, "uniform"),        tp_function(tp, random_uniform));
    tp_set(tp, random_mod, tp_string_atom(tp, "normalvariate"),  tp_function(tp, random_normalvariate));
    tp_set(tp, random_mod, tp_string_atom(tp, "lognormvariate"), tp_function(tp, random_lognormvariate));
    tp_set(tp, random_mod, tp_string_atom(tp, "expovariate"),    tp_function(tp, random_expovariate));
    tp_set(tp, random_mod, tp_string_atom(tp, "vonmisesvariate"), tp_function(tp, random_vonmisesvariate));
    tp_set(tp, random_mod, tp_string_atom(tp, "gammavariate"),   tp_function(tp, random_gammavariate));
    tp_set(tp, random_mod, tp_string_atom(tp, "betavariate"),    tp_function(tp, random_betavariate));
    tp_set(tp, random_mod, tp_string_atom(tp, "paretovariate"),  tp_function(tp, random_paretovariate));
    tp_set(tp, random_mod, tp_string_atom(tp, "weibullvariate"), tp_function(tp, random_weibullvariate));
    tp_set(tp, random_mod, tp_string_atom(tp, "randrange"),      tp_function(tp, random_randrange));
    tp_set(tp, random_mod, tp_string_atom(tp, "randint"),        tp_function(tp, random_randint));
    tp_set(tp, random_mod, tp_string_atom(tp, "choice"),         tp_function(tp, random_choice));
    tp_set(tp, random_mod, tp_string_atom(tp, "shuffle"),        tp_function(tp, random_shuffle));

    /*
     * bind special attributes to random module
     */
    tp_set(tp, random_mod, tp_string_atom(tp, "__doc__"),  tp_string_atom(tp, "Random variable generators."));
    tp_set(tp, random_mod, tp_string_atom(tp, "__name__"), tp_string_atom(tp, "random"));
    tp_set(tp, random_mod, tp_string_atom(tp, "__file__"), tp_string_atom(tp, __FILE__));

    /*
     * bind random module to tinypy modules[]
     */
    tp_set(tp, tp->modules, tp_string_atom(tp, "random"), random_mod);
}
