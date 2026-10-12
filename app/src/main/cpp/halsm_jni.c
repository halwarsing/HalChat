//
// JNI binding for HalSM + HalSMExecutable.
// Java class: halwarsing.net.halchatandroid.halsm.HalSMNative
//
// It mirrors the exports of HalSMExecutableWASM.c one to one (pointers are passed as jlong,
// exactly like the numbers that JS gets from Module._xxx), so the Java code can be written the
// same way as plugin.js. The logic is the same as in the WASM file (bugs too, they are fixed together),
// the differences are marked with "JNI:".
//
// Not thread safe: use it from one thread (for example a single-thread executor).
//
#include <jni.h>
#include <android/log.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <HalSMExecutable.h>

#define LOG_TAG "HalSM"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define HALSM_JNI(ret,name) JNIEXPORT ret JNICALL Java_halwarsing_net_halchatandroid_halsm_HalSMNative_##name
#define HALSM_CLASS "halwarsing/net/halchatandroid/halsm/HalSMNative"

#define PTR(type,v) ((type*)(intptr_t)(v))
#define JPTR(p) ((jlong)(intptr_t)(p))

Dict nulldic={0,(void*)0};

HalSMMemory* memory;
HalSMCompiler* mainHSMC;
HalSMCompiler** hsmcs;
HalFileSystem** hfsc;
HalSMExecutable** hsmec;
int hsmcCount=0;
HalSMMemoryManagmentLibrary* hmml;
HalSMSystemLibrary* hsysl;
HalSMFileSystemLibrary* hfsl;

HalSMNull hnulld;
HalSMVariable nulld={&hnulld,HalSMVariableType_HalSMNull};
Dict* modulesHSME;
DictElement nullde={&nulld,&nulld};

unsigned char htrue=1;
unsigned char hfalse=0;

static int initialized=0;

static const char* HalSMVariableTypeNames[]={
    "int",
    "double",
    "char",
    "void",
    "HalSMArray",
    "str",
    "int_array",
    "HalSMFunctionC",
    "HalSMClassC",
    "HalSMRunClassC",
    "HalSMSetArg",
    "HalSMError",
    "HalSMNull",
    "HalSMRunFunc",
    "HalSMRunFuncC",
    "HalSMLocalFunction",
    "HalSMCModule",
    "HalSMModule",
    "HalSMCompiler",
    "HalSMRunClass",
    "HalSMDoubleGet",
    "HalSMClass",
    "HalSMVar",
    "HalSMMult",
    "HalSMDivide",
    "HalSMPlus",
    "HalSMMinus",
    "HalSMEqual",
    "HalSMNotEqual",
    "HalSMMore",
    "HalSMLess",
    "HalSMBool",
    "HalSMDict",
    "HalSMSetVar",
    "HalSMReturn",
    "HalSMFunctionCTypeDef",
    "HalSMFunctionArray",
    "unsigned_int",
    "custom",
    "HalSMMoreEqual",
    "HalSMLessEqual",
    "HalSMShift",
    "HalSMPow",
    "HalSMModulo",
    "HalSMAND",
    "HalSMOR",
    "HalSMNegate",
    "HalSMNegativeSign",
    //0.1.5
    "HalSMGOTO",
    "HalSMGOTOSector",
    "HalSMInlineIf",
    "HalSMInlineElse",
    "HalSMBreak",
    "HalSMXOR",
    "HalSMBAND",
    "HalSMBOR",
    "HalSMBNOT",
    //0.1.8
    "fv",
    "HalSMOP"};

_Static_assert(sizeof(HalSMVariableTypeNames)/sizeof(HalSMVariableTypeNames[0])==HalSMVariableType_HalSMOP+1,
    "HalSMVariableTypeNames is out of sync with HalSMVariableType");

typedef struct FilePointer {
    unsigned long long int index;
    unsigned long long int size;
    unsigned char* p;
} FilePointer;

//JNI helpers

static JavaVM* g_vm;
static jobject g_host;
static jmethodID g_hostPrint;
static jmethodID g_hostPrintError;
static jmethodID g_hostInput;
static jmethodID g_functionCCall;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm,void* reserved) {
    g_vm=vm;
    return JNI_VERSION_1_6;
}

//Callbacks can be called from any thread that runs HalSM code, so attach if needed.
static JNIEnv* attachEnv(int* attached) {
    JNIEnv* env=NULL;
    *attached=0;
    if ((*g_vm)->GetEnv(g_vm,(void**)&env,JNI_VERSION_1_6)==JNI_OK){return env;}
    if ((*g_vm)->AttachCurrentThread(g_vm,&env,NULL)==JNI_OK){*attached=1;return env;}
    return NULL;
}

static void detachEnv(int attached) {
    if (attached){(*g_vm)->DetachCurrentThread(g_vm);}
}

static void clearException(JNIEnv* env) {
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }
}

