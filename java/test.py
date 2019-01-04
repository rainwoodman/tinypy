import jni

TinyPy = jni.find_class("TinyPy")
constructor = jni.get_method_id(TinyPy, "<init>", "()V")
tp = jni.new_object(TinyPy, constructor)
say = jni.get_method_id(TinyPy, "say2", "(Ljava/lang/String;)V")
jni.call_object_method(tp, say, "hello world")

foo = "hi there"
