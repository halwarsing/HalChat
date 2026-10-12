#include <HalSMExecutable.h>
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH 260
Dict nulldic={0,(void*)0};

HalSMMemory* memory;
HalSMCompiler* mainHSMC;
HalSMCompiler** hsmcs;
HalFileSystem** hfsc;
HalSMExecutable** hsmec;
int hsmcCount=0;
char* tempPathFile="/";
HalSMMemoryManagmentLibrary* hmml;
HalSMSystemLibrary* hsysl;
HalSMFileSystemLibrary* hfsl;

HalSMNull hnulld;
HalSMVariable nulld={&hnulld,HalSMVariableType_HalSMNull};
Dict* modulesHSME;
DictElement nullde={&nulld,&nulld};

unsigned char htrue=1;
unsigned char hfalse=0;

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
    "HalSMInlineIf", //if(a>b) 1 else 0
    "HalSMInlineElse",
    "HalSMBreak",
    "HalSMXOR",
    "HalSMBAND",
    "HalSMBOR",
    "HalSMBNOT"};

typedef struct FilePointer {
    unsigned long long int index;
    unsigned long long int size;
    unsigned char* p;
} FilePointer;

void* HalStringFormat_malloc(unsigned long long int size){return malloc(size);}

EM_JS(void,WASM_drawPixelArray,(unsigned int* pixels,int width,int height),{
    var palette=Module.ctx.getImageData(0,0,width,height);
    palette.data.set((new Uint8Array(wasmMemory.buffer,pixels,width*height*4)).reverse());
    Module.ctx.putImageData(palette,0,0);
});

void HSME_drawPixelArray(unsigned int* pixels,unsigned int width,unsigned int height) {
    WASM_drawPixelArray(pixels,width,height);
}

//HALSM

//HalSM Memory Library

EMSCRIPTEN_KEEPALIVE
void* HML_malloc(unsigned long long int size) {
    return malloc(size);
}

EMSCRIPTEN_KEEPALIVE
void* HML_calloc(unsigned long long int n,unsigned long long int size) {
    return calloc(n,size);
}

EMSCRIPTEN_KEEPALIVE
void* HML_realloc(void* p,unsigned long long int nsize) {
    return realloc(p,nsize);
}

void HML_free(void* p) {free(p);}

//HalSM Memory Library

//HalSM String Library

HalStringFormatChar* HSL_Decimal2Str(long long d)
{
    /*char* out=calloc(21,sizeof(char));
    sprintf(out,"%lld",d);
    return U8H(out);*/
    return U8H("0");
}

HalStringFormatChar* HSL_Decimal2HexStr(long long d)
{
    /*char* out=calloc(21,sizeof(char));
    sprintf(out,"%x",d);
    return U8H(out);*/
    return U8H("0");
}

HalStringFormatChar* HSL_Double2Str(double d)
{
    /*char* out=calloc(256,sizeof(char));
    sprintf(out,"%d",d);
    return U8H(out);*/
    return U8H("0.0");
}

long long HSL_ParseDecimal(HalStringFormatChar* str)
{
    //return strtoll(HSF2ASCII(str),NULL,10);
    return 0;
}

double HSL_ParseDouble(HalStringFormatChar* str)
{
    //return strtod(HSF2ASCII(str),NULL);
    return 0.0;
}

//HalSM String Library

//HalSM Load Shared Library

void* HLSL_loadLibrary(HalStringFormatChar* path){return NULL;}

void* HLSL_getAddressByName(void* library,HalStringFormatChar* name){return NULL;}

void HLSL_closeLibrary(void* library){return;}

//HalSM Load Shared Library

//HalSM System Library

void HSYSL_exit(int code){printf("exit\n");}

//HalSM System Library

void HSM_print(HalStringFormatChar* str){
    char* s=HU8(str);
    printf("%s\n",s);
}

void HSM_printError(HalStringFormatChar* str){char* s=HU8(str);printf("%s\n",s);}

EM_ASYNC_JS(char*,async_input_text,(char* str,int strLength),{
    var itext=new TextEncoder().encode(await Module.input_text(new TextDecoder().decode(new Uint8Array(wasmMemory.buffer,str,strLength))));
    var out=new Uint8Array(wasmMemory.buffer,Module._calloc(itext.length+1,1),itext.length+1);
    out.set(itext);
    return out.byteOffset;
});

HalStringFormatChar* HSM_input(HalStringFormatChar* str) {
    char* h=HU8(str);
    return U8H(async_input_text(h,strlen(h)));
    //return U8H("");
}

