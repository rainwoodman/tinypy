/* File: General
 * Things defined in tp.h.
 */
#ifndef TP_H
#define TP_H

#include <setjmp.h>
#include <sys/stat.h>
#ifndef __USE_ISOC99
#define __USE_ISOC99
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

#ifdef __GNUC__
#define tp_inline __inline__
#define TP_PACKED __attribute__ ((packed))
#endif

#ifdef _MSC_VER
#ifdef NDEBUG
#define tp_inline __inline
#else
/* don't inline in debug builds (for easier debugging) */
#define tp_inline
#endif

/* Find out what msvc expects for packed enum */
#define TP_PACKED __attribute__ ((packed))
#endif

#ifndef tp_inline
#error "Unsuported compiler"
#endif

/*  #define tp_malloc(x) calloc((x),1)
    #define tp_realloc(x,y) realloc(x,y)
    #define tp_free(x) free(x) */

/* #include <gc/gc.h>
   #define tp_malloc(x) GC_MALLOC(x)
   #define tp_realloc(x,y) GC_REALLOC(x,y)
   #define tp_free(x)*/

enum TP_PACKED TPTypeID {
    TP_NONE = 0,
    TP_NUMBER = 1,
    TP_GC_TRACKED = 9,
    TP_FUNC = 10,
    TP_DATA = 11,

    TP_STRING = 100,
    TP_DICT = 101,
    TP_LIST = 102,
};

enum TP_PACKED TPTypeMagic {
    TP_DICT_RAW,
    TP_DICT_CLASS, /* do not bind func members */
    TP_DICT_OBJECT, /* bind func members */

    TP_STRING_NONE,
    TP_STRING_ATOM,
    TP_STRING_EXTERN,
    TP_STRING_VIEW,

    TP_FUNC_MASK_STATIC = 1<<6,  /* function will not automatically bind to an instance. */
    TP_FUNC_MASK_METHOD = 1<<7,  /* (bound) prepend instance to params before call. */
};

typedef struct TPTypeInfo {
    enum TPTypeID typeid;
    enum TPTypeMagic magic;
} TPTypeInfo;

typedef double tp_num;

/* Type: tp_obj
 * Tinypy's object representation.
 * 
 * Every object in tinypy is of this type in the C API.
 *
 * Fields:
 * type - This determines what kind of objects it is. It is either TP_NONE, in
 *        which case this is the none type and no other fields can be accessed.
 *        Or it has one of the values listed below, and the corresponding
 *        fields can be accessed.
 * number - TP_NUMBER
 * number.val - A double value with the numeric value.
 * string - TP_STRING
 * string.val - A pointer to the string data.
 * string.len - Length in bytes of the string data.
 * dict - TP_DICT
 * list - TP_LIST
 * func - TP_FUNC
 * data - TP_DATA
 * data.val - The user-provided data pointer.
 * type.magic - The user-provided magic number for identifying the data type.
 */
typedef union tp_obj {
    TPTypeInfo type;
    struct { TPTypeInfo type; int * gci; } gc;
    struct { TPTypeInfo type; tp_num val; } number;
    struct { TPTypeInfo type; struct tpd_func *info; void *cfnc; } func;
    struct { TPTypeInfo type; struct tpd_data *info; void *val; } data;

    struct { TPTypeInfo type; struct tpd_obj *info; } obj;
    struct { TPTypeInfo type; struct tpd_list *val; } list;
    struct { TPTypeInfo type; struct tpd_dict *val; } dict;
    struct { TPTypeInfo type; struct tpd_string *info; const char * val;} string;
} tp_obj;

typedef struct tpd_obj {
    int gci;
} tpd_obj;

typedef struct tpd_string {
    int gci;
    tp_obj base;
    char * s;
    int len;
} tpd_string;

typedef struct tpd_list {
    int gci;
    tp_obj *items;
    int len;
    int alloc;
} tpd_list;

typedef struct tpd_item {
    int used;
    int hash;
    tp_obj key;
    tp_obj val;
} tpd_item;

typedef struct tpd_dict {
    int gci;
    tp_obj meta;
    tpd_item *items;
    int len;
    int alloc;
    int cur;
    int mask;
    int used;
} tpd_dict;

typedef struct tpd_func {
    int gci;
    tp_obj instance;
    tp_obj globals;
    tp_obj code;
} tpd_func;

typedef union tpd_code {
    unsigned char i;
    struct { unsigned char i,a,b,c; } regs;
    struct { char val[0]; } string;
    /* ensure the struct is 0 bytes long. */
    TP_PACKED struct { tp_num val[0]; } number;
} tpd_code;

