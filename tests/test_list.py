from tinypy.runtime.testing import UnitTest

class MyTest(UnitTest):

    def test_lessthan(self):
        assert [1, 2] < [2]
        assert [1, 2] <= [2]
        assert [1] < [2]
        assert [1] <= [2]
        assert [] < [1]

    def test_greaterthan(self):
        assert [2] > [1]
        assert [1, 2] > [1]
        assert [1, 2] >= [1]
        assert [1, 2] >= [1, 2]
        assert [2] > []

    def test_equal(self):
        assert [1] == [1]
        assert [1, 2] == [1, 2]
        assert [] == []

t = MyTest()

t.run()