//GetStringUTFChars returns modified UTF-8 (emoji become surrogate pairs of 3 bytes), so encode by hand.
static char* jstringToUtf8(JNIEnv* env,jstring str) {
    if (str==NULL){return calloc(1,1);}
    jsize length=(*env)->GetStringLength(env,str);
    const jchar* chars=(*env)->GetStringChars(env,str,NULL);
    char* out=malloc((size_t)length*3+1);
    char* p=out;
    jsize i;
    for (i=0;i<length;i++) {
        unsigned int c=chars[i];
        if (c>=0xD800&&c<=0xDBFF&&i+1<length&&chars[i+1]>=0xDC00&&chars[i+1]<=0xDFFF) {
            c=0x10000+((c-0xD800)<<10)+(chars[i+1]-0xDC00);
            i++;
        }
        if (c<0x80) {
            *p++=(char)c;
        } else if (c<0x800) {
            *p++=(char)(0xC0|(c>>6));
            *p++=(char)(0x80|(c&0x3F));
        } else if (c<0x10000) {
            *p++=(char)(0xE0|(c>>12));
            *p++=(char)(0x80|((c>>6)&0x3F));
            *p++=(char)(0x80|(c&0x3F));
        } else {
            *p++=(char)(0xF0|(c>>18));
            *p++=(char)(0x80|((c>>12)&0x3F));
            *p++=(char)(0x80|((c>>6)&0x3F));
            *p++=(char)(0x80|(c&0x3F));
        }
    }
    *p='\0';
    (*env)->ReleaseStringChars(env,str,chars);
    return out;
}

static jstring utf8ToJstring(JNIEnv* env,const char* utf8) {
    size_t length=strlen(utf8);
    jchar* out=malloc((length+1)*sizeof(jchar));
    const unsigned char* b=(const unsigned char*)utf8;
    size_t n=0,i=0;
    while (i<length) {
        unsigned int c;
        size_t len;
        if (b[i]<0x80){c=b[i];len=1;}
        else if ((b[i]&0xE0)==0xC0&&i+1<length){c=((b[i]&0x1F)<<6)|(b[i+1]&0x3F);len=2;}
        else if ((b[i]&0xF0)==0xE0&&i+2<length){c=((b[i]&0x0F)<<12)|((b[i+1]&0x3F)<<6)|(b[i+2]&0x3F);len=3;}
        else if ((b[i]&0xF8)==0xF0&&i+3<length){c=((b[i]&0x07)<<18)|((b[i+1]&0x3F)<<12)|((b[i+2]&0x3F)<<6)|(b[i+3]&0x3F);len=4;}
        else {c=0xFFFD;len=1;}
        if (c>=0x10000) {
            c-=0x10000;
            out[n++]=(jchar)(0xD800+(c>>10));
            out[n++]=(jchar)(0xDC00+(c&0x3FF));
        } else {
            out[n++]=(jchar)c;
        }
        i+=len;
    }
    jstring res=(*env)->NewString(env,out,(jsize)n);
    free(out);
    return res;
}

//HalStringFormatChar* -> jstring
static jstring hsfToJstring(JNIEnv* env,HalStringFormatChar* str) {
    char* utf8=HU8(str);
    jstring res=utf8ToJstring(env,utf8);
    free(utf8);
    return res;
}

//jstring -> HalStringFormatChar* (new buffer, the caller decides who owns it)
static HalStringFormatChar* jstringToHsf(JNIEnv* env,jstring str) {
    char* utf8=jstringToUtf8(env,str);
    HalStringFormatChar* res=U8H(utf8);
    free(utf8);
    return res;
}

void* HalStringFormat_malloc(unsigned long long int size){return malloc(size);}

//HALSM

//HalSM Memory Library

void* HML_malloc(unsigned long long int size) {
    return malloc(size);
}

void* HML_calloc(unsigned long long int n,unsigned long long int size) {
    return calloc(n,size);
}

void* HML_realloc(void* p,unsigned long long int nsize) {
    return realloc(p,nsize);
}

void HML_free(void* p) {free(p);}

//HalSM Memory Library

//HalSM System Library

void HSYSL_exit(int code){LOGI("exit %d",code);}

//HalSM System Library

//Host (print, printError, input). It is set in init(), like Module.print/Module.input_text in JS

static void hostText(jmethodID method,int priority,HalStringFormatChar* str) {
    int attached;
    JNIEnv* env;
    if (g_host==NULL||(env=attachEnv(&attached))==NULL) {
        char* s=HU8(str);
        __android_log_print(priority,LOG_TAG,"%s",s);
        free(s);
        return;
    }
    jstring text=hsfToJstring(env,str);
    (*env)->CallVoidMethod(env,g_host,method,text);
    clearException(env);
    (*env)->DeleteLocalRef(env,text);
    detachEnv(attached);
}

void HSM_print(HalStringFormatChar* str){hostText(g_hostPrint,ANDROID_LOG_INFO,str);}

