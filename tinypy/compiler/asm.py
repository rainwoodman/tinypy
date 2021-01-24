from tinypy.compiler.boot import *
from tinypy.compiler import opcodes

def prepare(x):
    """ Prepares the line for processing by breaking it into tokens,
        removing empty tokens and stripping whitespace """
    x = x.split('#', 1)[0]
    words = [w.strip() for w in x.split(":", 3)]
    if len(words) == 3:
        _, i, abc = words
        d = ""
    else:
        _, i, abc, d = words
    a, b, c = [int(w) for w in abc.split(None, 3)]
    return i, a, b, c, d

def from_hexform(x):
    """ Removes outermost quotes from a string, if they exist """    
    b = []
    for i in range(0, len(x), 2):
        b.append(int(x[i:i+2], 16))
    return bytes(b)

def assemble(asmc):
    asmc = asmc.strip()
    asmc = asmc.split('\n')
    bc = []
    ops = opcodes.codes
    for line in asmc:
        current = prepare(line)
        i,a,b,c,d = current
        bc.append(bytes([ops[i], a, b, c]))
        if i == "LINE":
            n = a * 4
            d = d.encode()
            text = d + b'\0' * (n - len(d))
            bc.append(text)
        elif i == "FILE":
            n = b * 256 + c
            d = d.encode()
            text = d + b"\0"*(n-len(d)%4)
            bc.append(text)
        elif i == "NAME":
            n = b * 256 + c
            d = d.encode()
            text = d + b"\0"*(n-len(d)%4)
            bc.append(text)
        elif i == "NUMBER":
            d = from_hexform(d)
            bc.append(d)
    bc = b''.join(bc)
    return bc
 
if __name__ == '__main__':
    asmc = load(ARGV[1])
    bc = assemble(asmc)
    save(ARGV[2], bc)
