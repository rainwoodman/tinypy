import dl

sqrt = dl.load('libm.so.6', 'sqrt', 'd', 'd')

for value in [2, 10, 100]:
    result = sqrt(value * value)
    print('sqrt(' + str(value * value) + ') = ' + str(result))
    if result != value:
        raise "unexpected result"

print(dl.size('III'))

data = dl.pack('III', [1, 2, 3])
print(data)
result = dl.unpack('III', data)
for i in result:
    print(i)
