from tinypy.runtime.testing import UnitTest

class MyClass:
    a = 3
    a = a

    def b(self, k):
        return k

#    def f(k):
#        return k
#    s = staticmethod(f)

class MyTest(UnitTest):

    def test_method(self):
        obj = MyClass()
        assert obj.b(3) == 3
        assert MyClass.b(obj, 3) == 3

    def test_static_attr(self):
        obj = MyClass()
        assert obj.a == 3
        assert MyClass.a == 3

    #def test_staticmethod(self):
    #    obj = MyClass()
    #    assert MyClass.s(3) == 3
    #    assert MyClass.f(3) == 3
    #    assert obj.s(3) == 3

t = MyTest()

t.run()