HalSMVariable* HSM_getVariable(HalStringFormatChar* name){return &nulld;}

//HalSM File System Library

void* HFSL_openFile(HalStringFormatChar* path,HalStringFormatChar* mode){
    //return fopen(HU8(path),HU8(mode));
    return NULL;
}

void HFSL_closeFile(void* p){
    //fclose(p);
}

unsigned long long int HFSL_read(void* out,unsigned long long int size,unsigned long long int count,void* p){
    //return fread(out,size,count,p);
    return 0;
}

unsigned long long int HFSL_write(void* out,unsigned long long int size,unsigned long long int count,void* p){return 0;}

void HFSL_puts(void* p,HalStringFormatChar* str){}

void HFSL_seek(void* p,unsigned long long int offset,unsigned long long int origin){
    //fseek(p,offset,origin);
}

unsigned long long int HFSL_tell(void* p){
    //return ftell(p);
    return 0;
}

void HFSL_rewind(void* p){}

void HFSL_flush(void* p){}

int HFSL_eof(void* p){
    //return feof(p);
    return -1;
}

unsigned char HFSL_isExistsDir(HalStringFormatChar* path){
    return 0;
}

unsigned char HFSL_isExistsFile(HalStringFormatChar* path){
    return 0;
}

HalSMArray* HFSL_getListFiles(HalSMCompiler* hsmc,HalStringFormatChar* path){return HalSMArray_init(hsmc->memory);}

HalStringFormatChar* getNameFile(HalStringFormatChar* path) {
    HalStringFormatChar* temp=malloc(0);
    HalStringFormatChar t;
    unsigned long long int i,l,size;
    l=HalStringFormat_length(path);
    size=0;
    HalStringFormatChar* p=path;
    for (i=0;i<l;i++,p++) {
        t=*p;
        if (HSFCharCompare(t,ASCIIH('/'))||HSFCharCompare(t,ASCIIH('\\'))) {
            size=0;
            free(temp);
            temp=malloc(0);
        } else {
            temp=realloc(temp,size*sizeof(HalStringFormatChar)+sizeof(HalStringFormatChar));
            temp[size++]=t;
        }
    }
    temp=realloc(temp,size*sizeof(HalStringFormatChar)+sizeof(HalStringFormatChar));
    temp[size]=ASCIIH('\0');
    return temp;
}

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

int getArgsSize(HalSMArray* args) {
    return args->size;
}

EM_ASYNC_JS(HalSMCModule_entry*,loadWASM,(void* p,int size,char* nameFile,int nameFileLength,char* nameFunc,int lengthNameFunc,HalSMCompiler* hsmc,HalSMArray* args),{
    var nameFileStr=new TextDecoder().decode(new Uint8Array(wasmMemory.buffer,nameFile,nameFileLength));
    var tfile=new File([new Uint8Array(wasmMemory.buffer,p,size)],nameFileStr,{type:"text/wasm"});
    var nameFuncStr=new TextDecoder().decode(new Uint8Array(wasmMemory.buffer,nameFunc,lengthNameFunc));
    var ourl=URL.createObjectURL(tfile);
    return await loadDynamicLibraryHSME(new Uint8Array(wasmMemory.buffer,p,size),nameFileStr,{loadAsync:true,global:true,nodelete:true}).then((d)=>{
        return LDSO.loadedLibsByName[nameFileStr]["exports"][nameFuncStr](hsmc,args);
    });
});

