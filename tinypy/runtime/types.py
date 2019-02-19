StringType = getmeta("")
ListType = getmeta([])
DictType = getmeta({})

class Exception:
    def __init__(self, message):
        self.message = message
    def __repr__(self):
        return self.message

class ImportError(Exception):
    pass    

def string_startswith(self, prefix):
    return self.find(prefix) == 0

StringType['startswith'] = string_startswith

"""
def format(a, b):
    r = []
    i = 0
    j = 0
    while i < len(a):
        if a[i] == '{':
            r.append(a[j:i])
            j = i
            while j < len(a):
                if a[j] == '}':
                    break
                j = j + 1

            name = a[i+1:j]
            r.append(str(b[name]))
            i = j
        i = i + 1
    return ''.join(r) 
"""
