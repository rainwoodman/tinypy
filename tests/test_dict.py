import sys
from tinypy.runtime.testing import UnitTest

class MyTest(UnitTest):

    def test_eq(self):
        assert {'a':1, 'b':2} != {'a':1, 'c':2}
        assert {'a':1} == {'a':1}
        assert {'a':1, 'b':2} == {'a':1, 'b':2}

    def test_lessthan(self):
        try:
            assert {'a':1, 'b':2} < {'a':1, 'c':2}
        except:
            assert 'TypeError' in sys.get_exc()[0]

    def test_update(self):
        a = {}
        b = {'a' : 1}
        a.update(b)
        assert a['a'] == 1

t = MyTest()

t.run()
