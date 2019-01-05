from tinypy.language.builtins import *
import sys

class Exception:
    def __init__(self, message):
        self.message = message
    def __repr__(self):
        return self.message

class ImportError(Exception):
    pass    

def compile(s, fname):
    # must be here because at import time of builtin,
    # py2bc is not imported yet
    from tinypy.compiler.py2bc import _compile
    return _compile(s, fname)

def __import__(name):
    # this function overwrites the temporary
    # C based __import__ which only bootstraps the compiler

    if name in sys.modules:
        return sys.modules[name]
    py = name+".py"
    tpc = name+".tpc"
    if exists(py):
        if not exists(tpc) or mtime(py) > mtime(tpc):
            s = load(py)
            code = compile(s,py)
            save(tpc,code)

    if not exists(tpc):
        raise ImportError("Cannot import the compiled script from " +  tpc)

    code = load(tpc)
    g = {'__name__':name,'__code__':code}
    g['__dict__'] = g
    sys.modules[name] = g
    exec(code,g)
    return g

def import_fname(fname,name):
    g = {}
    g['__name__'] = name
    sys.modules[name] = g
    s = load(fname)
    code = compile(s,fname)
    g['__code__'] = code
    exec(code, g)
    return g

def _entry_point():
    return import_fname(sys.argv[0], '__main__')
            
