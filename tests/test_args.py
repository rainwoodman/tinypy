from tinypy.runtime.testing import UnitTest

class MyTest(UnitTest):
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

    # We need to fix this.
    def known_failure_test_default_pos_mixed(self):
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

    # these tests require a change of calling convention
    # in encode.py and tp_call() to fix.
    def known_failure_test_mix_kw_default(self):
        def func(a, b=9, c=9):
            return a, b, c
        assert func(1, 2, c=3) == (1, 2, 3)
        assert func(1, b=2, c=3) == (1, 2, 3)
        assert func(a=1, b=2, c=3) == (1, 2, 3)

    def known_failure_test_args_and_kwargs(self):
        def func(*args, **kwargs):
            return args, kwargs
        c, d = func(1, 2, d=1)
        assert c == [1, 2]
        assert d == {'d': 1}

    def known_failure_test_pos_kwargs(self):
        def func(a, **kwargs):
            return a, kwargs
        a = func(1, d1=2, d2=3)
        assert a == 1
        assert d['d1'] == 2
        assert d['d2'] == 3

t = MyTest()

t.run()
