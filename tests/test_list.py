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

    # FIXME:
    # As we don't have an iterable type, there is not much point
    # to define min and max.
    # We shall probably remove min and max from builtins.
    def test_max(self):
        assert max(1, 2, 3) == 3
        assert max(3, 1, 2) == 3
        assert max(3, 1, 3) == 3

    def test_min(self):
        assert min(1, 2, 3) == 1
        assert min(3, 1, 2) == 1
        assert min(2, 1, 1) == 1

t = MyTest()

t.run()
