#include <stdlib.h>
#include <dlfcn.h>
#include <ffi.h>
#include <stdint.h>

#define VOID_T 'V'
#define UINT8_T '8'
#define SINT8_T '9'
#define UINT16_T '6'
#define SINT16_T '7'
#define UINT32_T '2'
#define SINT32_T '3'
#define UINT64_T '4'
#define SINT64_T '5'
#define FLOAT_T 'f'
#define DOUBLE_T 'd'
#define UCHAR_T 'c'
#define SCHAR_T 'C'
#define USHORT_T 't'
#define SSHORT_T 'T'
#define UINT_T 'i'
#define SINT_T 'I'
#define ULONG_T 'l'
#define SLONG_T 'L'
#define LDOUBLE_T 'D'
#define STRING_T 'S'
#define POINTER_T '*'

int type_size(TP, char type) {
    switch(type) {
        case VOID_T: return 0; break;
        case UINT8_T: 
        case SINT8_T: return sizeof(int8_t); break;
        case UINT16_T: 
        case SINT16_T: return sizeof(int16_t); break;
        case UINT32_T: 
        case SINT32_T: return sizeof(int32_t); break;
        case UINT64_T: 
        case SINT64_T: return sizeof(int64_t); break;
        case FLOAT_T: return sizeof(float); break;
        case DOUBLE_T: return sizeof(double); break;
        case UCHAR_T: 
        case SCHAR_T: return sizeof(char); break;
        case USHORT_T: 
        case SSHORT_T: return sizeof(short); break;
        case UINT_T: 
        case SINT_T: return sizeof(int); break;
        case ULONG_T: 
        case SLONG_T: return sizeof(long int); break;
        case LDOUBLE_T: return sizeof(long double); break;
        case STRING_T:
        case POINTER_T: return sizeof(void*); break;
        default: tp_raise(-1, tp_printf(tp, "invalid type \"%c\""));
    }
}

ffi_type* map_type(TP, char type) {
    switch(type) {
        case VOID_T: return &ffi_type_void; break;
        case UINT8_T: return &ffi_type_uint8; break;
        case SINT8_T: return &ffi_type_sint8; break;
        case UINT16_T: return &ffi_type_uint16; break;
        case SINT16_T: return &ffi_type_sint16; break;
        case UINT32_T: return &ffi_type_uint32; break;
        case SINT32_T: return &ffi_type_sint32; break;
        case UINT64_T: return &ffi_type_uint64; break;
        case SINT64_T: return &ffi_type_sint64; break;
        case FLOAT_T: return &ffi_type_float; break;
        case DOUBLE_T: return &ffi_type_double; break;
        case UCHAR_T: return &ffi_type_uchar; break;
        case SCHAR_T: return &ffi_type_schar; break;
        case USHORT_T: return &ffi_type_ushort; break;
        case SSHORT_T: return &ffi_type_sshort; break;
        case UINT_T: return &ffi_type_uint; break;
        case SINT_T: return &ffi_type_sint; break;
        case ULONG_T: return &ffi_type_ulong; break;
        case SLONG_T: return &ffi_type_slong; break;
        case LDOUBLE_T: return &ffi_type_longdouble; break;
        case STRING_T:
        case POINTER_T: return &ffi_type_pointer; break;
        default: tp_raise(&ffi_type_void, tp_printf(tp, "invalid type \"%c\""));
    }
}

#define _map_value(type, value) result = malloc(sizeof(type)); *((type*)result) = (type) value; break
void* map_value(TP, char type, tp_obj value) {
    void* result = NULL;
    switch(type) {
        case UCHAR_T:
        case UINT8_T: _map_value(uint8_t, value.number.val);
        case SCHAR_T:
        case SINT8_T: _map_value(int8_t, value.number.val);
        case USHORT_T:
        case UINT16_T: _map_value(uint16_t, value.number.val);
        case SSHORT_T:
        case SINT16_T: _map_value(int16_t, value.number.val);
        case UINT_T:
        case UINT32_T: _map_value(uint32_t, value.number.val);
        case SINT_T:
        case SINT32_T: _map_value(int32_t, value.number.val);
        case ULONG_T:
        case UINT64_T: _map_value(uint64_t, value.number.val);
        case SLONG_T:
        case SINT64_T: _map_value(int64_t, value.number.val);
        case FLOAT_T: _map_value(float, value.number.val);
        case DOUBLE_T: _map_value(double, value.number.val);
        case LDOUBLE_T: _map_value(long double, value.number.val);
        case STRING_T: _map_value(char*, value.string.info->s);
        case POINTER_T: _map_value(void*, value.data.val);
        default: tp_raise(&ffi_type_void, tp_printf(tp, "invalid type \"%c\""));
    }
    return result;
}

