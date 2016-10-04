import java.util.Scanner;

class TinyPy {
    public static void main(String [] args) {
        init();
        run("test.py");
        say(getString("foo"));
        say(eval("say"));
        deinit();
    }

    static void gdb() {
        System.out.println("enter number when gdb is ready");
        Scanner sc = new Scanner(System.in);
        int intNum = sc.nextInt();
    }

    void say2(String message) {
        System.out.println(message);
    }

    static void say(String message) {
        System.out.println(message);
    }

    static public native void init();
    static public native String run(String filename);
    static public native String eval(String code);
    static public native void deinit();

    static public native String getString(String name);
    static public native void call();

    static public native long getPyObject(String name);

    static {
        System.loadLibrary("tinypy");
    }
}
