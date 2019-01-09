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
#endif

#ifdef _MSC_VER
#ifdef NDEBUG
#define tp_inline __inline
#else
/* don't inline in debug builds (for easier debugging) */
#define tp_inline
#endif
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

enum {
    TP_NONE,TP_NUMBER,TP_STRING,TP_DICT,
    TP_LIST,TP_FNC,TP_DATA,
};

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
 * fnc - TP_FNC
 * data - TP_DATA
 * data.val - The user-provided data pointer.
 * data.magic - The user-provided magic number for identifying the data type.
 */
typedef union tp_obj {
    int type;
    struct { int type; int *data; } gci;
    struct { int type; tp_num val; } number;
    struct { int type; struct tpd_string *info; char const *val; int len; } string;
    struct { int type; struct tpd_list *val; } list;
    struct { int type; struct tpd_dict *val; int dtype; } dict;
    struct { int type; struct tpd_fnc *info; int ftype; void *cfnc; } fnc;
    struct { int type; struct tpd_data *info; void *val; int magic; } data;
} tp_obj;

typedef struct tpd_string {
    int gci;
    int len;
    char s[1];
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
    tpd_item *items;
    int len;
    int alloc;
    int cur;
    int mask;
    int used;
    tp_obj meta;
} tpd_dict;

typedef struct tpd_fnc {
    int gci;
    tp_obj self;
    tp_obj globals;
    tp_obj code;
} tpd_fnc;

typedef union tpd_code {
    unsigned char i;
    struct { unsigned char i,a,b,c; } regs;
    struct { char val[4]; } string;
    struct { float val; } number;
} tpd_code;

typedef struct tpd_frame {
/*    tpd_code *codes; */
    tp_obj code;
    tpd_code *cur;
    tpd_code *jmp;
    tp_obj *regs;
    tp_obj *ret_dest;
    tp_obj fname;
    tp_obj name;
    tp_obj line;
    tp_obj globals;
    int lineno;
    int cregs;
} tpd_frame;

#define TP_GCMAX 16384 /* FIXME: increased so that gc doesn't get called while running tp_str() */
#define TP_FRAMES 256
#define TP_REGS_EXTRA 2
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
    tpd_frame frames[TP_FRAMES];
    tp_obj _params;
    tp_obj params;
    tp_obj _regs;
    tp_obj *regs;
    tp_obj root;
    jmp_buf buf;
#ifdef CPYTHON_MOD
    jmp_buf nextexpr;
#endif
    int jmp;
    tp_obj ex;
    tp_obj last_result;
    char chars[256][2];
    int cur;
    void (*echo)(const char* data, int length);
    /* gc */
    tpd_list *white;
    tpd_list *grey;
    tpd_list *black;
    int steps;
    /* sandbox */
    clock_t clocks;
    double time_elapsed;
    double time_limit;
    unsigned long mem_limit;
    unsigned long mem_used;
    int mem_exceeded;
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

void   tp_run(TP,int cur);

void   tp_set(TP, tp_obj, tp_obj, tp_obj);
tp_obj tp_get(TP, tp_obj, tp_obj);
tp_obj tp_has(TP, tp_obj self, tp_obj k);
tp_obj tp_len(TP, tp_obj);

void   tp_del(TP, tp_obj, tp_obj);
tp_obj tp_str_tracked(TP, tp_obj);
tp_obj tp_repr_tracked(TP, tp_obj);
int    tp_true(TP, tp_obj);
int    tp_cmp(TP, tp_obj, tp_obj);
tp_obj tp_add(TP,tp_obj a, tp_obj b) ;
tp_obj tp_mul(TP, tp_obj a, tp_obj b);
int    tp_hash(TP, tp_obj v);

tp_obj tp_track(TP, tp_obj);
void   tp_grey(TP,tp_obj);
tp_obj tp_call(TP, tp_obj fnc, tp_obj params);

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
    _tp_raise(tp, tp_printf_tracked(tp, fmt, __VA_ARGS__)); \
    return r; \
}

/* Function: tp_string
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
/* FIXME: rename this tp tp_string_const; */
tp_inline static
tp_obj tp_string(char const *v) {
    tp_obj r;
    r.string.type = TP_STRING;
    r.string.info = NULL;
    r.string.val = v;
    r.string.len = strlen(v);
    return r;
}

#define TP_CSTR_LEN 256

/* Function: tp_cstr
 *
 * Fill a C string from a tinypy string, and return as a buffer
 * that needs to be freed by tp_free
 *
 */
tp_inline static
char * tp_cstr(TP, tp_obj v) {
    char * buffer;
    char const * val;
    if(v.type != TP_STRING) {
        val = "NOT A STRING";
        buffer = tp_malloc(tp, strlen(val) + 1);
        memcpy(buffer, val, strlen(val) + 1);
    } else {
        val = v.string.val;
        buffer = tp_malloc(tp, v.string.len + 1);
        memset(buffer, 0, v.string.len + 1);
        memcpy(buffer, val, v.string.len);
    }
    
    return buffer;
}


tp_inline static
tp_obj tp_check_type(TP,int t, tp_obj v) {
    if (v.type != t) {
        tp_raise(tp_None,
            tp_string("(tp_check_type) TypeError: unexpected type"));
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
/* #define TP_STR() (TP_CSTR(TP_TYPE(TP_STRING))) */
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

tp_inline static int _tp_min(int a, int b) { return (a<b?a:b); }
tp_inline static int _tp_max(int a, int b) { return (a>b?a:b); }
tp_inline static int _tp_sign(tp_num v) { return (v<0?-1:(v>0?1:0)); }

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
tp_obj tp_string_n(char const * v, int n) {
    tp_obj r;
    r.string.type = TP_STRING;
    r.string.info = NULL;
    r.string.val = v;
    r.string.len = n;
    return r;
}

tp_obj tp_params(TP);
tp_obj tp_params_n(TP, int n, tp_obj argv[]);
tp_obj tp_params_v(TP, int n, ...);

tp_vm *tp_init(int argc, char *argv[]);
void tp_deinit(TP);

tp_obj tp_import(TP, tp_obj fname, tp_obj name, tp_obj code);
tp_obj tp_import_from_buffer(TP, const char * fname, const char * name, void *codes, int len);
tp_obj tp_ez_call(TP, const char *mod, const char *fnc, tp_obj params);
tp_obj tp_eval_from_cstr(TP, const char *text, tp_obj globals);
tp_obj tp_exec(TP, tp_obj code, tp_obj globals);
tp_obj tp_compile(TP, tp_obj text, tp_obj fname);

void   tp_run(TP, int cur);

tp_obj tpy_copy(TP);
tp_obj tpy_print(TP);
tp_obj tpy_load(TP);
tp_obj tpy_save(TP);

tp_obj tpy_list(TP);
tp_obj tpy_dict(TP);
tp_obj tpy_fnc(TP, tp_obj v(TP));
tp_obj tpy_method(TP,tp_obj self,tp_obj v(TP));
tp_obj tpy_def(TP, tp_obj code, tp_obj g);

tp_obj tp_printf_tracked(TP, const char * fmt, ...);

void tp_module_sys_init(TP, int argc, char * argv[]);
void tp_module_builtins_init(TP);
void tp_module_compiler_init(TP);
void tp_module_corelib_init(TP);

#endif
