#include <jni.h>
#include <stdarg.h>

#define DATA_CLASS 2
#define DATA_METHOD 1

extern JNIEnv *env; /* this is set elsewhere */

typedef struct {
    jclass cls;
    jmethodID id;
    char* name;
    char* signature;
} jni_method_t;

void jni_free_method(TP, tp_obj obj) {
    jni_method_t* method = (jni_method_t*) obj.data.val;
    //(*env)->DeleteGlobalRef(env, method->id);
    free(method->name);
    free(method->signature);
    free(method);
}

jvalue jNULL = {.l=NULL};

tp_obj jni_find_class(TP) {
    tp_obj class_name = TP_STR();
    jclass cls = (*env)->FindClass(env, class_name.string.val->s);
    return tpy_data(tp, 0, cls);
}

tp_obj jni_get_method_id(TP) {
    tp_obj cls = TP_TYPE(TP_DATA);
    tp_obj name = TP_STR();
    tp_obj signature = TP_STR();

    jmethodID id = (*env)->GetMethodID(env, cls.data.val, name.string.val->s, signature.string.val->s);
    if(id == NULL) return tp_None;

    jni_method_t* method = malloc(sizeof(jni_method_t));
    method->cls = cls.data.val;
    method->id = id; //(*env)->NewGlobalRef(env, id);
    method->name = strdup(name.string.val->s);
    method->signature = strdup(signature.string.val->s);
    //(*env)->DeleteLocalRef(env, id);

    tp_obj result = tpy_data(tp, DATA_METHOD, method);
    result.data.info->free = jni_free_method;

    return result;
}

jvalue jni_map_array(TP, const char* type, tp_obj value) {
    tp_raise(jNULL, tp_printf(tp, "not implemented"));
}

tp_obj jni_unmap_array(TP, const char* type, jvalue value) {
    tp_raise(tp_None, tp_printf(tp, "not implemented"));
}

jvalue jni_map_value(TP, const char* type, tp_obj value) {
    jvalue result;
    switch(*type) {
        case 'Z': if(value.type.typeid == TP_NUMBER) { result.z = (int) value.number.val; return result; } break;
        case 'B': if(value.type.typeid == TP_NUMBER) { result.b = (unsigned char) value.number.val; return result; } break;
        case 'C': if(value.type.typeid == TP_NUMBER) { result.c = (char) value.number.val; return result; } break;
        case 'S': if(value.type.typeid == TP_NUMBER) { result.s = (short) value.number.val; return result; } break;
        case 'I': if(value.type.typeid == TP_NUMBER) { result.i = (int) value.number.val; return result; } break;
        case 'J': if(value.type.typeid == TP_NUMBER) { result.j = (long int) value.number.val; return result; } break;
        case 'F': if(value.type.typeid == TP_NUMBER) { result.f = (float) value.number.val; return result; } break;
        case 'D': if(value.type.typeid == TP_NUMBER) { result.d = (double) value.number.val; return result; } break;
        case 'L': if(value.type.typeid == TP_NONE) {
                      result.l = NULL;
                  } else if(!strncmp(type, "Ljava/lang/String;", 18) && value.type.typeid == TP_STRING) {
                      result.l = (*env)->NewStringUTF(env, value.string.val->s); 
                      return result;
                  } else if(value.type.typeid == TP_DATA) {
                      result.l = value.data.val; 
                      return result;
                  } 
                  break;
                  if(value.type.typeid == TP_NONE) {
                      result.l = NULL;
                  } else if(value.type.typeid == TP_LIST) {
                      return jni_map_array(tp, type + 1, value);
                  }
                  break;
    }
    tp_raise(jNULL, tp_printf(tp, "unsupported type or type mismatch"));
}

tp_obj jni_unmap_value(TP, const char* type, jvalue value) {
    switch(*type) {
        case 'Z': return tp_number(value.z); break;
        case 'B': return tp_number(value.b); break;
        case 'C': return tp_number(value.c); break;
        case 'S': return tp_number(value.s); break;
        case 'I': return tp_number(value.i); break;
        case 'J': return tp_number(value.j); break;
        case 'F': return tp_number(value.f); break;
        case 'D': return tp_number(value.d); break;
        case 'L': if(value.l == NULL) {
                      return tp_None;
                  } else if(!strncmp(type, "Ljava/lang/String;", 18)) {
                          return tp_string_atom(tp, (*env)->GetStringUTFChars(env, value.l, NULL));
                  } else {
                      return tpy_data(tp, 0, value.l);
                  } 
                  break;
        case '[': return jni_unmap_array(tp, type + 1, value);
    }
    tp_raise(tp_None, tp_printf(tp, "unsupported type or type mismatch"));
}