EM_JS(void*,load_file,(char* urlStr,int urlLength),{
    var url=new TextDecoder().decode(new Uint8Array(wasmMemory.buffer,urlStr,urlLength));
    /*var xhr = new XMLHttpRequest();
    xhr.onload = function()
    {
        if (xhr.status == 200)
        {
            //var b = malloc_array(new Uint8Array(xhr.response));
            //WAFNHTTP(200, b, xhr.response.byteLength, userdata);
            //WA.asm.free(b);
            var u=new Uint8Array(xhr.response);
            var b=new Uint8Array(wasmMemory.buffer,Module._HML_malloc(u.length),u.length);
            b.set(u);
            Module._load_file_event_success(b.byteOffset,u.length);
        } else {
            Module._load_file_event_error();
        }
    };
    xhr.responseType = 'arraybuffer';
    xhr.open('GET', url, true);

    xhr.send();*/
    /*if(url.startsWith("https://haldrive.halwarsing.net/file/")) {
        url = new URL(url);
        url.searchParams.set("isJson", "1");
        url=url.toString();
        fetch(url,{
            method: "GET",
            mode: "cors",
            credentials: "include"
        }).then((response)=>response.json()).then((data)=>{
            if(data['errorCode']===0) {
                fetch(data['url'], {
                    method: "GET",
                    mode: "cors"
                }).then(response => response.arrayBuffer()).then(buf => {
                    var u = new Uint8Array(buf);
                    var b = new Uint8Array(wasmMemory.buffer, Module._HML_malloc(u.length), u.length);
                    b.set(u);
                    Module._load_file_event_success(b.byteOffset, u.length);
                });
            } else {
                console.log("Load file error: "+data['errorCode']+":"+data['error']);
                Module._load_file_event_error();
            }
        });
    } else {*/
    fetch(url,{
        method:"GET",
        mode:"cors",
        credentials:"include"
    }).then((response)=>response.arrayBuffer()).then((buf)=>{
        var u=new Uint8Array(buf);
        var b=new Uint8Array(wasmMemory.buffer,Module._HML_malloc(u.length),u.length);
        b.set(u);
        Module._load_file_event_success(b.byteOffset,u.length);
    });
});

unsigned char GetHalSMCompilerIndex(HalSMCompiler* hsmc,int* out) {
    int i;
    HalSMCompiler** p=hsmcs;
    for (i=0;i<hsmcCount;i++) {
        if (*p++==hsmc) {
            *out=i;
            return 0;
        }
    }
    return 1;
}

HalSMVariable* HSMLoadModule(HalSMCompiler* hsmc,HalStringFormatChar* name) {
    HalStringFormatChar* modulePath=name,*moduleDirPath,*dataStr;
    unsigned char isModule=0,*data;
    unsigned long long int size,i,l,indexFile;
    int indexHSMC;
    HalSMVariable* res,*varName;
    HalFileSystemFile file;
    HalFileSystem* hfs;
    if (hsmcCount==0){return &nulld;}
    if (GetHalSMCompilerIndex(hsmc,&indexHSMC)>0){indexHSMC=0;}
    hfs=hfsc[indexHSMC];
    varName=HalSMVariable_init_str(memory,name);
    if (DictElementIndexByKey(hsmc,modulesHSME,varName)>-1){
        return DictElementFindByKey(hsmc,modulesHSME,varName)->value;
    }
    if (HSFCharCompare(modulePath[1],ASCIIH(':'))==0) {
        if (StringIndexOf(hsmc->memory,modulePath,U8H("/"))==-1&&(tempPathFile!=NULL)) {
            moduleDirPath=ConcatenateStrings(hsmc->memory,modulePath,U8H("/"));
            modulePath=ConcatenateStrings(hsmc->memory,moduleDirPath,U8H("module.hcfg"));
            isModule=1;
        } else {
            modulePath=ConcatenateStrings(hsmc->memory,modulePath,U8H(".hsm"));
        }
    } else {
        modulePath=ConcatenateStrings(hsmc->memory,modulePath,U8H(".hsm"));
    }
    if (isModule==1) {
        if (HalFileSystem_getIndexFileByPath(hfs,&indexFile,modulePath,1)>0||HalFileSystem_getFileByIndex(hfs,indexFile,&file)>0){return &nulld;}
        size=file.size;
        data=calloc(size+1,sizeof(char));
        HalFileSystem_readFileByIndex(hfs,indexFile,data,1,size);
        dataStr=U8H((char*)data);
        free(data);
        if (HSFCharCompare(dataStr[0],ASCIIH('{'))==0||HSFCharCompare(dataStr[HalStringFormat_length(dataStr)-1],ASCIIH('}'))==0||HalStringFormat_length(dataStr)<3) {
            free(dataStr);
            return &nulld;
        }
        string_cpy(dataStr,&dataStr[1]);
        dataStr[HalStringFormat_length(dataStr)-1]=ASCIIH('\0');
        Dict* config=HalSMCompiler_getArgsDict(hsmc,dataStr,&nulldic);
        free(dataStr);
        HalSMVariable* tempKey=HalSMVariable_init_str(hsmc->memory,U8H("platforms"));
        if (DictElementIndexByKey(hsmc,config,tempKey)==-1){return &nulld;}
        HalSMVariable* tempValue=DictElementFindByKey(hsmc,config,tempKey)->value;
        if (tempValue->type!=HalSMVariableType_HalSMDict){return &nulld;}
        Dict* platforms=(Dict*)tempValue->value;
        tempKey=HalSMVariable_init_str(hsmc->memory,U8H("wasm"));
        if (DictElementIndexByKey(hsmc,platforms,tempKey)==-1){return &nulld;}
        tempValue=DictElementFindByKey(hsmc,platforms,tempKey)->value;
        if (tempValue->type!=HalSMVariableType_HalSMDict){return &nulld;}
        Dict* platform=(Dict*)tempValue->value;
        tempKey=HalSMVariable_init_str(hsmc->memory,U8H("main"));
        if (DictElementIndexByKey(hsmc,platform,tempKey)==-1){return &nulld;}
        tempValue=DictElementFindByKey(hsmc,platform,tempKey)->value;
        if (tempValue->type!=HalSMVariableType_str){return &nulld;}
        HalStringFormatChar* platformMain=*(HalStringFormatChar**)tempValue->value;
        if (string_len(platformMain)==0){return &nulld;}
        tempKey=HalSMVariable_init_str(hsmc->memory,U8H("module"));
        if (DictElementIndexByKey(hsmc,platform,tempKey)==-1){return &nulld;}
        tempValue=DictElementFindByKey(hsmc,platform,tempKey)->value;
        if (tempValue->type!=HalSMVariableType_str){return &nulld;}
        HalStringFormatChar* platformModulePath=*(HalStringFormatChar**)tempValue->value;
        if (string_len(platformModulePath)==0){return &nulld;}
        platformModulePath=ConcatenateStrings(hsmc->memory,moduleDirPath,platformModulePath);
        if (HalFileSystem_getIndexFileByPath(hfs,&indexFile,platformModulePath,1)>0||HalFileSystem_getFileByIndex(hfs,indexFile,&file)){return &nulld;}
        size=file.size;
        data=malloc(size);
        HalFileSystem_readFileByIndex(hfs,indexFile,data,1,size);
        char* tempdata=HU8(platformMain);
        HalSMArray* argsFunc=HalSMArray_init(hsmc->memory);
        HalSMArray_add(hsmc->memory,argsFunc,HalSMVariable_init(hsmc->memory,HSME_drawPixelArray,HalSMVariableType_void));
        char* nameFile=HU8(getNameFile(platformModulePath));
        return HalSMVariable_init(hsmc->memory,HalSMCModule_load(hsmc,loadWASM(data,size,nameFile,strlen(nameFile),tempdata,strlen(tempdata),hsmc,argsFunc)),HalSMVariableType_HalSMCModule);
    }
    return &nulld;
}

