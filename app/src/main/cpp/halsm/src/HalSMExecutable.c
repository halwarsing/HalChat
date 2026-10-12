#include <HalSMExecutable.h>

Dict nulldicthsme={0,NOTHALSMNULLPOINTER};
HalSMVariable vnulldicthsme={&nulldicthsme,HalSMVariableType_HalSMDict};
HalSMNull nullhsme;
HalSMVariable vnullhsme={&nullhsme,HalSMVariableType_HalSMNull};

void HalSMExecutable_init(HalSMExecutable* hsme,HalFileSystem* hfs)
{
    hsme->hfs=hfs;
    hsme->version[0]=0;
    hsme->version[1]=0;
    hsme->version[2]=1;
}

unsigned int HalSMExecutable_load(HalSMExecutable* hsme,HalSMCompiler* hsmc)
{
    unsigned long long int index,i,l,size;
    HalSMVariable* var;
    HalStringFormatChar* temp,*path,*mainFile;
    if (hsme->hfs->disk==NOTHALSMNULLPOINTER){return 1;}
    path=U8H("HalSMExecutable.hcfg");
    if (HalFileSystem_getIndexFileByPath(hsme->hfs,&index,path,1)>0){return 2;}
    if (HalFileSystem_getSizeFile(hsme->hfs,path,&size)>0){return 3;}
    if (size<2){return 3;}
    temp=hsmc->memory->mml->calloc(size+1,sizeof(HalStringFormatChar));
    HalFileSystem_readFile(hsme->hfs,path,(unsigned char*)temp,0,size);
    if (HSFCharCompare(temp[0],ASCIIH('{'))==0||HSFCharCompare(temp[HalStringFormat_length(temp)-1],ASCIIH('}'))==0||HalStringFormat_length(temp)<3){return 4;}
    string_cpy(temp,&temp[1]);
    temp[HalStringFormat_length(temp)-1]=ASCIIH('\0');
    Dict* config=HalSMCompiler_getArgsDict(hsmc,temp,&nulldicthsme);
    hsmc->memory->mml->free(temp);
    if (DictElementIndexByKey(hsmc,config,HalSMVariable_init_str(hsmc->memory,U8H("mainFile")))==-1){return 5;}
    var=DictElementFindByKey(hsmc,config,HalSMVariable_init_str(hsmc->memory,U8H("mainFile")))->value;
    if (var->type!=HalSMVariableType_str){return 6;}
    mainFile=ConcatenateStrings(hsmc->memory,U8H("main/"),*(HalStringFormatChar**)var->value);
    if (HalFileSystem_getIndexFileByPath(hsme->hfs,&index,mainFile,1)){return 7;}
    HalFileSystem_getSizeFile(hsme->hfs,mainFile,&size);
    if (size==0){return 0;}
    //if ((size%4)>0){return 8;}
    //size/=4;
    char* mainFileUTF8=hsmc->memory->mml->calloc(size+1,sizeof(char));
    HalFileSystem_readFile(hsme->hfs,mainFile,(unsigned char*)mainFileUTF8,0,size);
    HalStringFormatChar* decodedMainFile=U8H(mainFileUTF8);
    hsmc->memory->mml->free(mainFileUTF8);
    ui64 mainFileLength=string_len(decodedMainFile);
    hsme->mainFile=HalSMMemory_calloc(hsmc->memory,mainFileLength+1,sizeof(HalStringFormatChar));
    stringncpy(hsme->mainFile,decodedMainFile,mainFileLength);
    hsmc->memory->mml->free(decodedMainFile);
    return 0;
}

void HalSMExecutable_run(HalSMExecutable* hsme,HalSMCompiler* hsmc)
{
    HalSMCompiler_compile(hsmc,hsme->mainFile,0,&vnullhsme);
    //return 0;
}
