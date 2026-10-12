package halwarsing.net.halchatandroid.halsm;

import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * The same things that plugin.js does over Module._xxx (HalChatPlugins and HalSMExecutable classes), but over
 * {@link HalSMNative}. The names are the same, so it is easy to sync with plugin.js of HalChatWeb:
 * <ul>
 * <li>Module.HalSM.null/true/false/nulldict/nullarray - {@link #nullVar}, {@link #trueVar}, {@link #falseVar},
 * {@link #nullDict}, {@link #nullArray}</li>
 * <li>Module.HalSM.HalSMVariableType.x - {@link #type}("x")</li>
 * <li>HalSMVarToJsValue/jsValueToHalSMVar/getHalSMArguments/dictToHalSMDict - HalSMVarToJava/javaToHalSMVar/getHalSMArguments/dictToHalSMDict</li>
 * <li>MainHalChatPlugins.addEvent/runEvent/loadPlugin/parseArgs - the same methods here</li>
 * <li>HalSMExecutable.addModule/getHalSMCompiler/getLocalFunction/runLocalFunction - the same methods here</li>
 * </ul>
 * The values: null - HalSMNull, String - str, Byte/Short/Integer/Long - int, Float/Double - double,
 * Boolean - bool, Object[]/List - HalSMArray, Map - HalSMDict.
 * <p>
 * It is not thread safe: call it from one thread only. The functions of modules are called from the thread that runs
 * the plugin (run/runEvent/runLocalFunction).
 */
public final class HalSMExecutable {
    private static final String TAG="HalSMExecutable";
    private static final int MAX_DEPTH=64;

    //Module.HalSM.HalSMVariableType
    public static final Map<String,Integer> variableTypes=new HashMap<>();

    static {
        String[] names=HalSMNative.getVariableTypeNames();
        for(int i=0;i<names.length;i++) {
            variableTypes.put(names[i],i);
        }
    }

    private static final int T_INT=type("int");
    private static final int T_DOUBLE=type("double");
    private static final int T_VOID=type("void");
    private static final int T_ARRAY=type("HalSMArray");
    private static final int T_STR=type("str");
    private static final int T_FUNCTION_C=type("HalSMFunctionC");
    private static final int T_CLASS_C=type("HalSMClassC");
    private static final int T_SET_ARG=type("HalSMSetArg");
    private static final int T_NULL=type("HalSMNull");
    private static final int T_LOCAL_FUNCTION=type("HalSMLocalFunction");
    private static final int T_BOOL=type("HalSMBool");
    private static final int T_DICT=type("HalSMDict");

    //Module.HalSM.null, true, false, nulldict, nullarray (HalSMVariable*, HalSMVariable*, HalSMVariable*, Dict*, HalSMArray*)
    public long nullVar;
    public long trueVar;
    public long falseVar;
    public long nullDict;
    public long nullArray;
    public boolean isInitialized=false;

    //Module.plugins_hsme: id of the plugin -> index of the compiler; Module.plugins_data: id of the plugin -> data of the plugin
    public final Map<String,Integer> pluginsHSME=new HashMap<>();
    public final Map<String,Object> pluginsData=new HashMap<>();
    public boolean isLoadedPlugins=false;

    //Handler of an event: HalSMLocalFunction* and the key of pluginTag (a[i][0] and a[i][1] in JS)
    public static class Event {
        public final long func;
        public final String namePluginData;

        public Event(long func,String namePluginData) {
            this.func=func;
            this.namePluginData=namePluginData;
        }
    }

    public final Map<String,List<Event>> events=new LinkedHashMap<>();

    public HalSMExecutable() {
        events.put("onStart",new ArrayList<>());

        //User
        events.put("onUserSendMessage",new ArrayList<>());
        events.put("onUserAddMessage",new ArrayList<>());

        //Local Bot
        events.put("onLocalBotSendedMessage",new ArrayList<>());

        //Custom Btn Disabled
        events.put("onClickBtnPluginDisabled",new ArrayList<>());
    }

    //Module.HalSM.HalSMVariableType[name]
    public static int type(String name) {
        Integer type=variableTypes.get(name);
        if(type==null) {
            throw new IllegalArgumentException("Unknown HalSMVariableType: "+name);
        }
        return type;
    }

    //Module.ccall("init") and the variables of Module.HalSM. The HalChat module is not added here, use addModule.
    public void init(HalSMNative.Host host) {
        HalSMNative.init(host);
        if(isInitialized) {
            return;
        }
        nullVar=HalSMNative.getHalSMNull();
        trueVar=HalSMNative.getHalSMTrue();
        falseVar=HalSMNative.getHalSMFalse();
        nullDict=HalSMNative.initDict();
        nullArray=HalSMNative.initHalSMArray();
        isInitialized=true;
    }

    /* Convert */

    //HalSMArrayToJSArray
    public static List<Object> HalSMArrayToList(long arr) {
        return HalSMArrayToList(arr,0);
    }

    private static List<Object> HalSMArrayToList(long arr,int depth) {
        List<Object> out=new ArrayList<>();
        long size=HalSMNative.getSizeHalSMArray(arr);

        for(int i=0;i<size;i++) {
            out.add(HalSMVarToJava(HalSMNative.getVariableFromHalSMArray(arr,i),depth+1));
        }

        return out;
    }

    //HalSMDictToJSDict
    public static Map<String,Object> HalSMDictToMap(long dict) {
        return HalSMDictToMap(dict,0);
    }

    private static Map<String,Object> HalSMDictToMap(long dict,int depth) {
        Map<String,Object> out=new LinkedHashMap<>();
        int size=HalSMNative.getSizeDict(dict);

        for(int i=0;i<size;i++) {
            long dictElem=HalSMNative.getDictElementFromDict(dict,i);
            Object dictKey=HalSMVarToJava(HalSMNative.getKeyFromDictElement(dictElem),depth+1);
            Object dictValue=HalSMVarToJava(HalSMNative.getValueFromDictElement(dictElem),depth+1);
            out.put(String.valueOf(dictKey),dictValue);
        }

        return out;
    }

    //HalSMVarToJsValue: the other types (functions, classes...) are null like in JS
    public static Object HalSMVarToJava(long v) {
        return HalSMVarToJava(v,0);
    }

    private static Object HalSMVarToJava(long v,int depth) {
        if(depth>MAX_DEPTH) {
            throw new IllegalArgumentException("The variable is nested too deep");
        }

        int type=HalSMNative.getTypeVariable(v);
        long value=HalSMNative.getValueVariable(v);

        if(type==T_STR) {
            return HalSMNative.getStringFromValue(value);
        } else if(type==T_INT) {
            return HalSMNative.getLongIntFromValue(value);
        } else if(type==T_DOUBLE) {
            try {
                return Double.parseDouble(HalSMNative.getDoubleFromValue(value));
            } catch (NumberFormatException e) {
                return Double.NaN;
            }
        } else if(type==T_LOCAL_FUNCTION) {
            return value;
        } else if(type==T_NULL||type==T_VOID) {
            return null;
        } else if(type==T_BOOL) {
            return HalSMNative.getCharFromValue(value)==1;
        } else if(type==T_ARRAY) {
            return HalSMArrayToList(value,depth);
        } else if(type==T_DICT) {
            return HalSMDictToMap(value,depth);
        }
        return null;
    }

    //jsValueToHalSMVar. Returns HalSMVariable*
    public static long javaToHalSMVar(Object arg) {
        return javaToHalSMVar(arg,0);
    }

    private static long javaToHalSMVar(Object arg,int depth) {
        if(depth>MAX_DEPTH) {
            throw new IllegalArgumentException("The value is nested too deep");
        }

        if(arg==null) {
            return HalSMNative.getHalSMNull();
        } else if(arg instanceof CharSequence) {
            return HalSMNative.stringToVar(arg.toString());
        } else if(arg instanceof Integer||arg instanceof Short||arg instanceof Byte) {
            return HalSMNative.variableInit(HalSMNative.intToValue(((Number)arg).intValue()),T_INT);
        } else if(arg instanceof Long) {
            return HalSMNative.variableInit(HalSMNative.longToValue((Long)arg),T_INT);
        } else if(arg instanceof Double||arg instanceof Float) {
            //NaN and Infinity cannot be a HalSMDouble, BigDecimal throws for them
            String text=BigDecimal.valueOf(((Number)arg).doubleValue()).toPlainString();
            return HalSMNative.variableInit(HalSMNative.floatToValue(text),T_DOUBLE);
        } else if(arg instanceof Boolean) {
            return (Boolean)arg?HalSMNative.getHalSMTrue():HalSMNative.getHalSMFalse();
        } else if(arg instanceof Object[]) {
            return HalSMNative.variableInit(getHalSMArguments(Arrays.asList((Object[])arg),depth+1),T_ARRAY);
        } else if(arg instanceof List) {
            return HalSMNative.variableInit(getHalSMArguments((List<?>)arg,depth+1),T_ARRAY);
        } else if(arg instanceof Map) {
            return HalSMNative.variableInit(dictToHalSMDict((Map<?,?>)arg,depth+1),T_DICT);
        } else if(arg instanceof JSONArray) {
            List<Object> objects=new ArrayList<>();
            JSONArray jarg=(JSONArray)arg;
            try {
                for(int i=0;i<jarg.length();i++) {
                    objects.add(jarg.get(i));
                }
            } catch (JSONException e) {
                Log.e(TAG,"Error in javaToHalSMVar JSONArray: ",e);
            }
            return HalSMNative.variableInit(getHalSMArguments(objects,depth+1),T_ARRAY);
        } else if(arg instanceof JSONObject) {
            Map<String,Object> out=new HashMap<>();
            JSONObject jarg=(JSONObject)arg;

            try {
                for (Iterator<String> it = jarg.keys(); it.hasNext(); ) {
                    String key = it.next();
                    out.put(key,jarg.get(key));
                }
            } catch (JSONException e) {
                Log.e(TAG,"Error in javaToHalSMVar JSONObject: ",e);
            }

            return HalSMNative.variableInit(dictToHalSMDict(out,depth+1),T_DICT);
        }
        throw new IllegalArgumentException("Cannot convert "+arg.getClass().getName()+" to HalSMVariable");
    }

    //dictToHalSMDict. Returns Dict*
    public static long dictToHalSMDict(Map<?,?> dict) {
        return dictToHalSMDict(dict,0);
    }

    private static long dictToHalSMDict(Map<?,?> dict,int depth) {
        long out=HalSMNative.initDict();

        for(Map.Entry<?,?> entry:dict.entrySet()) {
            long dictelem=HalSMNative.initDictElement(HalSMNative.stringToVar(String.valueOf(entry.getKey())),javaToHalSMVar(entry.getValue(),depth));
            HalSMNative.putToDict(out,dictelem);
        }

        return out;
    }

    //getHalSMArguments. Returns HalSMArray*
    public static long getHalSMArguments(List<?> args) {
        return getHalSMArguments(args,0);
    }

    private static long getHalSMArguments(List<?> args,int depth) {
        long out=HalSMNative.initHalSMArray();

        for(Object arg:args) {
            HalSMNative.appendToHalSMArray(out,javaToHalSMVar(arg,depth));
        }

        return out;
    }

    /* Events */

    //Returns the index of the handler or -1 if there is no such event
    public int addEvent(String name,long func) {
        return addEvent(name,func,"");
    }

    public int addEvent(String name,long func,String namePluginData) {
        List<Event> list=events.get(name);
        if(list==null) {
            return -1;
        }
        list.add(new Event(func,namePluginData));
        return list.size()-1;
    }

    //The handler gets args and the last argument is the value of pluginTag[namePluginData] (false if there is no such key)
    public boolean runEvent(String name,List<?> args,Map<String,?> pluginTag) {
        List<Event> a=events.get(name);
        if(a==null) {
            return false;
        }
        if(pluginTag==null) {
            pluginTag=new HashMap<>();
        }

        long argsH=getHalSMArguments(args);
        long lindex=HalSMNative.getSizeHalSMArray(argsH);
        long argPlugin=HalSMNative.getHalSMNull();
        HalSMNative.appendToHalSMArray(argsH,argPlugin);
        for(Event event:a) {
            if(event.namePluginData.length()==0||pluginTag.containsKey(event.namePluginData)) {
                HalSMNative.setToHalSMArray(argsH,javaToHalSMVar(pluginTag.containsKey(event.namePluginData)?pluginTag.get(event.namePluginData):false),lindex);
                HalSMNative.runLocalFunction(event.func,argsH,nullDict);
            }
        }
        return true;
    }

    /* Plugins */

    public int loadPlugin(String path,String pluginId) {
        return loadPlugin(path,pluginId,new ArrayList<>());
    }

    public int loadPlugin(String path,String pluginId,Object data) {
        int index=HalSMNative.load(path,pluginId);

        if(index<0) {
            Log.e(TAG,"Failed to load the plugin "+pluginId+": "+path+" ("+index+")");
            return -1;
        }

        //The plugin can ask its data while it is running (getPluginData)
        pluginsHSME.put(pluginId,index);
        pluginsData.put(pluginId,data);

        HalSMNative.run(index);

        isLoadedPlugins=true;
        return index;
    }

    public Object getPluginData(String pluginId) {
        if(!pluginsData.containsKey(pluginId)) {
            return null;
        }

        return pluginsData.get(pluginId);
    }

    public Map<String,Object> parseArgs(long args,String params,String[] keysVars) {
        return parseArgs(args,params,keysVars,"");
    }

    public Map<String,Object> parseArgs(long args,String params,String[] keysVars,String additional) {
        Map<String,Object> out=new LinkedHashMap<>();
        int size=(int)HalSMNative.getSizeHalSMArray(args);

        if(size<params.length())return null;

        params+=additional;

        int j=0;

        for(int i=0;i<size;i++) {
            char c=j<params.length()?params.charAt(j):'\0';
            long v=HalSMNative.getVariableFromHalSMArray(args,i);
            long value=HalSMNative.getValueVariable(v);
            int type=HalSMNative.getTypeVariable(v);

            String key=j<keysVars.length?keysVars[j]:null;

            if(type==T_SET_ARG) {
                String sname=String.valueOf(HalSMVarToJava(HalSMNative.getNameHalSMSetArg(value)));
                long sval=HalSMNative.getValueHalSMSetArg(value);
                int ind=Arrays.asList(keysVars).indexOf(sname);

                if(ind!=-1) {
                    c=ind<params.length()?params.charAt(ind):'\0';
                    v=sval;
                    value=HalSMNative.getValueVariable(sval);
                    type=HalSMNative.getTypeVariable(sval);

                    key=sname;

                    j--;
                } else {
                    Log.e(TAG,"Failed SetArg: "+sname+";"+HalSMVarToJava(sval));
                    return null;
                }
            }

            switch(c) {
                case 's':
                    if(type!=T_STR)return null;
                    break;
                case 'i':
                    if(type!=T_INT)return null;
                    break;
                case 'f':
                    if(type!=T_DOUBLE)return null;
                    break;
                case 'b':
                    if(type!=T_BOOL)return null;
                    break;
                case 'a':
                    if(type!=T_ARRAY)return null;
                    break;
                case 'd':
                    if(type!=T_DICT)return null;
                    break;
                case 'n':
                    if(type!=T_NULL)return null;
                    break;
                case '-':
                    j++;
                    continue;
                case 'l':
                    if(type!=T_LOCAL_FUNCTION)return null;
                    break;
                default:
                    return null;
            }
            out.put(String.valueOf(key),HalSMVarToJava(v));

            j++;
        }

        return out;
    }

    /* HalSMExecutable */

    //initializeVars()).
    public void addModule(String name,Class<?> moduleClass) {
        long classes=HalSMNative.initDict();
        for(Class<?> nested:moduleClass.getDeclaredClasses()) {
            if(!Modifier.isStatic(nested.getModifiers()))continue;

            long cls=HalSMNative.initClassC(nested.getSimpleName());
            for(Method m:nested.getDeclaredMethods()) {
                if(!isModuleFunction(m))continue;
                HalSMNative.addFunctionToClassC(cls,m.getName(),HalSMNative.initFunctionC(functionOf(m)));
            }
            for(Map.Entry<String,Object> var:initializeVars(nested).entrySet()) {
                HalSMNative.addVariableToClassC(cls,var.getKey(),javaToHalSMVar(var.getValue()));
            }

            HalSMNative.putToDict(
                classes,
                HalSMNative.initDictElement(
                    HalSMNative.stringToVar(nested.getSimpleName()),
                    HalSMNative.variableInit(cls,T_CLASS_C)
                )
            );
        }

        HalSMNative.addModule(name,funcsDict(moduleClass),varsDict(moduleClass),classes);
    }

    private long funcsDict(Class<?> cls) {
        long funcs=HalSMNative.initDict();
        for(Method m:cls.getDeclaredMethods()) {
            if(!isModuleFunction(m))continue;
            HalSMNative.putToDict(
                funcs,
                HalSMNative.initDictElement(
                    HalSMNative.stringToVar(m.getName()),
                    HalSMNative.variableInit(HalSMNative.initFunctionC(functionOf(m)),T_FUNCTION_C)
                )
            );
        }
        return funcs;
    }

    private long varsDict(Class<?> cls) {
        long vars=HalSMNative.initDict();
        for(Map.Entry<String,Object> var:initializeVars(cls).entrySet()) {
            HalSMNative.putToDict(
                vars,
                HalSMNative.initDictElement(
                    HalSMNative.stringToVar(var.getKey()),
                    javaToHalSMVar(var.getValue())
                )
            );
        }
        return vars;
    }

    private static boolean isModuleFunction(Method m) {
        return Modifier.isStatic(m.getModifiers())&&!m.isSynthetic()&&!m.isBridge()
            &&m.getReturnType()==long.class
            &&Arrays.equals(m.getParameterTypes(),new Class<?>[]{long.class,long.class,long.class});
    }

    private static HalSMNative.FunctionC functionOf(Method m) {
        m.setAccessible(true);
        return (hsmc,args,vars)->{
            try {
                return (long)m.invoke(null,hsmc,args,vars);
            } catch(ReflectiveOperationException e) {
                Throwable cause=e.getCause()!=null?e.getCause():e;
                throw new RuntimeException("HalSM function "+m.getDeclaringClass().getSimpleName()+"."+m.getName()+" failed",cause);
            }
        };
    }

    @SuppressWarnings("unchecked")
    private static Map<String,Object> initializeVars(Class<?> cls) {
        Method m;
        try {
            m=cls.getDeclaredMethod("initializeVars");
        } catch(NoSuchMethodException e) {
            return Collections.emptyMap();
        }
        if(!Modifier.isStatic(m.getModifiers()))return Collections.emptyMap();
        m.setAccessible(true);
        try {
            Object result=m.invoke(null);
            return result==null?Collections.emptyMap():(Map<String,Object>)result;
        } catch(ReflectiveOperationException e) {
            Throwable cause=e.getCause()!=null?e.getCause():e;
            throw new RuntimeException("HalSM initializeVars() failed for "+cls.getSimpleName(),cause);
        }
    }

    //Returns HalSMCompiler* (0 if there is no such index)
    public long getHalSMCompiler(int index) {
        return HalSMNative.getHalSMCompiler(index);
    }

    //hsmc: HalSMCompiler*
    //name: HalSMVariable*
    //Returns HalSMLocalFunction* (0 if there is no such function)
    public long getLocalFunction(long hsmc,long name) {
        return HalSMNative.getLocalFunction(hsmc,name);
    }

    //Runs the function of the plugin by the name. Returns the result of the function (null if there is nothing to run).
    public Object runLocalFunction(String pluginId,String name,List<?> args) {
        Integer index=pluginsHSME.get(pluginId);
        if(index==null)return null;
        long hsmc=getHalSMCompiler(index);
        long func=getLocalFunction(hsmc,HalSMNative.stringToVar(name));
        if(func==0)return null;
        long argsH=getHalSMArguments(args);
        return HalSMVarToJava(HalSMNative.runLocalFunction(func,argsH,nullDict));
    }

    //Module._getPluginId(hsmc): the id of the plugin of the compiler (for the functions of modules)
    public String getPluginId(long hsmc) {
        return HalSMNative.getPluginId(hsmc);
    }
}