EM_JS(void,HSME_stopLoadModule,(),{
    Module.is_load_module_resolve(null);
});

EM_JS(void,HSME_runEventOnLoadHSMEFile,(int index),{
    Module.is_load_module_resolve(index);
    Module.onLoadHSMEFile(index);
});

EMSCRIPTEN_KEEPALIVE
void load_file_event_success(unsigned char* buffer,int bufferLength) {
    FilePointer* fp=(FilePointer*)malloc(sizeof(FilePointer));
    fp->index=0;
    fp->p=buffer;
    fp->size=bufferLength;
    HalFileSystem_open(hfsc[hsmcCount],fp,0);
    HalSMExecutable_init(hsmec[hsmcCount],hfsc[hsmcCount]);
    unsigned int ec=HalSMExecutable_load(hsmec[hsmcCount],hsmcs[hsmcCount]);
    if (ec>0){printf("Error: load HalSMExecutable unsuccessfully\n");}
    HSME_runEventOnLoadHSMEFile(hsmcCount++);
}

EMSCRIPTEN_KEEPALIVE
void load_file_event_error() {
    printf("Error: load file unsuccessfully\n");
    HSME_stopLoadModule();
}

EMSCRIPTEN_KEEPALIVE
int load(char* path,char* plugin_id) {
    char* pathName=HU8(getNameFile(U8H(path)));
    hsmcs=realloc(hsmcs,sizeof(HalSMCompiler*)*hsmcCount+sizeof(HalSMCompiler*));
    hfsc=realloc(hfsc,sizeof(HalFileSystem*)*hsmcCount+sizeof(HalFileSystem*));
    hsmec=realloc(hsmec,sizeof(HalSMExecutable*)*hsmcCount+sizeof(HalSMExecutable*));
    hfsc[hsmcCount]=malloc(sizeof(HalFileSystem));
    hsmec[hsmcCount]=malloc(sizeof(HalSMExecutable));
    HalFileSystem_init(hfsc[hsmcCount],HFS_read,HFS_write,HFS_seek,HFS_tell,HFS_eof,HFS_flush,HML_malloc,HML_calloc,HML_realloc,HML_free);
    hsmcs[hsmcCount]=HalSMCompiler_init(U8H(plugin_id),HSM_print,HSM_printError,HSM_input,HSM_getVariable,memory,hsysl,hfsl,HSMLoadModule);
    load_file(path,strlen(path));
    return hsmcCount;
}

