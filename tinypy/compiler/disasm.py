from tinypy.compiler.boot import *
from tinypy.compiler import opcodes

def isupper(x):
    return ord(x) >= ord("A") and ord(x) <= ord("Z")

def pad(s, n):
    p = ""
    if n < 0:
        m = -n - len(s)
        if m > 0: p = " " * m
        return p + s
    m = n - len(s)
    if m > 0: p = " " * m
    return s + p

def text(x, ip, bc):
    return bytes(bc[ip:ip+x])

def trim(x):
    txt = []
    for c in x:
        if ord(c):
            txt.append(c)
    return "".join(txt)

def disassemble(bc):
    bc = [x for x in bc]
    asmc = []
    ip = 0
    names = opcodes.names
    while ip < len(bc):
        i, a, b, c = bc[ip:ip + 4]
        line = ""
        line += pad(str(ip), 4) + ":" 
        line += pad(names[i], 10) + ":" 
        line += " " + pad(str(a), -3)
        line += " " + pad(str(b), -3)
        line += " " + pad(str(c), -3)
        ip += 4
        if i == opcodes.LINE:
            n = a * 4
            line += " " + str(text(n,ip,bc))
            line = trim(line)
            ip += n
        elif i == opcodes.VAR:
            n = b * 256 + c
            line += " " + str(a) + ": " + str(text(n,ip,bc))
            line = trim(line)
            ip += (int(n / 4) + 1) * 4
        elif i == opcodes.STRING:
            n = b * 256 + c
            line += " " + str(text(n,ip,bc))
            line = trim(line)
            ip += (int(n / 4) + 1) * 4
        elif i == opcodes.NUMBER:
            f = unpack('=' + chr(b), text(c,ip,bc))
            line += " " + str(f)
            ip += c
        asmc.append(line)
        print(line)
    asmc = "\n".join(asmc)
    return asmc
    
if __name__ == "__main__":
    bc = load(ARGV[1])
    asmc = disassemble(bc)
    print(asmc)
