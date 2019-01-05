#include "core.h"
#include "vm.h"

#include "corelib/encode.c"
#include "corelib/parse.c"
#include "corelib/tokenize.c"
#include "corelib/py2bc.c"

void tp_compiler(TP) {
    tp_import(tp, 0, "tinypy.compiler.tokenize", _tp_tokenize_tpc,  sizeof(_tp_tokenize_tpc));
    tp_import(tp, 0, "tinypy.compiler.parse", _tp_parse_tpc, sizeof(_tp_parse_tpc));
    tp_import(tp, 0, "tinypy.compiler.encode", _tp_encode_tpc, sizeof(_tp_encode_tpc));
    tp_import(tp, 0, "tinypy.compiler.py2bc", _tp_py2bc_tpc, sizeof(_tp_py2bc_tpc));
    tp_ez_call(tp, "tinypy.compiler.py2bc", "_init", tp_None);
}