EMSCRIPTEN_KEEPALIVE
char* getPluginId(HalSMCompiler* hsmc) {
    return HU8(hsmc->path);
}

EMSCRIPTEN_KEEPALIVE
void run(int index) {
    if (index>=hsmcCount||index<0){return;}
    HalSMExecutable_run(hsmec[index],hsmcs[index]);
}

EMSCRIPTEN_KEEPALIVE
HalSMCompiler* getHalSMCompiler(int index) {
    if(index>=hsmcCount||index<0){return NOTHALSMNULLPOINTER;}
    return hsmcs[index];
}

EMSCRIPTEN_KEEPALIVE
int initHSMCCode() {
    hsmcs=realloc(hsmcs,sizeof(HalSMCompiler*)*hsmcCount+sizeof(HalSMCompiler*));
    hfsc=realloc(hfsc,sizeof(HalFileSystem*)*hsmcCount+sizeof(HalFileSystem*));
    hsmec=realloc(hsmec,sizeof(HalSMExecutable*)*hsmcCount+sizeof(HalSMExecutable*));
    hfsc[hsmcCount]=malloc(sizeof(HalFileSystem));
    hsmec[hsmcCount]=malloc(sizeof(HalSMExecutable));
    HalFileSystem_init(hfsc[hsmcCount],HFS_read,HFS_write,HFS_seek,HFS_tell,HFS_eof,HFS_flush,HML_malloc,HML_calloc,HML_realloc,HML_free);
    hsmcs[hsmcCount]=HalSMCompiler_init(U8H(""),HSM_print,HSM_printError,HSM_input,HSM_getVariable,memory,hsysl,hfsl,HSMLoadModule);
    return hsmcCount++;
}

EMSCRIPTEN_KEEPALIVE
void runCode(int index,char* code,int isConsole) {
    HalSMCompiler_compile(hsmcs[index],U8H(code),isConsole,&nulld);
}

EMSCRIPTEN_KEEPALIVE
HalSMCModule* initModule(char* name) {
    return HalSMCModule_init(memory,U8H(name));
}

EMSCRIPTEN_KEEPALIVE
void addClassCToModule(HalSMCModule* module,HalSMClassC* classc) {
    PutDictElementToDict(mainHSMC,module->classes,DictElementInit(memory,HalSMVariable_init_str(memory,classc->name),HalSMVariable_init(memory,classc,HalSMVariableType_HalSMClassC)));
}

EMSCRIPTEN_KEEPALIVE
void addFunctionToModule(HalSMCModule* module,char* name,HalSMFunctionC* funcc) {
    PutDictElementToDict(mainHSMC,module->lfuncs,DictElementInit(memory,HalSMVariable_init_str(memory,U8H(name)),HalSMVariable_init(memory,funcc,HalSMVariableType_HalSMFunctionC)));
}

EMSCRIPTEN_KEEPALIVE
void addVariableToModule(HalSMCModule* module,char* name,HalSMVariable* var) {
    PutDictElementToDict(mainHSMC,module->vrs,DictElementInit(memory,HalSMVariable_init_str(memory,U8H(name)),var));
}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getVariableFromModule(HalSMCModule* module,HalSMVariable* key) {
    return DictElementFindByKey(mainHSMC,module->vrs,key)->value;
}

EMSCRIPTEN_KEEPALIVE
int indexVariableFromModule(HalSMCModule* module,HalSMVariable* key) {
    return DictElementIndexByKey(mainHSMC,module->vrs,key);
}

//HalSMRunClassC
EMSCRIPTEN_KEEPALIVE
void setVariableRunClassC(HalSMRunClassC* runclassc, HalSMVariable* key, HalSMVariable* var)
{
    PutDictElementToDict(mainHSMC,runclassc->vrs,DictElementInit(memory,key,var));
}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getVariableRunClassC(HalSMRunClassC* runclassc, HalSMVariable* key)
{
    return DictElementFindByKey(mainHSMC, runclassc->vrs, key)->value;
}