void HSM_printError(HalStringFormatChar* str){hostText(g_hostPrintError,ANDROID_LOG_ERROR,str);}

HalStringFormatChar* HSM_input(HalStringFormatChar* str) {
    int attached;
    JNIEnv* env;
    if (g_host==NULL||(env=attachEnv(&attached))==NULL){return U8H("");}
    jstring prompt=hsfToJstring(env,str);
    jstring answer=(jstring)(*env)->CallObjectMethod(env,g_host,g_hostInput,prompt);
    clearException(env);
    HalStringFormatChar* out=jstringToHsf(env,answer);
    (*env)->DeleteLocalRef(env,prompt);
    if (answer!=NULL){(*env)->DeleteLocalRef(env,answer);}
    detachEnv(attached);
    return out;
}

HalSMVariable* HSM_getVariable(HalStringFormatChar* name){return &nulld;}

//HalSM File System Library
//JNI: like in the WASM build the plugins have no access to the real file system.

void* HFSL_openFile(HalStringFormatChar* path,HalStringFormatChar* mode){return NULL;}

void HFSL_closeFile(void* p){}

unsigned long long int HFSL_read(void* out,unsigned long long int size,unsigned long long int count,void* p){return 0;}

unsigned long long int HFSL_write(void* out,unsigned long long int size,unsigned long long int count,void* p){return 0;}

void HFSL_puts(void* p,HalStringFormatChar* str){}

void HFSL_seek(void* p,unsigned long long int offset,unsigned long long int origin){}

unsigned long long int HFSL_tell(void* p){return 0;}

void HFSL_rewind(void* p){}

void HFSL_flush(void* p){}

int HFSL_eof(void* p){return -1;}

unsigned char HFSL_isExistsDir(HalStringFormatChar* path){return 0;}

unsigned char HFSL_isExistsFile(HalStringFormatChar* path){return 0;}

HalSMArray* HFSL_getListFiles(HalSMCompiler* hsmc,HalStringFormatChar* path){return HalSMArray_init(hsmc->memory);}

//HalFileSystem over a memory buffer (the HalSMExecutable file)

unsigned long long int HFS_read(void* out,unsigned long long int size,void* p) {
    FilePointer* fp=(FilePointer*)p;
    memory_cpy(out,fp->p+fp->index,size);
    fp->index+=size;
    return size;
}

unsigned long long int HFS_write(void* out,unsigned long long int size,void* p){return 0;}

void HFS_seek(void* p,unsigned long long int s,unsigned long long int w) {
    FilePointer* fp=(FilePointer*)p;
    if (w==SEEK_SET) {
        fp->index=s;
    } else if (w==SEEK_END) {
        fp->index=fp->size-s;
    }
}

unsigned long long int HFS_tell(void* p) {
    return ((FilePointer*)p)->index;
}

int HFS_eof(void* p) {
    return 0;
}

void HFS_flush(void* p){}

//Modules
//JNI: the WASM build can also load a "wasm" platform module from the HalSMExecutable. Here only the modules
//registered with addModule/addModuleByModule (for example the host module "HalChat") can be imported.

HalSMVariable* HSMLoadModule(HalSMCompiler* hsmc,HalStringFormatChar* name) {
    HalSMVariable* varName=HalSMVariable_init_str(memory,name);
    if (DictElementIndexByKey(hsmc,modulesHSME,varName)>-1){
        return DictElementFindByKey(hsmc,modulesHSME,varName)->value;
    }
    return &nulld;
}

//Compilers (plugins)

//Creates a compiler with its own file system and executable. Returns its index.
static int addCompiler(HalStringFormatChar* pluginId) {
    hsmcs=realloc(hsmcs,sizeof(HalSMCompiler*)*hsmcCount+sizeof(HalSMCompiler*));
    hfsc=realloc(hfsc,sizeof(HalFileSystem*)*hsmcCount+sizeof(HalFileSystem*));
    hsmec=realloc(hsmec,sizeof(HalSMExecutable*)*hsmcCount+sizeof(HalSMExecutable*));
    hfsc[hsmcCount]=malloc(sizeof(HalFileSystem));
    hsmec[hsmcCount]=malloc(sizeof(HalSMExecutable));
    HalFileSystem_init(hfsc[hsmcCount],HFS_read,HFS_write,HFS_seek,HFS_tell,HFS_eof,HFS_flush,HML_malloc,HML_calloc,HML_realloc,HML_free);
    hsmcs[hsmcCount]=HalSMCompiler_init(pluginId,HSM_print,HSM_printError,HSM_input,HSM_getVariable,memory,hsysl,hfsl,HSMLoadModule);
    return hsmcCount++;
}