jvalue* jni_convert_args(TP, const char* signature) {
    jvalue* arguments = malloc(sizeof(jvalue) * tp->params.list.val->len);
    int i = 0;
    signature++;
    while(*signature != '\0' && *signature != ')') {
        if(*signature == '(') {
            continue;
        } else if(*signature == ')') {
            break;
        } else {
            tp_obj arg = _tp_list_get(tp,tp->params.list.val, i, "convert_args");
            arguments[i] = jni_map_value(tp, signature, arg);
            if(*signature == '[') signature++;
            else if(*signature == 'L') {
                signature++;
                while(*signature != ';') signature++;
            }
            i++;
        }
        signature++;
    }
    return arguments;
}

tp_obj jni_call_object_method(TP) {
    tp_obj object = TP_TYPE(TP_DATA);
    tp_obj tp_method = TP_TYPE(TP_DATA);
    jni_method_t* method = (jni_method_t*) tp_method.data.val;

    jvalue* arguments = jni_convert_args(tp, method->signature);
    const char* return_type = strchr(method->signature, ')') + 1;

    jvalue result;
    switch(*return_type) {
        case 'Z': result.z = (*env)->CallBooleanMethodA(env, object.data.val, method->id, arguments); break;
        case 'B': result.b = (*env)->CallByteMethodA(env, object.data.val, method->id, arguments); break;
        case 'C': result.c = (*env)->CallCharMethodA(env, object.data.val, method->id, arguments); break;
        case 'S': result.s = (*env)->CallShortMethodA(env, object.data.val, method->id, arguments); break;
        case 'I': result.i = (*env)->CallIntMethodA(env, object.data.val, method->id, arguments); break;
        case 'J': result.j = (*env)->CallLongMethodA(env, object.data.val, method->id, arguments); break;
        case 'F': result.f = (*env)->CallFloatMethodA(env, object.data.val, method->id, arguments); break;
        case 'D': result.d = (*env)->CallDoubleMethodA(env, object.data.val, method->id, arguments); break;
        case 'L': result.l = (*env)->CallObjectMethodA(env, object.data.val, method->id, arguments); break;
        case 'V': (*env)->CallVoidMethodA(env, object.data.val, method->id, arguments); free(arguments); return tp_None; break;
        default: tp_raise(tp_None, tp_printf(tp, "unsupported return type"));
    }
    free(arguments);
    return jni_unmap_value(tp, return_type, result);
}

tp_obj jni_new_object(TP) {
    tp_obj cls = TP_TYPE(TP_DATA);
    tp_obj tp_constructor = TP_TYPE(TP_DATA);
    jni_method_t* constructor = (jni_method_t*) tp_constructor.data.val;

    jvalue* arguments = jni_convert_args(tp, constructor->signature);
    jobject object = (*env)->NewObjectA(env, cls.data.val, constructor->id, arguments);

    free(arguments);
    return tpy_data(tp, 0, object);
}

/*
 * init jni module, namely, set its dictionary
 */
void jni_init(TP)
{
    /*
     * new a module dict for jni
     */
    tp_obj jni_mod = tp_dict(tp);

    tp_set(tp, jni_mod, tp_string_atom(tp, "find_class"), tp_func(tp, jni_find_class));
    tp_set(tp, jni_mod, tp_string_atom(tp, "get_method_id"), tp_func(tp, jni_get_method_id));
    tp_set(tp, jni_mod, tp_string_atom(tp, "get_static_method_id"), tp_func(tp, jni_get_method_id));
    tp_set(tp, jni_mod, tp_string_atom(tp, "call_object_method"), tp_func(tp, jni_call_object_method));
    tp_set(tp, jni_mod, tp_string_atom(tp, "new_object"), tp_func(tp, jni_new_object));
    tp_set(tp, jni_mod, tp_string_atom(tp, "find_class"), tp_func(tp, jni_find_class));

    /*
     * bind special attributes to jni module
     */
    tp_set(tp, jni_mod, tp_string_atom(tp, "__doc__"), tp_string_atom(tp, "This module gives access to java classes."));
    tp_set(tp, jni_mod, tp_string_atom(tp, "__name__"), tp_string_atom(tp, "jni"));
    tp_set(tp, jni_mod, tp_string_atom(tp, "__file__"), tp_string_atom(tp, __FILE__));

    /*
     * bind to tiny modules[]
     */
    tp_set(tp, tp->modules, tp_string_atom(tp, "jni"), jni_mod);
}