EMSCRIPTEN_KEEPALIVE
int indexVariableRunClassC(HalSMRunClassC* runclassc, HalSMVariable* key)
{
    return DictElementIndexByKey(mainHSMC,runclassc->vrs,key);
}

//HalSMClassC

EMSCRIPTEN_KEEPALIVE
HalSMClassC* initClassC(char* name) {
    return HalSMClassC_init(memory,U8H(name));
}

EMSCRIPTEN_KEEPALIVE
void addFunctionToClassC(HalSMClassC* classc,char* name,HalSMFunctionC* funcc) {
    PutDictElementToDict(mainHSMC,classc->funcs,DictElementInit(memory,HalSMVariable_init_str(memory,U8H(name)),HalSMVariable_init(memory,funcc,HalSMVariableType_HalSMFunctionC)));
}

EMSCRIPTEN_KEEPALIVE
void addVariableToClassC(HalSMClassC* classc,char* name,HalSMVariable* var) {
    PutDictElementToDict(mainHSMC,classc->vrs,DictElementInit(memory,HalSMVariable_init_str(memory,U8H(name)),var));
}

EMSCRIPTEN_KEEPALIVE
void addModuleByModule(HalSMCModule* module) {
    PutDictElementToDict(mainHSMC,modulesHSME,DictElementInit(memory,HalSMVariable_init_str(memory,module->name),HalSMVariable_init(memory,module,HalSMVariableType_HalSMCModule)));
}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getVariableFromClassC(HalSMClassC* classc,HalSMVariable* key) {
    return DictElementFindByKey(mainHSMC,classc->vrs,key)->value;
}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getFunctionFromClassC(HalSMClassC* classc,HalSMVariable* key) {
    return DictElementFindByKey(mainHSMC,classc->funcs,key)->value;
}

EMSCRIPTEN_KEEPALIVE
int indexVariableFromClassC(HalSMClassC* classc,HalSMVariable* key) {
    return DictElementIndexByKey(mainHSMC,classc->vrs,key);
}

EMSCRIPTEN_KEEPALIVE
int indexFunctionFromClassC(HalSMClassC* classc,HalSMVariable* key) {
    return DictElementIndexByKey(mainHSMC,classc->funcs,key);
}

EMSCRIPTEN_KEEPALIVE
HalSMVariableCustom* initVariableCustom(HalSMVariableTypeCustom* type,void* value,Dict* vars,Dict* funcs) {
    return HalSMVariableCustom_init(memory,type,value,vars,funcs);
}

EMSCRIPTEN_KEEPALIVE
HalSMVariableTypeCustom* initVariableTypeCustom(HalStringFormatChar* name,
    HalStringFormatChar*(*toString)(HalSMCompiler*,HalSMVariable*),
    HalSMVariable*(*add)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*sub)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*mul)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*div)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*pow)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*modulo)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*increment)(HalSMCompiler*,HalSMVariable*),
    HalSMVariable*(*decrement)(HalSMCompiler*,HalSMVariable*),
    unsigned char(*isEqual)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    unsigned char(*isMore)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    unsigned char(*isLess)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    unsigned char(*getBool)(HalSMCompiler*,HalSMVariable*),
    long long int(*indexVariableByName)(HalSMCompiler*,HalSMVariable*,HalStringFormatChar*),
    long long int(*indexFunctionByName)(HalSMCompiler*,HalSMVariable*,HalStringFormatChar*),
    HalSMVariable*(*getVariableByName)(HalSMCompiler*,HalSMVariable*,HalStringFormatChar*),
    HalSMVariable*(*getFunctionByName)(HalSMCompiler*,HalSMVariable*,HalStringFormatChar*),
    HalSMVariable*(*getItem)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*shiftRight)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*shiftLeft)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*convert)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*bxor)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*band)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*bor)(HalSMCompiler*,HalSMVariable*,HalSMVariable*),
    HalSMVariable*(*bnot)(HalSMCompiler*,HalSMVariable*),
    Dict* vars,Dict* funcs) {
    HalSMVariableTypeCustom type;
    type.name=name;
    type.toString=toString;
    type.add=add;
    type.sub=sub;
    type.mul=mul;
    type.div=div;
    type.pow=pow;
    type.modulo=modulo;
    type.increment=increment;
    type.decrement=decrement;
    type.isEqual=isEqual;
    type.isMore=isMore;
    type.isLess=isLess;
    type.getBool=getBool;
    type.indexVariableByName=indexVariableByName;
    type.indexFunctionByName=indexFunctionByName;
    type.getVariableByName=getVariableByName;
    type.getFunctionByName=getFunctionByName;
    type.getItem=getItem;
    type.shiftRight=shiftRight;
    type.shiftLeft=shiftLeft;
    type.convert=convert;
    type.bxor=bxor;
    type.band=band;
    type.bor=bor;
    type.bnot=bnot;
    type.vars=vars;
    type.funcs=funcs;
    return HalSMVariableTypeCustom_init(memory,type);
}