//Takes the ownership of buffer. Returns the index of the compiler or -1-errorCode.
static int loadExecutable(unsigned char* buffer,unsigned long long int size,HalStringFormatChar* pluginId) {
    int index=addCompiler(pluginId);
    FilePointer* fp=(FilePointer*)malloc(sizeof(FilePointer));
    fp->index=0;
    fp->p=buffer;
    fp->size=size;
    unsigned int ec=HalFileSystem_open(hfsc[index],fp,0);
    if (ec>0) {
        hfsc[index]->disk=NOTHALSMNULLPOINTER;
        LOGE("load HalSMExecutable unsuccessfully: it is not a HalFileSystem (%u)",ec);
        return -1-(int)ec;
    }
    HalSMExecutable_init(hsmec[index],hfsc[index]);
    ec=HalSMExecutable_load(hsmec[index],hsmcs[index]);
    if (ec>0) {
        LOGE("load HalSMExecutable unsuccessfully: %u",ec);
        return -1-(int)ec;
    }
    return index;
}

//load(path,plugin_id): JNI: the file is read right here, so it is not asynchronous.
//Returns the index of the compiler or a negative number (-1-errorCode).
HALSM_JNI(jint,load)(JNIEnv* env,jclass cls,jstring path,jstring pluginId) {
    char* p=jstringToUtf8(env,path);
    FILE* f=fopen(p,"rb");
    free(p);
    if (f==NULL){LOGE("load file unsuccessfully");return -1;}
    fseek(f,0,SEEK_END);
    long size=ftell(f);
    fseek(f,0,SEEK_SET);
    if (size<0){fclose(f);return -1;}
    unsigned char* buffer=(unsigned char*)malloc((size_t)size+1);
    size_t read=fread(buffer,1,(size_t)size,f);
    fclose(f);
    return loadExecutable(buffer,read,jstringToHsf(env,pluginId));
}

HALSM_JNI(jstring,getPluginId)(JNIEnv* env,jclass cls,jlong hsmc) {
    return hsfToJstring(env,PTR(HalSMCompiler,hsmc)->path);
}

HALSM_JNI(void,run)(JNIEnv* env,jclass cls,jint index) {
    if (index>=hsmcCount||index<0){return;}
    HalSMExecutable_run(hsmec[index],hsmcs[index]);
}

HALSM_JNI(jlong,getHalSMCompiler)(JNIEnv* env,jclass cls,jint index) {
    if (index>=hsmcCount||index<0){return 0;}
    return JPTR(hsmcs[index]);
}

HALSM_JNI(jint,initHSMCCode)(JNIEnv* env,jclass cls) {
    return addCompiler(U8H(""));
}

HALSM_JNI(void,runCode)(JNIEnv* env,jclass cls,jint index,jstring code,jboolean isConsole) {
    HalSMCompiler_compile(hsmcs[index],jstringToHsf(env,code),isConsole,&nulld);
}

//HalSMCModule

HALSM_JNI(jlong,initModule)(JNIEnv* env,jclass cls,jstring name) {
    return JPTR(HalSMCModule_init(memory,jstringToHsf(env,name)));
}

HALSM_JNI(void,addClassCToModule)(JNIEnv* env,jclass cls,jlong module,jlong classc) {
    HalSMClassC* c=PTR(HalSMClassC,classc);
    PutDictElementToDict(mainHSMC,PTR(HalSMCModule,module)->classes,DictElementInit(memory,HalSMVariable_init_str(memory,c->name),HalSMVariable_init(memory,c,HalSMVariableType_HalSMClassC)));
}

HALSM_JNI(void,addFunctionToModule)(JNIEnv* env,jclass cls,jlong module,jstring name,jlong funcc) {
    HalStringFormatChar* n=jstringToHsf(env,name);
    PutDictElementToDict(mainHSMC,PTR(HalSMCModule,module)->lfuncs,DictElementInit(memory,HalSMVariable_init_str(memory,n),HalSMVariable_init(memory,PTR(HalSMFunctionC,funcc),HalSMVariableType_HalSMFunctionC)));
    free(n);
}

HALSM_JNI(void,addVariableToModule)(JNIEnv* env,jclass cls,jlong module,jstring name,jlong var) {
    HalStringFormatChar* n=jstringToHsf(env,name);
    PutDictElementToDict(mainHSMC,PTR(HalSMCModule,module)->vrs,DictElementInit(memory,HalSMVariable_init_str(memory,n),PTR(HalSMVariable,var)));
    free(n);
}

HALSM_JNI(jlong,getVariableFromModule)(JNIEnv* env,jclass cls,jlong module,jlong key) {
    return JPTR(DictElementFindByKey(mainHSMC,PTR(HalSMCModule,module)->vrs,PTR(HalSMVariable,key))->value);
}

HALSM_JNI(jint,indexVariableFromModule)(JNIEnv* env,jclass cls,jlong module,jlong key) {
    return (jint)DictElementIndexByKey(mainHSMC,PTR(HalSMCModule,module)->vrs,PTR(HalSMVariable,key));
}

