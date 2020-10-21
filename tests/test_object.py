from tinypy.runtime.testing import UnitTest

class Create:
    def __init__(self, **args):
        self.__dict__.update(args)

    def create_with_a(a):
        return Create(a=a)
    create_with_a = staticmethod(create_with_a)

    def create_with_b(b):
        return Create(b=b)
    create_with_b = staticmethod(create_with_b)

# This is not our canonical way of defining a class.
class New:
    # __new__ is an implicitly classmethod.
    def __new__(kls, a):
        # be aware that we currently need to pass the args to __init__.
        self = object.__new__(kls, 3)
        self.a = a
        return self

    def __init__(self, b):
        self.b = b

class MyTest(UnitTest):

    def test_new(self):
        obj = New(3)
        assert obj.a == 3
        assert obj.b == 3

    def test_create(self):
        obj = Create.create_with_a(3)
        assert obj.a == 3
        obj = Create.create_with_b(3)
        assert obj.b == 3

t = MyTest()

t.run()