EMSCRIPTEN_KEEPALIVE
void addModule(char* name,Dict* funcs,Dict* vars,Dict* classes) {
    HalSMCModule* cmodule=HalSMCModule_init(memory,U8H(name));
    cmodule->lfuncs=funcs;
    cmodule->vrs=vars;
    cmodule->classes=classes;
    PutDictElementToDict(mainHSMC,modulesHSME,DictElementInit(memory,HalSMVariable_init_str(memory,U8H(name)),HalSMVariable_init(memory,cmodule,HalSMVariableType_HalSMCModule)));
}

EMSCRIPTEN_KEEPALIVE
HalSMFunctionC* initFunctionC(HalSMFunctionCTypeDef func) {
    return HalSMFunctionC_init(mainHSMC,func);
}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* runFunctionC(HalSMCompiler* hsmc,HalSMFunctionC* func,HalSMArray* args,Dict* vars) {
    return HalSMFunctionC_run(hsmc,func,args,vars);
}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* runLocalFunction(HalSMLocalFunction* func,HalSMArray* args,Dict* vars) {
    return HalSMLocalFunction_run(func,args,vars);
}



EMSCRIPTEN_KEEPALIVE
HalSMLocalFunction* getLocalFunction(HalSMCompiler* hsmc,HalSMVariable* name) {
    if(DictElementIndexByKey(hsmc,hsmc->localFunctions,name)==-1){return NOTHALSMNULLPOINTER;}
    return (HalSMLocalFunction*)DictElementFindByKey(hsmc,hsmc->localFunctions,name)->value->value;
}



EMSCRIPTEN_KEEPALIVE
HalSMVariable* initVariable(void* value,HalSMVariableType type) {
    return HalSMVariable_init(memory,value,type);
}

EMSCRIPTEN_KEEPALIVE
long long int getSizeHalSMArray(HalSMArray* arr){return arr->size;}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getVariableFromHalSMArray(HalSMArray* arr,int index) {
    if (index<0){index=arr->size+index;}
    if (index<0||index>=arr->size){return &nulld;}
    return arr->arr[index];
}

EMSCRIPTEN_KEEPALIVE
void appendToHalSMArray(HalSMArray* arr,HalSMVariable* value){HalSMArray_add(memory,arr,value);}

EMSCRIPTEN_KEEPALIVE
void setToHalSMArray(HalSMArray* arr,HalSMVariable* value,long long int index){
    HalSMArray_set(arr,value,index);
}

EMSCRIPTEN_KEEPALIVE
int indexFromHalSMArray(HalSMArray* arr,HalSMVariable* value){return HalSMArray_index(mainHSMC,arr,value);}

EMSCRIPTEN_KEEPALIVE
HalSMArray* initHalSMArray(){return HalSMArray_init(memory);}

EMSCRIPTEN_KEEPALIVE
int getSizeDict(Dict* dict){return dict->size;}

EMSCRIPTEN_KEEPALIVE
DictElement* getDictElementFromDict(Dict* dict,int index) {
    if (index<0){index=dict->size+index;}
    if (index<0||index>=dict->size){return &nullde;}
    return dict->elements[index];
}

EMSCRIPTEN_KEEPALIVE
DictElement* getDictElementFromDictByKey(Dict* dict,HalSMVariable* key){return DictElementFindByKey(mainHSMC,dict,key);}

EMSCRIPTEN_KEEPALIVE
DictElement* getDictElementFromDictByValue(Dict* dict,HalSMVariable* value){return DictElementFindByValue(mainHSMC,dict,value);}

EMSCRIPTEN_KEEPALIVE
int indexDictElementFromDictByKey(Dict* dict,HalSMVariable* key){return DictElementIndexByKey(mainHSMC,dict,key);}

EMSCRIPTEN_KEEPALIVE
int indexDictElementFromDictByValue(Dict* dict,HalSMVariable* value){return DictElementIndexByValue(mainHSMC,dict,value);}

