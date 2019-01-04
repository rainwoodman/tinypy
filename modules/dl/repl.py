import dl

old_str = BUILTINS['str']

def repr(value, seen=[]):
    if istype(value, "string"):
        return "'" + my_str(value, seen).replace("'", "\\'") + "'"
    return my_str(value, seen)

def my_str(value, seen=[]):
    if value in seen:
        return '...'
    seen.append(value)
    if istype(value, "list"):
        return '[' + ', '.join([repr(v, seen) for v in value]) + ']'
    elif istype(value, "dict"):
        return '{' + ', '.join([repr(k, seen) + ': ' + repr(value[k], seen) for k in value]) + '}'
    return old_str(value)

def globals():
    return BUILTINS['__globals__']

BUILTINS['__globals__'] = {}
BUILTINS['globals'] = globals
BUILTINS['str'] = my_str
BUILTINS['repr'] = repr

fgets = dl.load('', 'fgets', 'S', 'SI*')
puts = dl.load('', 'printf', 'V', 'S')
handle = dl.open('')
stdin = dl.sym(handle, '_IO_2_1_stdin_')

data = " " * 1024

while True:
    puts('> ')
    source = fgets(data, len(data), stdin)
    if source is None:
        break
    if source.strip() == '':
        continue
    try:
        result = eval(source, globals())
        print(str(result))
    except:
        print(dl.exception())
print('bye')