typedef struct tpd_frame {
/*    tpd_code *codes; */
    tp_obj code;
    tpd_code *cur;
    tpd_code *jmp;
    tp_obj *regs;
    tp_obj *ret_dest;
    tp_obj *fname;
    tp_obj *name;
    tp_obj *line;
    tp_obj globals;
    int lineno;
    int cregs;
} tpd_frame;

#define TP_FRAMES 256
#define TP_REGS_EXTRA 5
/* #define TP_REGS_PER_FRAME 256*/
#define TP_REGS 16384

/* Type: tp_vm
 * Representation of a tinypy virtual machine instance.
 * 
 * A new tp_vm struct is created with <tp_init>, and will be passed to most
 * tinypy functions as first parameter. It contains all the data associated
 * with an instance of a tinypy virtual machine - so it is easy to have
 * multiple instances running at the same time. When you want to free up all
 * memory used by an instance, call <tp_deinit>.
 * 
 * Fields:
 * These fields are currently documented: 
 * 
 * builtins - A dictionary containing all builtin objects.
 * modules - A dictionary with all loaded modules.
 * params - A list of parameters for the current function call.
 * frames - A list of all call frames.
 * cur - The index of the currently executing call frame.
 * frames[n].globals - A dictionary of global sybmols in callframe n.
 */
typedef struct tp_vm {
    tp_obj builtins;
    tp_obj modules;

    /* types */
    tp_obj object_class;
    tp_obj _list_meta;
    tp_obj _dict_meta;
    tp_obj _string_meta;

    /* call */
    int cur;
    tpd_frame frames[TP_FRAMES];
    tp_obj _params;
    tp_obj params;
    tp_obj _regs;
    tp_obj *regs;
    tp_obj *last_result;

    /* exception */
    jmp_buf buf;
#ifdef CPYTHON_MOD
    jmp_buf nextexpr;
#endif
    int jmp;
    tp_obj _exc;
    tp_obj * exc;
    tp_obj * exc_stack;

    /* gc */
    int gcmax;
    tp_obj root;
    tpd_list *white;
    tpd_list *grey;
    tpd_list *black;
    int steps;
    /* cached objects */
    char chars[256][2];
    /* sandbox */
    clock_t clocks;
    double time_elapsed;
    double time_limit;
    unsigned long mem_limit;
    unsigned long mem_used;
    int mem_exceeded;

    void (*echo)(const char* data, int length);
} tp_vm;

#define TP tp_vm *tp
typedef struct tpd_data {
    int gci;
    void (*free)(TP,tp_obj);
} tpd_data;

#define tp_True tp_number(1)
#define tp_False tp_number(0)

extern tp_obj tp_None;

#ifdef TP_SANDBOX
void *tp_malloc(TP, unsigned long);
void *tp_realloc(TP, void *, unsigned long);
void tp_free(TP, void *);
#else
#define tp_malloc(TP,x) calloc((x),1)
#define tp_realloc(TP,x,y) realloc(x,y)
#define tp_free(TP,x) free(x)
#endif

void tp_sandbox(TP, double, unsigned long);
void tp_time_update(TP);
void tp_mem_update(TP);

tp_obj tp_track(TP, tp_obj);
void   tp_grey(TP,tp_obj);

/* __func__ __VA_ARGS__ __FILE__ __LINE__ */

/* Function: tp_raise
 * Macro to raise an exception.
 * 
 * This macro will return from the current function returning "r". The
 * remaining parameters are used to format the exception message.
 */
void   _tp_raise(TP,tp_obj);
#define tp_raise(r, obj) { \
    _tp_raise(tp, obj); \
    return r; \
}

#define tp_raise_printf(r,fmt,...) { \
    _tp_raise(tp, tp_printf(tp, fmt, __VA_ARGS__)); \
    return r; \
}

/* Function: tp_string_from_const
 * Creates a new string object from a C string.
 * 
 * Given a pointer to a C string, creates a tinypy object representing the
 * same string.
 * 
 * *Note* Only a reference to the string will be kept by tinypy, so make sure
 * it does not go out of scope, and don't de-allocate it. Also be aware that
 * tinypy will not delete the string for you. In many cases, it is best to
 * use <tp_string_t> or <tp_string_slice> to create a string where tinypy
 * manages storage for you.
 */
tp_obj tp_string_atom(TP, const char * v);
#define tp_string_equal_atom(a, cstr) (0 == tp_string_cmp_const((a), (cstr), sizeof(cstr) - 1))

int tp_string_len(tp_obj s);
char * tp_string_getptr(tp_obj s);