//HalSMRunClassC

HALSM_JNI(void,setVariableRunClassC)(JNIEnv* env,jclass cls,jlong runclassc,jlong key,jlong var) {
    PutDictElementToDict(mainHSMC,PTR(HalSMRunClassC,runclassc)->vrs,DictElementInit(memory,PTR(HalSMVariable,key),PTR(HalSMVariable,var)));
}

HALSM_JNI(jlong,getVariableRunClassC)(JNIEnv* env,jclass cls,jlong runclassc,jlong key) {
    return JPTR(DictElementFindByKey(mainHSMC,PTR(HalSMRunClassC,runclassc)->vrs,PTR(HalSMVariable,key))->value);
}

HALSM_JNI(jint,indexVariableRunClassC)(JNIEnv* env,jclass cls,jlong runclassc,jlong key) {
    return (jint)DictElementIndexByKey(mainHSMC,PTR(HalSMRunClassC,runclassc)->vrs,PTR(HalSMVariable,key));
}

//HalSMClassC

HALSM_JNI(jlong,initClassC)(JNIEnv* env,jclass cls,jstring name) {
    return JPTR(HalSMClassC_init(memory,jstringToHsf(env,name)));
}

HALSM_JNI(void,addFunctionToClassC)(JNIEnv* env,jclass cls,jlong classc,jstring name,jlong funcc) {
    HalStringFormatChar* n=jstringToHsf(env,name);
    PutDictElementToDict(mainHSMC,PTR(HalSMClassC,classc)->funcs,DictElementInit(memory,HalSMVariable_init_str(memory,n),HalSMVariable_init(memory,PTR(HalSMFunctionC,funcc),HalSMVariableType_HalSMFunctionC)));
    free(n);
}

HALSM_JNI(void,addVariableToClassC)(JNIEnv* env,jclass cls,jlong classc,jstring name,jlong var) {
    HalStringFormatChar* n=jstringToHsf(env,name);
    PutDictElementToDict(mainHSMC,PTR(HalSMClassC,classc)->vrs,DictElementInit(memory,HalSMVariable_init_str(memory,n),PTR(HalSMVariable,var)));
    free(n);
}

HALSM_JNI(void,addModuleByModule)(JNIEnv* env,jclass cls,jlong module) {
    HalSMCModule* m=PTR(HalSMCModule,module);
    PutDictElementToDict(mainHSMC,modulesHSME,DictElementInit(memory,HalSMVariable_init_str(memory,m->name),HalSMVariable_init(memory,m,HalSMVariableType_HalSMCModule)));
}

HALSM_JNI(jlong,getVariableFromClassC)(JNIEnv* env,jclass cls,jlong classc,jlong key) {
    return JPTR(DictElementFindByKey(mainHSMC,PTR(HalSMClassC,classc)->vrs,PTR(HalSMVariable,key))->value);
}

HALSM_JNI(jlong,getFunctionFromClassC)(JNIEnv* env,jclass cls,jlong classc,jlong key) {
    return JPTR(DictElementFindByKey(mainHSMC,PTR(HalSMClassC,classc)->funcs,PTR(HalSMVariable,key))->value);
}

HALSM_JNI(jint,indexVariableFromClassC)(JNIEnv* env,jclass cls,jlong classc,jlong key) {
    return (jint)DictElementIndexByKey(mainHSMC,PTR(HalSMClassC,classc)->vrs,PTR(HalSMVariable,key));
}

HALSM_JNI(jint,indexFunctionFromClassC)(JNIEnv* env,jclass cls,jlong classc,jlong key) {
    return (jint)DictElementIndexByKey(mainHSMC,PTR(HalSMClassC,classc)->funcs,PTR(HalSMVariable,key));
}

HALSM_JNI(void,addModule)(JNIEnv* env,jclass cls,jstring name,jlong funcs,jlong vars,jlong classes) {
    HalStringFormatChar* n=jstringToHsf(env,name);
    HalSMCModule* cmodule=HalSMCModule_init(memory,n);
    cmodule->lfuncs=PTR(Dict,funcs);
    cmodule->vrs=PTR(Dict,vars);
    cmodule->classes=PTR(Dict,classes);
    PutDictElementToDict(mainHSMC,modulesHSME,DictElementInit(memory,HalSMVariable_init_str(memory,n),HalSMVariable_init(memory,cmodule,HalSMVariableType_HalSMCModule)));
}

//Functions
//JNI: WASM gets a function pointer from Module.addFunction(func,"iiii"). Here initFunctionC gets a
//HalSMNative.FunctionC object: long call(long hsmc,long args,long vars), which returns HalSMVariable* (0 is null).

