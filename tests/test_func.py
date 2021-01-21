from tinypy.runtime.testing import UnitTest

class FuncTest(UnitTest):

    def test_attrs(self):
        def func(a, b, c=3, *args, **kwargs):
            pass
        assert func.__args__ == ('a', 'b', 'c')
        assert func.__defaults__ == [3]
        assert func.__varargs__ == 'args'
        assert func.__varkw__ == 'kwargs'

    def test_pos(self):
        def func(a, b):
            return a, b
        assert func(1, 2) == (1, 2)

    def test_pos_args(self):
        def func(a, b, *c):
            return a, b, c
        assert func(1, 2, 3, 4) == (1, 2, (3, 4))

    def test_default_pos(self):
        def func(a, b=9, c=9):
            return a, b, c
        assert func(1) == (1, 9, 9)
        assert func(1, 2) == (1, 2, 9)
        assert func(1, 2, 3) == (1, 2, 3)

    def test_default_pos_mixed(self):
        def func(a, b=9, c=9):
            return a, b, c
        assert func(1, b=9) == (1, 9, 9)

    def test_kwargs(self):
        def func(**c):
            return c
        c = func(c1=1, c2=2)
        assert c['c1'] == 1
        assert c['c2'] == 2

    def test_args(self):
        def func(*args):
            return args
        c = func(1, 2)
        assert c == [1, 2]

    def test_mix_kw_default(self):
        def func(a, b=9, c=9):
            return a, b, c
        assert func(1, 2, c=3) == (1, 2, 3)
        assert func(1, b=2, c=3) == (1, 2, 3)
        assert func(a=1, b=2, c=3) == (1, 2, 3)

    def test_args_and_kwargs(self):
        def func(*args, **kwargs):
            return args, kwargs
        c, d = func(1, 2, d=1)
        assert c == [1, 2]
        assert d == {'d': 1}

    def test_pos_kwargs(self):
        def func(a, **kwargs):
            return a, kwargs
        a, d = func(1, d1=2, d2=3)
        assert a == 1
        assert d['d1'] == 2
        assert d['d2'] == 3

    def known_failure_test_lparams_overwrites_dparams(self):
        # We are following CPython now.
        def func(a, b):
            return a, b
        # unlikely CPython, we use positional argument
        # if dparams conflicts with positional argument.
        a, b = func(1, **{'a':2, 'b':3})
        assert a == 1
        assert b == 3

    def test_dparams_overwrites_default(self):
        def func(a, b=1):
            return a, b
        # dparams overwrites default value.
        a, b = func(**dict(a=2, b=3))
        assert a == 2
        assert b == 3

    def test_lparams_overwrites_default(self):
        def func(a, b=2):
            return a, b
        # lparams overwrites default value.
        a, b = func(*[2, 3])
        assert a == 2
        assert b == 3


if __name__ == '__main__':
    t = FuncTest()
    t.run()
