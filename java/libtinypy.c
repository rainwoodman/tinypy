#define CPYTHON_MOD
#include "../tinypy/tp.c"
#include "../modules/jni/init.c"

typedef struct {
    tp_vm* tp;
    tp_obj globals;
} interpreter_t;

JNIEnv* env = NULL;
interpreter_t* interpreter = NULL;

tp_obj eval(TP, const char* text, tp_obj globals) {
    if(setjmp(tp->nextexpr)) {
        --(tp->cur);
        tp_print_stack(tp);
        return tp->ex;
    }
    tp_obj code = tp_compile(tp, tp_string_const(text), tp_string_const("<eval>"));
    tp_set(tp, globals, tp_string_const("__name__"), tp_string_const("<eval>"));
    tp_set(tp, globals, tp_string_const("__code__"), code);
    tp_exec(tp, code, globals);
    return tp->last_result;
}

tp_obj run_file(TP, const char* filename, tp_obj globals) {
    if(setjmp(tp->nextexpr)) {
        --(tp->cur);
        tp_print_stack(tp);
        return tp_str(tp, tp->ex);
    }
    tp_params_v(tp, 1, tp_string_const(filename));
    tp_obj text = tp_load(tp);
    tp_obj code = tp_compile(tp, text, tp_string_const("__main__"));
    tp_set(tp, globals, tp_string_const("__name__"), tp_string_const("__main__"));
    tp_set(tp, globals, tp_string_const("__code__"), code);
    tp_exec(tp, code, globals);
    return tp->last_result;
}

void Java_TinyPy_init(JNIEnv* _env, jobject thiz) {
    env = _env;
    interpreter = malloc(sizeof(interpreter_t));

    char* argv[] = {"jni"};
    tp_vm* tp = tp_init(1, argv);
    interpreter->tp = tp;
    interpreter->globals = tp_dict(tp);
    jni_init(interpreter->tp);
}

jstring Java_TinyPy_run(JNIEnv* _env, jobject thiz, jstring filename_) {
    env = _env;
    if(interpreter) {
        const char* filename = (*env)->GetStringUTFChars(env, filename_, NULL);
        tp_obj result = run_file(interpreter->tp, filename, interpreter->globals);
        result = tp_str(interpreter->tp, result);
        return (*env)->NewStringUTF(env, result.string.val);
    }
    return NULL;
}

jstring Java_TinyPy_eval(JNIEnv* _env, jobject thiz, jstring code_) {
    env = _env;
    if(interpreter) {
        const char* code = (*env)->GetStringUTFChars(env, code_, NULL);
        tp_obj result = eval(interpreter->tp, code, interpreter->globals);
        result = tp_str(interpreter->tp, result);
        return (*env)->NewStringUTF(env, result.string.val);
    }
    return NULL;
}

jstring Java_TinyPy_getString(JNIEnv* _env, jobject thiz, jstring name_) {
    env = _env;
    if(interpreter) {
        tp_vm* tp = interpreter->tp;
        const char* name = (*env)->GetStringUTFChars(env, name_, 0);
        if(tp_has(tp, interpreter->globals, tp_string_const(name)).number.val) {
            tp_obj result = tp_get(tp, interpreter->globals, tp_string_const(name));
            return (*env)->NewStringUTF(env, result.string.val);
        } else {
            fprintf(stderr, "WARNING: variable \"%s\" not found in tinypy globals\n", name);
            return NULL;
        }
    }
    return NULL;
}

void Java_TinyPy_deinit(JNIEnv* _env, jobject thiz) {
    env = _env;
    if(interpreter) {
        tp_deinit(interpreter->tp);
        free(interpreter);
    }
}