EMSCRIPTEN_KEEPALIVE
void putToDict(Dict* dict,DictElement* elem){PutDictElementToDict(mainHSMC,dict,elem);}

EMSCRIPTEN_KEEPALIVE
Dict* initDict(){return DictInit(memory);}

EMSCRIPTEN_KEEPALIVE
DictElement* initDictElement(HalSMVariable* key,HalSMVariable* value){return DictElementInit(memory,key,value);}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getValueFromDictElement(DictElement* elem){return elem->value;}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getKeyFromDictElement(DictElement* elem){return elem->key;}

EMSCRIPTEN_KEEPALIVE
HalSMVariableType getTypeVariable(HalSMVariable* var){return var->type;}

EMSCRIPTEN_KEEPALIVE
void* getValueVariable(HalSMVariable* var){return var->value;}

EMSCRIPTEN_KEEPALIVE
char* getStringFromValue(void* value){return HU8(*(HalStringFormatChar**)value);}

EMSCRIPTEN_KEEPALIVE
char* getIntFromValue(void* value){return HU8(HalSMInteger_toString(memory,(HalSMInteger*)value,0));}

EMSCRIPTEN_KEEPALIVE
long long int getLongIntFromValue(void* value){return ((long long int)HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)value))*(((HalSMInteger*)value)->negative==1?-1:1);}

EMSCRIPTEN_KEEPALIVE
char* getDoubleFromValue(void* value){return HU8(HalSMDouble_toString(memory,(HalSMDouble*)value,0));}

EMSCRIPTEN_KEEPALIVE
int getCharFromValue(void* value){return (int)(*(char*)value);}

EMSCRIPTEN_KEEPALIVE
int getUnsignedIntFromValue(void* value){return (int)(*(unsigned int*)value);}

EMSCRIPTEN_KEEPALIVE
HalSMClassC* getHalSMClassCFromValue(void* value){return (HalSMClassC*)value;}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* variableInit(void* value,HalSMVariableType type){return HalSMVariable_init(memory,value,type);}

EMSCRIPTEN_KEEPALIVE
void* stringToValue(char* str){return U8H(str);}

EMSCRIPTEN_KEEPALIVE
void* stringToVar(char* str){return HalSMVariable_init_str(memory,U8H(str));}

EMSCRIPTEN_KEEPALIVE
void* intToValue(int n){return HalSMInteger_FromSignedInteger(memory,(signed int)n);}

EMSCRIPTEN_KEEPALIVE
void* floatToValue(char* n){
    return HalSMDouble_parse(memory,U8H(n));
}

EMSCRIPTEN_KEEPALIVE
void* charToValue(char n){char* out=malloc(sizeof(char));*out=n;return out;}

EMSCRIPTEN_KEEPALIVE
void* unsignedIntToValue(unsigned int n){unsigned int* out=malloc(sizeof(unsigned int));*out=n;return out;}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getHalSMNull(){return HalSMVariable_init(memory,&hnulld,HalSMVariableType_HalSMNull);}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getHalSMTrue(){return HalSMVariable_init(memory,&htrue,HalSMVariableType_HalSMBool);}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getHalSMFalse(){return HalSMVariable_init(memory,&hfalse,HalSMVariableType_HalSMBool);}

EMSCRIPTEN_KEEPALIVE
int StrGetLength(char* text) {
    return strlen(text);
}

EM_JS(void*,CharArrayToString,(char* text),{
    return new TextDecoder().decode(new Uint8Array(wasmMemory.buffer,text,Module._StrGetLength(text)));
});

EM_JS(void,addHalSMVariableTypeToList,(char* name,void* value),{
    Module['HalSM']['HalSMVariableType'][CharArrayToString(name)]=value;
});

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getNameHalSMSetArg(HalSMSetArg* setarg) {
    return setarg->name;
}

EMSCRIPTEN_KEEPALIVE
HalSMVariable* getValueHalSMSetArg(HalSMSetArg* setarg) {
    return setarg->value;
}

EMSCRIPTEN_KEEPALIVE
void init() {
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
    char** pointer=(char**)HalSMVariableTypeNames;
    for (HalSMVariableType ht=HalSMVariableType_int;ht<=HalSMVariableType_HalSMBNOT;ht++,pointer++) {
        addHalSMVariableTypeToList(*pointer,(void*)ht);
    }
}

int main(void) {
    return 0;
}