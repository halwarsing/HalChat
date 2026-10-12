package halwarsing.net.halchatandroid.halsm;

/**
 * Raw JNI of HalSM + HalSMExecutable (halsm_jni.c). It is the same set of functions that HalSMExecutableWASM.c
 * exports to JS (Module._xxx): every pointer (HalSMVariable*, HalSMArray*, Dict*, ...) is a long.
 * <p>
 * Use {@link HalSMExecutable} for the convenient API.
 * <p>
 * It is not thread safe: call it from one thread only.
 */
public final class HalSMNative {
    /** Module.print / Module.printErr / Module.input_text in JS. */
    public interface Host {
        void print(String text);

        void printError(String text);

        /** The answer of input(prompt) in HalSM. */
        String input(String prompt);
    }

    /**
     * The function of a module (HalSMFunctionCTypeDef): the pointers of the compiler, the arguments
     * (HalSMArray*) and the variables (Dict*). It returns HalSMVariable* (0 is null).
     */
    public interface FunctionC {
        long call(long hsmc, long args, long vars);
    }

    static {
        System.loadLibrary("hal_halsm");
    }

    private HalSMNative() {
    }

    /** host can be null (then print goes to Logcat and input gives an empty string). It is safe to call it again. */
    public static native void init(Host host);

    /** The names of HalSMVariableType, the index is the value of the type. */
    public static native String[] getVariableTypeNames();

    // Plugins

    /**
     * load(path,plugin_id) in JS. The file is read right here (it is not asynchronous) and the plugin is not run,
     * run it with {@link #run}. Returns the index of the compiler or a negative number if it failed:
     * -1 - the file cannot be read, -1-code - HalSMExecutable_load/HalFileSystem_open returned code.
     */
    public static native int load(String path, String pluginId);

    public static native String getPluginId(long hsmc);

    public static native void run(int index);

    /** 0 if there is no such index. */
    public static native long getHalSMCompiler(int index);

    /** Makes a compiler for a code (without a HalSMExecutable). Returns its index. */
    public static native int initHSMCCode();

    public static native void runCode(int index, String code, boolean isConsole);

    // HalSMCModule

    public static native long initModule(String name);

    public static native void addClassCToModule(long module, long classc);

    public static native void addFunctionToModule(long module, String name, long funcc);

    public static native void addVariableToModule(long module, String name, long var);

    public static native long getVariableFromModule(long module, long key);

    public static native int indexVariableFromModule(long module, long key);

    public static native void addModuleByModule(long module);

    public static native void addModule(String name, long funcs, long vars, long classes);

    // HalSMRunClassC

    public static native void setVariableRunClassC(long runclassc, long key, long var);

    public static native long getVariableRunClassC(long runclassc, long key);

    public static native int indexVariableRunClassC(long runclassc, long key);

    // HalSMClassC

    public static native long initClassC(String name);

    public static native void addFunctionToClassC(long classc, String name, long funcc);

    public static native void addVariableToClassC(long classc, String name, long var);

    public static native long getVariableFromClassC(long classc, long key);

    public static native long getFunctionFromClassC(long classc, long key);

    public static native int indexVariableFromClassC(long classc, long key);

    public static native int indexFunctionFromClassC(long classc, long key);

    // Functions

    /** Returns HalSMFunctionC*. Make a variable of it with variableInit(..., type("HalSMFunctionC")). */
    public static native long initFunctionC(FunctionC callback);

    public static native long runFunctionC(long hsmc, long func, long args, long vars);

    public static native long runLocalFunction(long func, long args, long vars);

    /** 0 if there is no such function. */
    public static native long getLocalFunction(long hsmc, long name);

    // Variables

    public static native long initVariable(long value, int type);

    public static native long variableInit(long value, int type);

    public static native int getTypeVariable(long var);

    public static native long getValueVariable(long var);

    // HalSMArray

    public static native long initHalSMArray();

    public static native long getSizeHalSMArray(long arr);

    public static native long getVariableFromHalSMArray(long arr, int index);

    public static native void appendToHalSMArray(long arr, long value);

    public static native void setToHalSMArray(long arr, long value, long index);

    public static native int indexFromHalSMArray(long arr, long value);

    // Dict

    public static native long initDict();

    public static native long initDictElement(long key, long value);

    public static native int getSizeDict(long dict);

    public static native long getDictElementFromDict(long dict, int index);

    /** 0 if there is no such element. */
    public static native long getDictElementFromDictByKey(long dict, long key);

    /** 0 if there is no such element. */
    public static native long getDictElementFromDictByValue(long dict, long value);

    public static native int indexDictElementFromDictByKey(long dict, long key);

    public static native int indexDictElementFromDictByValue(long dict, long value);

    public static native void putToDict(long dict, long elem);

    public static native long getValueFromDictElement(long elem);

    public static native long getKeyFromDictElement(long elem);

    // Values: getXFromValue takes getValueVariable(var), xToValue makes a value for variableInit

    public static native String getStringFromValue(long value);

    /** The decimal text of int (it can be bigger than long). */
    public static native String getIntFromValue(long value);

    public static native long getLongIntFromValue(long value);

    public static native String getDoubleFromValue(long value);

    /** One C char, plugin.js uses it to read HalSMBool (=== 1). */
    public static native int getCharFromValue(long value);

    public static native int getUnsignedIntFromValue(long value);

    public static native long getHalSMClassCFromValue(long value);

    /** HalStringFormatChar* (not a value of str). Use {@link #stringToVar} to make a str variable. */
    public static native long stringToValue(String str);

    public static native long stringToVar(String str);

    public static native long intToValue(int n);

    /** Not in WASM: 64-bit integer (intToValue is 32-bit). */
    public static native long longToValue(long n);

    public static native long floatToValue(String n);

    public static native long charToValue(int n);

    public static native long unsignedIntToValue(int n);

    public static native long getHalSMNull();

    public static native long getHalSMTrue();

    public static native long getHalSMFalse();

    // HalSMSetArg

    public static native long getNameHalSMSetArg(long setarg);

    public static native long getValueHalSMSetArg(long setarg);
}
