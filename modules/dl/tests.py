import dl

sqrt = dl.load('libm.so.6', 'sqrt', 'd', 'd')

for value in [2, 10, 100]:
    result = sqrt(value * value)
    print('sqrt(' + str(value * value) + ') = ' + str(result))
    if result != value:
        raise "unexpected result"