static HalSMVariable* functionCCallback(void* data,HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    int attached;
    JNIEnv* env=attachEnv(&attached);
    if (env==NULL){return &nulld;}
    jlong res=(*env)->CallLongMethod(env,(jobject)data,g_functionCCall,JPTR(hsmc),JPTR(args),JPTR(vars));
    if ((*env)->ExceptionCheck(env)) {
        LOGE("HalSMNative.FunctionC threw an exception");
        clearException(env);
        res=0;
    }
    detachEnv(attached);
    //JNI: Java returns 0 for null
    return res==0?&nulld:PTR(HalSMVariable,res);
}

//The global reference of the callback lives as long as the process (the same as functions added by Module.addFunction).
HALSM_JNI(jlong,initFunctionC)(JNIEnv* env,jclass cls,jobject callback) {
    if (callback==NULL){return 0;}
    return JPTR(HalSMFunctionC_initWithData((void*)(*env)->NewGlobalRef(env,callback),mainHSMC,functionCCallback));
}

HALSM_JNI(jlong,runFunctionC)(JNIEnv* env,jclass cls,jlong hsmc,jlong func,jlong args,jlong vars) {
    return JPTR(HalSMFunctionC_run(PTR(HalSMCompiler,hsmc),PTR(HalSMFunctionC,func),PTR(HalSMArray,args),PTR(Dict,vars)));
}

HALSM_JNI(jlong,runLocalFunction)(JNIEnv* env,jclass cls,jlong func,jlong args,jlong vars) {
    return JPTR(HalSMLocalFunction_run(PTR(HalSMLocalFunction,func),PTR(HalSMArray,args),PTR(Dict,vars)));
}

HALSM_JNI(jlong,getLocalFunction)(JNIEnv* env,jclass cls,jlong hsmc,jlong name) {
    HalSMCompiler* c=PTR(HalSMCompiler,hsmc);
    if (DictElementIndexByKey(c,c->localFunctions,PTR(HalSMVariable,name))==-1){return 0;}
    return JPTR(DictElementFindByKey(c,c->localFunctions,PTR(HalSMVariable,name))->value->value);
}

//Variables

HALSM_JNI(jlong,initVariable)(JNIEnv* env,jclass cls,jlong value,jint type) {
    return JPTR(HalSMVariable_init(memory,PTR(void,value),(HalSMVariableType)type));
}

HALSM_JNI(jlong,variableInit)(JNIEnv* env,jclass cls,jlong value,jint type) {
    return JPTR(HalSMVariable_init(memory,PTR(void,value),(HalSMVariableType)type));
}

HALSM_JNI(jint,getTypeVariable)(JNIEnv* env,jclass cls,jlong var) {
    return (jint)PTR(HalSMVariable,var)->type;
}

HALSM_JNI(jlong,getValueVariable)(JNIEnv* env,jclass cls,jlong var) {
    return JPTR(PTR(HalSMVariable,var)->value);
}

//HalSMArray

HALSM_JNI(jlong,getSizeHalSMArray)(JNIEnv* env,jclass cls,jlong arr) {
    return (jlong)PTR(HalSMArray,arr)->size;
}

HALSM_JNI(jlong,getVariableFromHalSMArray)(JNIEnv* env,jclass cls,jlong arr,jint index) {
    HalSMArray* a=PTR(HalSMArray,arr);
    long long int i=index;
    if (i<0){i=(long long int)a->size+i;}
    if (i<0||i>=(long long int)a->size){return JPTR(&nulld);}
    return JPTR(a->arr[i]);
}

HALSM_JNI(void,appendToHalSMArray)(JNIEnv* env,jclass cls,jlong arr,jlong value) {
    HalSMArray_add(memory,PTR(HalSMArray,arr),PTR(HalSMVariable,value));
}

HALSM_JNI(void,setToHalSMArray)(JNIEnv* env,jclass cls,jlong arr,jlong value,jlong index) {
    HalSMArray_set(PTR(HalSMArray,arr),PTR(HalSMVariable,value),(unsigned long long int)index);
}

HALSM_JNI(jint,indexFromHalSMArray)(JNIEnv* env,jclass cls,jlong arr,jlong value) {
    return (jint)HalSMArray_index(mainHSMC,PTR(HalSMArray,arr),PTR(HalSMVariable,value));
}

HALSM_JNI(jlong,initHalSMArray)(JNIEnv* env,jclass cls) {
    return JPTR(HalSMArray_init(memory));
}

//Dict

HALSM_JNI(jint,getSizeDict)(JNIEnv* env,jclass cls,jlong dict) {
    return (jint)PTR(Dict,dict)->size;
}

HALSM_JNI(jlong,getDictElementFromDict)(JNIEnv* env,jclass cls,jlong dict,jint index) {
    Dict* d=PTR(Dict,dict);
    long long int i=index;
    if (i<0){i=(long long int)d->size+i;}
    if (i<0||i>=(long long int)d->size){return JPTR(&nullde);}
    return JPTR(d->elements[i]);
}