tp_obj tp_string_t(TP, int n);
/* Function: tp_cstr
 *
 * Fill a C string from a tinypy string, and return as a buffer
 * that needs to be freed by tp_free
 *
 */
char * tp_cstr(TP, tp_obj v);


tp_inline static
tp_obj tp_check_type(TP, int t, tp_obj v) {
    if (v.type.typeid != t) {
        tp_raise(tp_None,
            tp_string_atom(tp, "(tp_check_type) TypeError: unexpected type"));
    }
    return v;
}
/* 
 * Macros for obtaining objects from the parameter list of the current
 * function scope.
 * */
#define TP_NO_LIMIT 0
#define TP_OBJ() (tp_get(tp, tp->params, tp_None))
#define TP_TYPE(t) tp_check_type(tp, t, TP_OBJ())
#define TP_NUM() (TP_TYPE(TP_NUMBER).number.val)
#define TP_STR() (TP_TYPE(TP_STRING))
#define TP_DEFAULT(d) (tp->params.list.val->len?tp_get(tp, tp->params, tp_None):(d))

/* Macro: TP_LOOP
 * Macro to iterate over all remaining arguments.
 *
 * If you have a function which takes a variable number of arguments, you can
 * iterate through all remaining arguments for example like this:
 *
 * > tp_obj *my_func(tp_vm *tp)
 * > {
 * >     // We retrieve the first argument like normal.
 * >     tp_obj first = TP_OBJ();
 * >     // Then we iterate over the remaining arguments.
 * >     tp_obj arg;
 * >     TP_LOOP(arg)
 * >         // do something with arg
 * >     TP_END
 * > }
 */
tp_obj tpd_list_get(TP, tpd_list *self, int k, const char *error);
#define TP_LOOP(e) \
    int __l = tp->params.list.val->len; \
    int __i; for (__i=0; __i<__l; __i++) { \
        (e) = tpd_list_get(tp, tp->params.list.val, __i, "TP_LOOP");
#define TP_END \
    }

/* Function: tp_number
 * Creates a new numeric object.
 */
tp_inline static tp_obj tp_number(tp_num v) {
    tp_obj val = {TP_NUMBER};
    val.number.val = v;
    return val;
}

/* Function: tp_string_n
 * Creates a new string object from a partial C string.
 * 
 * Like <tp_string>, but you specify how many bytes of the given C string to
 * use for the string object. The *note* also applies for this function, as the
 * string reference and length are kept, but no actual substring is stored.
 */
tp_inline static
tp_obj tp_string_from_const(TP, char const * v, int n);
int tp_string_cmp_const(tp_obj, const char * v, int n);

tp_obj tp_params(TP);
tp_obj tp_params_n(TP, int n, tp_obj argv[]);
tp_obj tp_params_v(TP, int n, ...);

tp_obj tp_import(TP, tp_obj name, tp_obj code, tp_obj fname);
tp_obj tp_import_from_buffer(TP, const char * fname, const char * name, void *codes, int len);
tp_obj tp_ez_call(TP, const char *mod, const char *func, tp_obj params);
tp_obj tp_eval_from_cstr(TP, const char *text, tp_obj globals);
tp_obj tp_exec(TP, tp_obj code, tp_obj globals);
tp_obj tp_compile(TP, tp_obj text, tp_obj fname);

tp_obj tp_data_t(TP, int magic, void *v);
tp_obj tp_list_t(TP);
tp_obj tp_list_nt(TP);
tp_obj tp_dict_t(TP);
tp_obj tp_dict_nt(TP);
tp_obj tp_object(TP);
tp_obj tp_class(TP);
tp_obj tp_function(TP, tp_obj v(TP));
tp_obj tp_method(TP, tp_obj self,tp_obj v(TP));
tp_obj tp_def(TP, tp_obj code, tp_obj g);
tp_obj tp_bind(TP, tp_obj function, tp_obj self);
tp_obj tp_staticmethod(TP, tp_obj function);
void tp_set_meta(TP, tp_obj self, tp_obj meta);
tp_obj tp_get_meta(TP, tp_obj self);
int tp_true(TP, tp_obj);
int tp_none(tp_obj);

tp_obj tp_format_stack(TP);

tp_obj tp_printf(TP, const char * fmt, ...);

tp_vm * tp_init(int argc, char *argv[]);
void tp_deinit(TP);

void tp_module_sys_init(TP, int argc, char * argv[]);
void tp_module_builtins_init(TP);
void tp_module_compiler_init(TP);
void tp_module_corelib_init(TP);

#include "tp_ops.h"

#endif
