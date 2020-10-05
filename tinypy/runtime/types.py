StringType = getmeta("")
ListType = getmeta([])
DictType = getmeta({})

class Exception:
    def __init__(self, *args):
        self.args = args
    def __repr__(self):
        return str(self.args)

class ImportError(Exception):
    pass

def startswith(self, prefix):
    return self.find(prefix) == 0

StringType['startswith'] = startswith

def format(s, d):
    r = []
    i = 0
    j = 0
    n = len(s)
    while i < n:
        if s[i] == '{':
            r.append(s[j:i])
            j = i
            while j < n:
                if s[j] == '}':
                    j = j + 1
                    break
                j = j + 1

            spec = s[i+1:j-1]
            #name, fmt = spec.split(':')
            foo = d[spec]  # eval requires the compiler: eval(spec, d)
            #foo = spec
            # print('foo', foo, spec, d)
            r.append(str(foo))
            i = j - 1
        i = i + 1
    r.append(s[j:i])
    return ''.join(r)

StringType['format'] = format