//JNI: 0 if there is no such element
HALSM_JNI(jlong,getDictElementFromDictByKey)(JNIEnv* env,jclass cls,jlong dict,jlong key) {
    return JPTR(DictElementFindByKey(mainHSMC,PTR(Dict,dict),PTR(HalSMVariable,key)));
}

HALSM_JNI(jlong,getDictElementFromDictByValue)(JNIEnv* env,jclass cls,jlong dict,jlong value) {
    return JPTR(DictElementFindByValue(mainHSMC,PTR(Dict,dict),PTR(HalSMVariable,value)));
}

HALSM_JNI(jint,indexDictElementFromDictByKey)(JNIEnv* env,jclass cls,jlong dict,jlong key) {
    return (jint)DictElementIndexByKey(mainHSMC,PTR(Dict,dict),PTR(HalSMVariable,key));
}

HALSM_JNI(jint,indexDictElementFromDictByValue)(JNIEnv* env,jclass cls,jlong dict,jlong value) {
    return (jint)DictElementIndexByValue(mainHSMC,PTR(Dict,dict),PTR(HalSMVariable,value));
}

HALSM_JNI(void,putToDict)(JNIEnv* env,jclass cls,jlong dict,jlong elem) {
    PutDictElementToDict(mainHSMC,PTR(Dict,dict),PTR(DictElement,elem));
}

HALSM_JNI(jlong,initDict)(JNIEnv* env,jclass cls) {
    return JPTR(DictInit(memory));
}

HALSM_JNI(jlong,initDictElement)(JNIEnv* env,jclass cls,jlong key,jlong value) {
    return JPTR(DictElementInit(memory,PTR(HalSMVariable,key),PTR(HalSMVariable,value)));
}

HALSM_JNI(jlong,getValueFromDictElement)(JNIEnv* env,jclass cls,jlong elem) {
    return JPTR(PTR(DictElement,elem)->value);
}

HALSM_JNI(jlong,getKeyFromDictElement)(JNIEnv* env,jclass cls,jlong elem) {
    return JPTR(PTR(DictElement,elem)->key);
}

//Values (getXFromValue takes getValueVariable(var), xToValue makes the value for variableInit)

HALSM_JNI(jstring,getStringFromValue)(JNIEnv* env,jclass cls,jlong value) {
    return hsfToJstring(env,*PTR(HalStringFormatChar*,value));
}

HALSM_JNI(jstring,getIntFromValue)(JNIEnv* env,jclass cls,jlong value) {
    return hsfToJstring(env,HalSMInteger_toString(memory,PTR(HalSMInteger,value),0));
}

HALSM_JNI(jlong,getLongIntFromValue)(JNIEnv* env,jclass cls,jlong value) {
    return ((long long int)HalSMInteger_ToUnsignedLongLongInteger(PTR(HalSMInteger,value)))*(PTR(HalSMInteger,value)->negative==1?-1:1);
}

HALSM_JNI(jstring,getDoubleFromValue)(JNIEnv* env,jclass cls,jlong value) {
    return hsfToJstring(env,HalSMDouble_toString(memory,PTR(HalSMDouble,value),0));
}

//JNI: like in WASM it reads one C char (plugin.js uses it for HalSMBool)
HALSM_JNI(jint,getCharFromValue)(JNIEnv* env,jclass cls,jlong value) {
    return (jint)(*PTR(char,value));
}

HALSM_JNI(jint,getUnsignedIntFromValue)(JNIEnv* env,jclass cls,jlong value) {
    return (jint)(*PTR(unsigned int,value));
}

HALSM_JNI(jlong,getHalSMClassCFromValue)(JNIEnv* env,jclass cls,jlong value) {
    return value;
}

HALSM_JNI(jlong,stringToValue)(JNIEnv* env,jclass cls,jstring str) {
    return JPTR(jstringToHsf(env,str));
}

HALSM_JNI(jlong,stringToVar)(JNIEnv* env,jclass cls,jstring str) {
    HalStringFormatChar* s=jstringToHsf(env,str);
    HalSMVariable* var=HalSMVariable_init_str(memory,s);
    free(s);
    return JPTR(var);
}

HALSM_JNI(jlong,intToValue)(JNIEnv* env,jclass cls,jint n) {
    return JPTR(HalSMInteger_FromSignedInteger(memory,(signed int)n));
}

//JNI: addition, 64-bit integer (intToValue is only 32-bit, and ids of messages do not fit)
HALSM_JNI(jlong,longToValue)(JNIEnv* env,jclass cls,jlong n) {
    HalSMInteger* out=HalSMInteger_FromUnsignedLongLongInteger(memory,n<0?0ULL-(unsigned long long int)n:(unsigned long long int)n);
    if (n<0){out->negative=1;}
    return JPTR(out);
}

HALSM_JNI(jlong,floatToValue)(JNIEnv* env,jclass cls,jstring n) {
    return JPTR(HalSMDouble_parse(memory,jstringToHsf(env,n)));
}

