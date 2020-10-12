from tinypy.runtime.testing import UnitTest

class MyTest(UnitTest):

    def test_eq(self):
        assert {'a':1, 'b':2} != {'a':1, 'c':2}
        assert {'a':1} == {'a':1}
        assert {'a':1, 'b':2} == {'a':1, 'b':2}

t = MyTest()

t.run()