#define _unmap_value(cast, mapper) result = mapper(*(cast*)value); break;
tp_obj unmap_value(TP, char type, void* value) {
    tp_obj result;
    switch(type) {
        case UCHAR_T:
        case UINT8_T: _unmap_value(uint8_t, tp_number);
        case SCHAR_T:
        case SINT8_T: _unmap_value(int8_t, tp_number);
        case USHORT_T:
        case UINT16_T: _unmap_value(uint16_t, tp_number);
        case SSHORT_T:
        case SINT16_T: _unmap_value(int16_t, tp_number);
        case UINT_T:
        case UINT32_T: _unmap_value(uint32_t, tp_number);
        case SINT_T:
        case SINT32_T: _unmap_value(int32_t, tp_number);
        case ULONG_T:
        case UINT64_T: _unmap_value(uint64_t, tp_number);
        case SLONG_T:
        case SINT64_T: _unmap_value(int64_t, tp_number);
        case FLOAT_T: _unmap_value(float, tp_number);
        case DOUBLE_T: _unmap_value(double, tp_number);
        case LDOUBLE_T: _unmap_value(long double, tp_number);
        case STRING_T: if(*(char**)value == NULL) result = tp_None; else _unmap_value(char*, tp_string);
        case POINTER_T: result = tpy_data(tp, 0, *(void**)value); break;
        default: tp_raise(tp_None, tp_printf(tp, "invalid type \"%c\""));
    }
    free(value);
    return result;
}

/* handle = dl.open("library") */
tp_obj dl_dlopen(TP) {
    tp_obj name = TP_STR();
    tp_obj handle = tpy_data(tp, 0, NULL);
    handle.data.val = dlopen(name.string.info->s, RTLD_LAZY);
    if (handle.data.val == NULL) {
        tp_raise(tp_None, tp_printf(tp, "%s", dlerror()));
    }
    return handle;
}

/* dl.close(handle) */
tp_obj dl_dlclose(TP) {
    tp_obj handle = TP_TYPE(TP_DATA);
    dlclose(handle.data.val);
    return tp_None;
}

/* sym = dl.sym(handle, "name") */
tp_obj dl_dlsym(TP) {
    tp_obj handle = TP_TYPE(TP_DATA);
    tp_obj name = TP_STR();
    tp_obj result = tpy_data(tp, 0, NULL);
    void* sym = dlsym(handle.data.val, name.string.info->s);
    result.data.val = sym;
    /*fprintf(stderr, "%s => %p\n", name.string.info->s, sym);*/
    if(result.data.val == NULL) {
        tp_raise(tp_None, tp_printf(tp, "%s", dlerror()));
    }
    return result;
}

tp_obj dl_size(TP) {
    tp_obj signature = TP_STR();
    int i;
    int size = 0;
    for(i = 0; i < signature.string.info->len; i++) {
        size += type_size(tp, signature.string.info->s[i]);
    }
    return tp_number(size);
}

void dl_free_data(TP, tp_obj self) {
    free(self.data.val);
}

tp_obj dl_pack(TP) {
    tp_obj signature = TP_STR();
    tp_obj values = TP_TYPE(TP_LIST);
    int i = 0;
    int size = 0;
    for(i = 0; i < signature.string.info->len; i++) {
        size += type_size(tp, signature.string.info->s[i]);
    }
    void* data = malloc(size);
    int offset = 0;
    for(i = 0; i < signature.string.info->len; i++) {
        void* arg = map_value(tp, signature.string.info->s[i], values.list.val->items[i]);
        memcpy(data + offset, arg, type_size(tp, signature.string.info->s[i]));
        offset += type_size(tp, signature.string.info->s[i]);
    }
    tp_obj result = tpy_data(tp, 0, data);
    result.data.info->free = dl_free_data;
    return result;
}

tp_obj dl_unpack(TP) {
    tp_obj signature = TP_STR();
    tp_obj packed = TP_TYPE(TP_DATA);
    int i = 0;
    int offset = 0;
    tp_obj result = tp_list(tp);

    for(i = 0; i < signature.string.info->len; i++) {
        void* value = map_value(tp, signature.string.info->s[i], tp_None);
        memcpy(value, packed.data.val + offset, type_size(tp, signature.string.info->s[i]));
        tp_params_v(tp, 2, result, unmap_value(tp, signature.string.info->s[i], value));
        tp_append(tp);
        offset += type_size(tp, signature.string.info->s[i]);
    }
    return result;
}