HALSM_JNI(jlong,charToValue)(JNIEnv* env,jclass cls,jint n) {
    char* out=malloc(sizeof(char));
    *out=(char)n;
    return JPTR(out);
}

HALSM_JNI(jlong,unsignedIntToValue)(JNIEnv* env,jclass cls,jint n) {
    unsigned int* out=malloc(sizeof(unsigned int));
    *out=(unsigned int)n;
    return JPTR(out);
}

HALSM_JNI(jlong,getHalSMNull)(JNIEnv* env,jclass cls) {
    return JPTR(HalSMVariable_init(memory,&hnulld,HalSMVariableType_HalSMNull));
}

HALSM_JNI(jlong,getHalSMTrue)(JNIEnv* env,jclass cls) {
    return JPTR(HalSMVariable_init(memory,&htrue,HalSMVariableType_HalSMBool));
}

HALSM_JNI(jlong,getHalSMFalse)(JNIEnv* env,jclass cls) {
    return JPTR(HalSMVariable_init(memory,&hfalse,HalSMVariableType_HalSMBool));
}

//HalSMSetArg

HALSM_JNI(jlong,getNameHalSMSetArg)(JNIEnv* env,jclass cls,jlong setarg) {
    return JPTR(PTR(HalSMSetArg,setarg)->name);
}

HALSM_JNI(jlong,getValueHalSMSetArg)(JNIEnv* env,jclass cls,jlong setarg) {
    return JPTR(PTR(HalSMSetArg,setarg)->value);
}

//JNI: replaces addHalSMVariableTypeToList. The index in the array is the value of the type.
HALSM_JNI(jobjectArray,getVariableTypeNames)(JNIEnv* env,jclass cls) {
    jsize count=(jsize)(sizeof(HalSMVariableTypeNames)/sizeof(HalSMVariableTypeNames[0]));
    jclass stringClass=(*env)->FindClass(env,"java/lang/String");
    jobjectArray out=(*env)->NewObjectArray(env,count,stringClass,NULL);
    jsize i;
    for (i=0;i<count;i++) {
        jstring name=(*env)->NewStringUTF(env,HalSMVariableTypeNames[i]);
        (*env)->SetObjectArrayElement(env,out,i,name);
        (*env)->DeleteLocalRef(env,name);
    }
    (*env)->DeleteLocalRef(env,stringClass);
    return out;
}

//host is HalSMNative.Host (print, printError, input) or null. It can be called again to change the host.
HALSM_JNI(void,init)(JNIEnv* env,jclass cls,jobject host) {
    if (g_host!=NULL){(*env)->DeleteGlobalRef(env,g_host);g_host=NULL;}
    if (host!=NULL) {
        jclass hostClass=(*env)->FindClass(env,HALSM_CLASS "$Host");
        g_hostPrint=(*env)->GetMethodID(env,hostClass,"print","(Ljava/lang/String;)V");
        g_hostPrintError=(*env)->GetMethodID(env,hostClass,"printError","(Ljava/lang/String;)V");
        g_hostInput=(*env)->GetMethodID(env,hostClass,"input","(Ljava/lang/String;)Ljava/lang/String;");
        (*env)->DeleteLocalRef(env,hostClass);
        g_host=(*env)->NewGlobalRef(env,host);
    }
    if (initialized){return;}
    jclass functionCClass=(*env)->FindClass(env,HALSM_CLASS "$FunctionC");
    g_functionCCall=(*env)->GetMethodID(env,functionCClass,"call","(JJJ)J");
    (*env)->DeleteLocalRef(env,functionCClass);

    hmml=malloc(sizeof(HalSMMemoryManagmentLibrary));
    hsysl=malloc(sizeof(HalSMSystemLibrary));
    hfsl=malloc(sizeof(HalSMFileSystemLibrary));
    *hmml=(HalSMMemoryManagmentLibrary){HML_malloc,HML_calloc,HML_realloc,HML_free};
    *hsysl=(HalSMSystemLibrary){HSYSL_exit};
    *hfsl=(HalSMFileSystemLibrary){HFSL_isExistsDir,HFSL_isExistsFile,HFSL_getListFiles,HFSL_openFile,HFSL_closeFile,HFSL_read,HFSL_write,HFSL_puts,HFSL_seek,HFSL_tell,HFSL_rewind,HFSL_flush,HFSL_eof};

    memory=(HalSMMemory*)malloc(sizeof(HalSMMemory));
    HalSMMemory_init(memory,hmml);

    HalStringFormat_init(HalStringFormat_malloc);
    hsmcs=malloc(0);
    hfsc=malloc(0);
    hsmec=malloc(0);
    modulesHSME=DictInit(memory);
    mainHSMC=HalSMCompiler_init(U8H(""),HSM_print,HSM_printError,HSM_input,HSM_getVariable,memory,hsysl,hfsl,HSMLoadModule);
    initialized=1;
}
