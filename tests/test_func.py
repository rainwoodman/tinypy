from tinypy.runtime.testing import UnitTest

class FuncTest(UnitTest):

    def test_attrs(self):
        def func(a, b, c=3, *args, **kwargs):
            pass
        assert func.__args__ == ('a', 'b', 'c')
        assert func.__defaults__ == [3]
        assert func.__varargs__ == 'args'
        assert func.__varkw__ == 'kwargs'

    def test_fixme(self):
        def func(): pass
        print(func.__adfwefw__) # getting with global?

if __name__ == '__main__':
    t = FuncTest()
    t.run()