/* result = dl.call(sym, 'return_type', 'signature', [args]) */
tp_obj dl_call(TP) {
    tp_obj method = TP_TYPE(TP_DATA);
    tp_obj return_type = TP_STR();
    tp_obj signature = TP_STR();
    tp_obj arguments = TP_TYPE(TP_LIST);

    int num_args = signature.string.info->len;
    ffi_type *args[num_args];
    void* values[num_args];
    int i;
    ffi_cif cif;

    for(i = 0; i < num_args; i++) {
        args[i] = map_type(tp, signature.string.info->s[i]);
        values[i] = map_value(tp, signature.string.info->s[i], arguments.list.val->items[i]);
        /*printf("%d: %c => %p %p %d\n", i, signature.string.info->s[i], *(void**)values[i], arguments.list.val->items[i].data.val, (int)arguments.list.val->items[i].number.val);*/
    }

    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, num_args, map_type(tp, return_type.string.info->s[0]), args) == FFI_OK) {
        void* result = NULL;
        if(return_type.string.info->s[0] != VOID_T) result = map_value(tp, return_type.string.info->s[0], tp_None);
        /*if(result != NULL) printf("result>%f\n", *(double*) result);*/
        ffi_call(&cif, (void (*)(void))method.data.val, result, values);
        /*printf("result<%f\n", *(double*) result);*/
        for(i = 0; i < num_args; i++) free(values[i]);
        if(result != NULL) {
            return unmap_value(tp, return_type.string.info->s[0], result);
        }
        return tp_None;
    }

    tp_raise(tp_None, tp_printf(tp, "dl.call: not implemented"));
}

tp_obj call_method(TP) {
    tp_obj self = TP_OBJ();
    tp_obj args = tp_list_copy(tp, tp->params);

    tp_obj symbol = tp_get(tp, self, tp_string_atom(tp, "symbol"));
    tp_obj return_type = tp_get(tp, self, tp_string_atom(tp, "return_type"));
    tp_obj signature = tp_get(tp, self, tp_string_atom(tp, "signature"));

    tp_params_v(tp, 4, symbol, return_type, signature, args);
    return dl_call(tp);
}

/* Returns an object which calls the method */
tp_obj dl_load(TP) {
    tp_obj library = TP_STR();
    tp_obj name = TP_STR();
    tp_obj return_type = TP_STR();
    tp_obj signature = TP_STR();

    tp_params_v(tp, 1, library);
    tp_obj handle = dl_dlopen(tp);

    tp_obj output = tp_object(tp);

    tp_params_v(tp, 2, handle, name);
    tp_set(tp, output, tp_string_atom(tp, "__name__"), name);
    tp_set(tp, output, tp_string_atom(tp, "symbol"), dl_dlsym(tp));
    tp_set(tp, output, tp_string_atom(tp, "return_type"), return_type);
    tp_set(tp, output, tp_string_atom(tp, "signature"), signature);
    tp_set(tp, output, tp_string_atom(tp, "__call__"), tp_method(tp, output, call_method));

    tp_params_v(tp, 1, handle);
    dl_dlclose(tp);
    return output;
}

tp_obj dl_exception(TP) {
    return tp->ex;
}

/*
 * dl_mod_init()
 *
 * dl module initialization function
 */
void dl_init(TP)
{
    /*
     * module dict for dl
     */
    tp_obj mod = tp_dict(tp);

    /*
     * bind functions to dl module
     */
    tp_set(tp, mod, tp_string_atom(tp, "open"),        tp_func(tp, dl_dlopen));
    tp_set(tp, mod, tp_string_atom(tp, "close"),       tp_func(tp, dl_dlclose));
    tp_set(tp, mod, tp_string_atom(tp, "sym"),         tp_func(tp, dl_dlsym));
    tp_set(tp, mod, tp_string_atom(tp, "call"),        tp_func(tp, dl_call));
    tp_set(tp, mod, tp_string_atom(tp, "load"),        tp_func(tp, dl_load)); 
    tp_set(tp, mod, tp_string_atom(tp, "size"),        tp_func(tp, dl_size)); /* size of data according to signature */
    tp_set(tp, mod, tp_string_atom(tp, "pack"),        tp_func(tp, dl_pack));
    tp_set(tp, mod, tp_string_atom(tp, "unpack"),      tp_func(tp, dl_unpack));
    tp_set(tp, mod, tp_string_atom(tp, "exception"),   tp_func(tp, dl_exception)); /* get current exception */

    /*
     * bind special attributes to random module
     */
    tp_set(tp, mod, tp_string_atom(tp, "__doc__"),  tp_string_atom(tp, "Dynamic library loader."));
    tp_set(tp, mod, tp_string_atom(tp, "__name__"), tp_string_atom(tp, "dl"));
    tp_set(tp, mod, tp_string_atom(tp, "__FILE__"), tp_string_atom(tp, __FILE__));

    /*
     * bind random module to tinypy modules[]
     */
    tp_set(tp, tp->modules, tp_string_atom(tp, "dl"), mod);
}
