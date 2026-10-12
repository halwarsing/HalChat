#include <HalSM.h>
#include <stdio.h>

HalSMNull hnull;
HalSMVariable null={&hnull,HalSMVariableType_HalSMNull};
HalSMPlus plus;
HalSMMinus minus;
HalSMMult mult;
HalSMDivide divide;
HalSMPow hpow;
HalSMModulo modulo;
HalSMEqual equal;
HalSMNotEqual notequal;
HalSMMore more;
HalSMLess less;
HalSMMoreEqual moreequal;
HalSMLessEqual lessequal;
HalSMAND and;
HalSMOR or;
HalSMNegate negate;
HalSMVariable vnegate={&negate,HalSMVariableType_HalSMNegate};
HalSMNegativeSign negativesign;
HalSMVariable vnegativesign={&negativesign,HalSMVariableType_HalSMNegativeSign};
HalSMShift shift_left={0};
HalSMShift shift_right={1};
HalSMArray* arrInt;
ui8 boolTrueVal=1;
HalSMVariable boolTrue={&boolTrueVal,HalSMVariableType_HalSMBool};
ui8 boolFalseVal=0;
HalSMVariable boolFalse={&boolFalseVal,HalSMVariableType_HalSMBool};
HalSMInlineIf inlineif;
HalSMInlineElse inlineelse;
HalSMBreak hbreak;
HalSMReturn hreturn;
HalSMVoid hsmvoid;
HalSMVariable hvoid={&hsmvoid,HalSMVariableType_void};
HalSMXOR hxor;
HalSMBAND hband;
HalSMBOR hbor;
HalSMBNOT hbnot;
HalSMVariable vxor={&hxor,HalSMVariableType_HalSMXOR};
HalSMVariable vband={&hband,HalSMVariableType_HalSMBAND};
HalSMVariable vbor={&hbor,HalSMVariableType_HalSMBOR};
HalSMVariable vbnot={&hbnot,HalSMVariableType_HalSMBNOT};

HalSMInteger HalSMIntegerZero={0,(ui8[]){0},1};
HalSMInteger HalSMIntegerOne={0,(ui8[]){1},1};
HalSMInteger HalSMIntegerTwo={0,(ui8[]){2},1};
HalSMInteger HalSMIntegerThree={0,(ui8[]){3},1};
HalSMInteger HalSMIntegerTen={0,(ui8[]){10},1};
HalSMInteger HalSMIntegerThousand={0,(ui8[]){3,232},2};

HalSMDouble HalSMDoubleOne={0,&HalSMIntegerOne,&HalSMIntegerOne};
HalSMDouble HalSMDoubleZero={0,&HalSMIntegerZero,&HalSMIntegerZero};
HalSMDouble HalSMDoubleTwo={0,&HalSMIntegerTwo,&HalSMIntegerOne};
HalSMDouble HalSMDoubleThree={0,&HalSMIntegerThree,&HalSMIntegerOne};
HalSMDouble HalSMDoubleTen={0,&HalSMIntegerTen,&HalSMIntegerOne};

Dict NULLDict={0,NOTHALSMNULLPOINTER};
HalSMArray NULLArray={0,NOTHALSMNULLPOINTER};
HalSMVariable HNULLArray={&NULLArray,HalSMVariableType_HalSMArray};

HalSMNull* HalSMNull_init(HalSMCompiler* hsmc) {
    HalSMNull* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMNull));
    return out;
}

HalSMError* HalSMError_init(HalSMCompiler* hsmc,HalSMErrorIndificator* ind,HalStringFormatChar* error){
    HalSMError* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMError));
    out->ind=ind;
    out->error=error;
    return out;
}

HalSMErrorIndificator* HalSMErrorIndificator_init(HalSMCompiler* hsmc,HalStringFormatChar* name,ui64 line,void* prev)
{
    HalSMErrorIndificator* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMErrorIndificator));
    out->line=line;
    out->name=name;
    out->prev=prev;
    return out;
}

//FILE SYSTEM

HalSMVariable* HalSMCompiler_isExistsPath(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    return HalSMVariable_init_bool(hsmc->memory,hsmc->filesystemlibrary->isExistsDir(*(HalStringFormatChar**)args->arr[0]->value)||hsmc->filesystemlibrary->isExistsFile(*(HalStringFormatChar**)args->arr[0]->value));
}

HalSMVariable* HalSMCompiler_isExistsFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    return HalSMVariable_init_bool(hsmc->memory,hsmc->filesystemlibrary->isExistsFile(*(HalStringFormatChar**)args->arr[0]->value));
}

HalSMVariable* HalSMCompiler_isExistsDir(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    return HalSMVariable_init_bool(hsmc->memory,hsmc->filesystemlibrary->isExistsDir(*(HalStringFormatChar**)args->arr[0]->value));
}

HalSMVariable* HalSMCompiler_getListFiles(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    return HalSMVariable_init(hsmc->memory,hsmc->filesystemlibrary->getListFiles(hsmc,*(HalStringFormatChar**)args->arr[0]->value),HalSMVariableType_HalSMArray);
}

HalSMVariable* HalSMCompiler_openFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==2) {
        return HalSMVariable_init(hsmc->memory,HalSMVariableCustom_init(hsmc->memory,(HalSMVariableTypeCustom*)hsmc->sys_custom_variables->elements[0]->value->value,hsmc->filesystemlibrary->openFile(*(HalStringFormatChar**)args->arr[0]->value,*(HalStringFormatChar**)args->arr[1]->value),&NULLDict,&NULLDict),HalSMVariableType_custom);
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_closeFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    hsmc->filesystemlibrary->closeFile(((HalSMVariableCustom*)args->arr[0]->value)->value);
    return &hvoid;
}

HalSMVariable* HalSMCompiler_readFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    ui64 count=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value);
    ui64 size=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[2]->value);
    HalStringFormatChar* out=HalSMMemory_malloc(hsmc->memory,count*size*sizeof(HalStringFormatChar));
    ui64 ncount=hsmc->filesystemlibrary->read(out,size,count,((HalSMVariableCustom*)args->arr[0]->value)->value);
    if (count>ncount) {
        out=HalSMMemory_realloc(hsmc->memory,out,ncount*size);
    }
    HalStringFormatChar** o=HalSMMemory_malloc(hsmc->memory,sizeof(HalStringFormatChar*));
    *o=out;
    return HalSMVariable_init(hsmc->memory,o,HalSMVariableType_str);
}

HalSMVariable* HalSMCompiler_puts(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    hsmc->filesystemlibrary->puts(((HalSMVariableCustom*)args->arr[0]->value)->value,*(HalStringFormatChar**)args->arr[1]->value);
    return &hvoid;
}

HalSMVariable* HalSMCompiler_seek(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    hsmc->filesystemlibrary->seek(((HalSMVariableCustom*)args->arr[0]->value)->value,HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value),HalSMInteger_ToUnsignedInteger((HalSMInteger*)args->arr[2]->value));
    return &hvoid;
}

HalSMVariable* HalSMCompiler_tell(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,hsmc->filesystemlibrary->tell(((HalSMVariableCustom*)args->arr[0]->value)->value)),HalSMVariableType_int);
}

HalSMVariable* HalSMCompiler_rewind(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    hsmc->filesystemlibrary->rewind(((HalSMVariableCustom*)args->arr[0]->value)->value);
    return &hvoid;
}
HalSMVariable* HalSMCompiler_flush(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    hsmc->filesystemlibrary->flush(((HalSMVariableCustom*)args->arr[0]->value)->value);
    return &hvoid;
}

//FILE SYSTEM

HalSMVariable* HalSMCompiler_input(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    HalStringFormatChar* text=*(HalStringFormatChar**)args->arr[0]->value;
    return HalSMVariable_init_str(hsmc->memory,hsmc->inputf(text));
}

HalSMVariable* HalSMCompiler_reversed(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    HalSMVariable* arr=args->arr[0];
    if (arr->type==HalSMVariableType_HalSMArray) {
        HalSMArray* out=HalSMArray_reverse(hsmc->memory,(HalSMArray*)arr->value);
        return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_HalSMArray);
    } else if (arr->type==HalSMVariableType_str) {
        HalStringFormatChar* arrv=*(HalStringFormatChar**)arr->value;
        ui64 arrl=string_len(arrv);
        if (arrl==0) {
            return HalSMVariable_init_str(hsmc->memory,U8H(""));
        }
        HalStringFormatChar* out=HalSMMemory_malloc(hsmc->memory,(arrl+1)*sizeof(HalStringFormatChar));
        i64 b=0;
        for (ui64 i=arrl;i--;) {
            out[b]=arrv[i];
            if (i==0) {break;}
            b++;
        }
        out[arrl]=ASCIIH('\0');
        return HalSMVariable_init_str(hsmc->memory,out);
    }
    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("This Variable not list or string")));
    return &hvoid;
}

HalSMVariable* HalSMCompiler_range(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==0) {return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->ind,U8H("Range cannot be without numeric args")),HalSMVariableType_HalSMError);}
    /*double start,end,step;
    if (args->size==1) {
        start=0;
        if (args->arr[0]->type==HalSMVariableType_int) {end=*(int*)args->arr[0]->value;}
        else {end=*(double*)args->arr[0]->value;}
        step=1;
    } else if (args->size==2) {
        if (args->arr[0]->type==HalSMVariableType_int) {start=*(int*)args->arr[0]->value;}
        else {start=*(double*)args->arr[0]->value;}
        if (args->arr[1]->type==HalSMVariableType_int) {end=*(int*)args->arr[1]->value;}
        else {end=*(double*)args->arr[1]->value;}
        step=1;
    } else if (args->size==3) {
        if (args->arr[0]->type==HalSMVariableType_int) {start=*(int*)args->arr[0]->value;}
        else {start=*(double*)args->arr[0]->value;}
        if (args->arr[1]->type==HalSMVariableType_int) {end=*(int*)args->arr[1]->value;}
        else {end=*(double*)args->arr[1]->value;}
        if (args->arr[2]->type==HalSMVariableType_int) {step=*(int*)args->arr[2]->value;}
        else {step=*(double*)args->arr[2]->value;}
    }
    HalSMArray* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMArray));
    out->size=MathCeilPos((MathAbs((end-start))/MathAbs(step)));
    out->arr=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable*)*out->size);
    double* p=HalSMMemory_malloc(hsmc->memory,sizeof(double)*out->size);

    HalSMVariable* t=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable)*out->size);
    for (ui64 i=0;i<out->size;i++) {
        p[i]=start+i*step;
        t[i].type=HalSMVariableType_double;
        t[i].value=&p[i];
        out->arr[i]=&t[i];
    }
    return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_HalSMArray);*/
    return &null;
}

HalSMArray* HalSMCompiler_get_print_text(HalSMCompiler* hsmc,HalSMArray* args,ui8 isToPrint) {
    HalSMArray* out=HalSMArray_init(hsmc->memory);
    HalSMVariable* a;
    for (ui64 i=0;i<args->size;i++) {
        a=args->arr[i];
        HalSMArray_add(hsmc->memory,out,HalSMVariable_init_str(hsmc->memory,HalSMVariable_to_str(hsmc,a,isToPrint)));
    }
    return out;
}

HalSMVariable* HalSMCompiler_print(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    HalSMArray* temp=HalSMCompiler_get_print_text(hsmc,args,0);
    hsmc->print(HalSMArray_join_str(hsmc->memory,temp,U8H(" ")));
    for (ui64 i=0;i<temp->size;i++) {
        HalSMMemory_free(hsmc->memory,*(HalStringFormatChar**)temp->arr[i]->value);
        HalSMMemory_free(hsmc->memory,temp->arr[i]->value);
        HalSMMemory_free(hsmc->memory,temp->arr[i]);
    }
    HalSMMemory_free(hsmc->memory,temp->arr);
    HalSMMemory_free(hsmc->memory,temp);
    return &hvoid;
}

HalSMVariable* HalSMCompiler_exit(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    hsmc->systemlibrary->exit(0);
    return &hvoid;
}

HalSMVariable* HalSMCompiler_for(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size!=2||args->arr[0]->type!=HalSMVariableType_str||args->arr[1]->type!=HalSMVariableType_HalSMLocalFunction) {
        return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Not Wrong Arguments")),HalSMVariableType_HalSMError);
    }
    HalSMArray* arguments=HalSMArray_split_str(hsmc->memory,*(HalStringFormatChar**)args->arr[0]->value,U8H(";"));
    if (arguments->size!=3){return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Not Wrong Count Arguments")),HalSMVariableType_HalSMError);}
    HalSMArray* variable=HalSMArray_split_str(hsmc->memory,*(HalStringFormatChar**)arguments->arr[0]->value,U8H("="));
    HalSMVariable* value;
    
    i64 index=DictElementIndexByKey(hsmc,vars,variable->arr[0]);
    if (index!=-1) {
        value=vars->elements[index]->value;
        HalSMVariable* v=HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)variable->arr[1]->value,vars)->arr[0];
        value->value=v->value;
        value->type=v->type;
    } else {
        index=DictElementIndexByKey(hsmc,hsmc->variables,variable->arr[0]);
        if (index!=-1) {
            value=hsmc->variables->elements[index]->value;
            HalSMVariable* v=HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)variable->arr[1]->value,vars)->arr[0];
            value->value=v->value;
            value->type=v->type;
        } else if (vars->elements!=NOTHALSMNULLPOINTER) {
            HalSMVariable* v=HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)variable->arr[1]->value,vars)->arr[0];
            value=HalSMVariable_init(hsmc->memory,v->value,v->type);
            DictElement* var=DictElementInit(hsmc->memory,variable->arr[0],value);
            PutDictElementToDict(hsmc,vars,var);
        } else {
            HalSMVariable* v=HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)variable->arr[1]->value,vars)->arr[0];
            value=HalSMVariable_init(hsmc->memory,v->value,v->type);
            DictElement* var=DictElementInit(hsmc->memory,variable->arr[0],value);
            PutDictElementToDict(hsmc,hsmc->variables,var);
        }
    }
    
    HalSMLocalFunction* lf=args->arr[1]->value;
    lf->name=U8H("for");
    HalSMVariable* err;
    while (1) {
        if (HalSMCompiler_getBool(hsmc,HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)arguments->arr[1]->value,vars)->arr[0])) {
            err=HalSMLocalFunction_run(lf,HalSMArray_init(hsmc->memory),vars);
            if (err->type==HalSMVariableType_HalSMError){return err;}
            else if(err->type==HalSMVariableType_HalSMBreak){return &hvoid;}
            else if(err->type==HalSMVariableType_HalSMReturn) {
                if (((HalSMReturn*)err->value)->idLocalFunction==lf->id){return &hvoid;}
                return err;
            }
            //value=HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)resRunFunc->args->arr[1]->value,vars)->arr[0];
            HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)arguments->arr[2]->value,vars);
        } else {break;}
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_while(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size!=2||args->arr[0]->type!=HalSMVariableType_str||args->arr[1]->type!=HalSMVariableType_HalSMLocalFunction) {
        return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Not Wrong Arguments")),HalSMVariableType_HalSMError);
    }
    HalSMLocalFunction* lf=args->arr[1]->value;
    HalSMVariable* err;
    
    lf->name=U8H("while");

    while (1) {
        if (HalSMCompiler_getBool(hsmc,HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)args->arr[0]->value,vars)->arr[0])) {
            err=HalSMLocalFunction_run(lf,HalSMArray_init(hsmc->memory),vars);
            if (err->type==HalSMVariableType_HalSMError){return err;}
            else if(err->type==HalSMVariableType_HalSMBreak){return &hvoid;}
            else if(err->type==HalSMVariableType_HalSMReturn) {
                if (((HalSMReturn*)err->value)->idLocalFunction==lf->id){return &hvoid;}
                return err;
            }
        } else {break;}
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_if(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size>3||args->size<2||args->arr[0]->type!=HalSMVariableType_str||args->arr[1]->type!=HalSMVariableType_HalSMLocalFunction) {
        return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Not Wrong Arguments")),HalSMVariableType_HalSMError);
    }
    HalSMLocalFunction* lf=args->arr[1]->value;
    lf->name=U8H("if");
    HalSMVariable* err;
    if (HalSMCompiler_getBool(hsmc,HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)args->arr[0]->value,vars)->arr[0])) {
        err=HalSMLocalFunction_run(lf,HalSMArray_init(hsmc->memory),vars);
        if (err->type==HalSMVariableType_HalSMError){return err;}
        else if(err->type==HalSMVariableType_HalSMBreak){return err;}
        else if(err->type==HalSMVariableType_HalSMReturn) {
            if (((HalSMReturn*)err->value)->idLocalFunction==lf->id){return &hvoid;}
            return err;
        }
        return err;
    } else if (args->size==3) {
        HalSMArray* logicFunctions=args->arr[2]->value;
        HalSMArray* logicTemp;
        for (ui64 i=0;i<logicFunctions->size;i++) {
            logicTemp=(HalSMArray*)logicFunctions->arr[i]->value;
            err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)logicTemp->arr[0]->value,(HalSMArray*)logicTemp->arr[1]->value,vars);
            if (err->type==HalSMVariableType_HalSMBool&&(*(ui8*)err->value)==1) {
                break;
            } else if (err->type==HalSMVariableType_HalSMError||err->type==HalSMVariableType_HalSMBreak||err->type==HalSMVariableType_HalSMReturn) {
                return err;
            }
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_elif(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size!=2||args->arr[0]->type!=HalSMVariableType_str||args->arr[1]->type!=HalSMVariableType_HalSMLocalFunction) {
        return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Not Wrong Arguments")),HalSMVariableType_HalSMError);
    }
    HalSMLocalFunction* lf=args->arr[1]->value;
    lf->name=U8H("elif");
    HalSMVariable* err;
    if (HalSMCompiler_getBool(hsmc,HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)args->arr[0]->value,vars)->arr[0])) {
        err=HalSMLocalFunction_run(lf,HalSMArray_init(hsmc->memory),vars);
        if (err->type==HalSMVariableType_HalSMError){return err;}
        else if(err->type==HalSMVariableType_HalSMBreak){return err;}
        else if(err->type==HalSMVariableType_HalSMReturn) {
            if (((HalSMReturn*)err->value)->idLocalFunction==lf->id){return &boolTrue;}
            return err;
        }
        return &boolTrue;
    }
    return &boolFalse;
}

HalSMVariable* HalSMCompiler_else(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size!=2||args->arr[0]->type!=HalSMVariableType_str||args->arr[1]->type!=HalSMVariableType_HalSMLocalFunction) {
        return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Not Wrong Arguments")),HalSMVariableType_HalSMError);
    }
    HalSMLocalFunction* lf=args->arr[1]->value;
    lf->name=U8H("else");
    HalSMVariable* err=HalSMLocalFunction_run(lf,HalSMArray_init(hsmc->memory),vars);
    if (err->type==HalSMVariableType_HalSMError){return err;}
    else if(err->type==HalSMVariableType_HalSMBreak){return err;}
    else if(err->type==HalSMVariableType_HalSMReturn) {
        if (((HalSMReturn*)err->value)->idLocalFunction==lf->id){return &boolTrue;}
        return err;
    }
    return &boolTrue;
}

HalSMVariable* HalSMCompiler_do(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size>3||args->size<2||args->arr[0]->type!=HalSMVariableType_str||args->arr[1]->type!=HalSMVariableType_HalSMLocalFunction) {
        return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Not Wrong Arguments")),HalSMVariableType_HalSMError);
    }
    HalSMLocalFunction* lf=args->arr[1]->value;
    lf->name=U8H("do");
    HalSMVariable* err=HalSMLocalFunction_run(lf,HalSMArray_init(hsmc->memory),vars);
    if (err->type==HalSMVariableType_HalSMError){return err;}
    else if(err->type==HalSMVariableType_HalSMBreak){return err;}
    else if(err->type==HalSMVariableType_HalSMReturn) {
        if (((HalSMReturn*)err->value)->idLocalFunction==lf->id){return &hvoid;}
        return err;
    }
    if (args->size==3) {
        HalSMArray* logicFunctions=args->arr[2]->value;
        HalSMArray* logicTemp;
        HalSMVariable* res;
        if (logicFunctions->size>0) {
            logicTemp=(HalSMArray*)logicFunctions->arr[0]->value;
            HalSMArray* logicArgs=HalSMArray_init(hsmc->memory);
            HalSMArray_add(hsmc->memory,logicArgs,((HalSMArray*)logicTemp->arr[1]->value)->arr[0]);
            HalSMArray_add(hsmc->memory,logicArgs,args->arr[1]);
            res=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)logicTemp->arr[0]->value,logicArgs,vars);
            if (res->type==HalSMVariableType_HalSMError) {
                return res;
            } else if(res->type==HalSMVariableType_HalSMBreak){return res;}
            else if(res->type==HalSMVariableType_HalSMReturn){return res;}
        }
    }
    return &hvoid;
}



HalSMCompiler* HalSMCompiler_init(HalStringFormatChar* path,void(*print)(HalStringFormatChar*),void(*printErrorf)(HalStringFormatChar*),HalStringFormatChar*(*inputf)(HalStringFormatChar*),HalSMVariable*(*getVariable)(HalStringFormatChar*),HalSMMemory* memory,HalSMSystemLibrary* systemlibrary,HalSMFileSystemLibrary* filesystemlibrary,HalSMVariable*(*HalSMLoadModule)(HalSMCompiler*,HalStringFormatChar*))
{
    HalSMCompiler* hsmc=HalSMMemory_malloc(memory,sizeof(HalSMCompiler));
    hsmc->versionName=U8H("0.1.8 (Static vs dynamic)");
    hsmc->version=18L;
    hsmc->path=path;
    hsmc->print=print;
    hsmc->printErrorf=printErrorf;
    hsmc->inputf=inputf;
    hsmc->getVariable=getVariable;
    //hsmc->memory=memorymanagmentlibrary;
    hsmc->memory=memory;
    hsmc->systemlibrary=systemlibrary;
    hsmc->filesystemlibrary=filesystemlibrary;
    hsmc->functions=DictInitWithElements(hsmc->memory,(DictElement*[]){
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("input")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_input),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("reversed")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_reversed),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("range")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_range),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("print")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_print),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("exit")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_exit),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("for")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_for),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("while")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_while),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("if")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_if),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("elif")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_elif),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("else")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_else),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("increment")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_increment),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("decrement")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_decrement),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("do")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_do),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("replace")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_replace),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("index")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_index),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("rindex")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_rindex),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("split")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_split),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("substring")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_substring),HalSMVariableType_HalSMFunctionC)),
        
        //ARRAY
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("append")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_append),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("len")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_len),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("clear")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_clear),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("insert")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_insert),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("pop")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_pop),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("remove")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_remove),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("reverse")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_reverse),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("copy")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_copy),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("appendArray")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_appendArray),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("slice")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_slice),HalSMVariableType_HalSMFunctionC)), 

        //DICT
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("appendDict")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_appendDict),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("indexByKey")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_indexByKey),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("indexByValue")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_indexByValue),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("set")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_set),HalSMVariableType_HalSMFunctionC)),

        //Math
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("sqrt")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_sqrt),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("int")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_int),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("double")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_double),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("str")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_str),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("floor")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_floor),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("ceil")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_ceil),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("round")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_round),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("abs")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_abs),HalSMVariableType_HalSMFunctionC)),

        //String
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("join")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_join),HalSMVariableType_HalSMFunctionC)),

        //File
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("fopen")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_openFile),HalSMVariableType_HalSMFunctionC)),

        //Convert
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("array")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_array),HalSMVariableType_HalSMFunctionC)),

        //System
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("dir")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_dir),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("keyByIndex")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_keyByIndex),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("valueByIndex")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_valueByIndex),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("dict")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_dict),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("pow")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_powFunc),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("bin")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_bin),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("hex")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_hex),HalSMVariableType_HalSMFunctionC)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("type")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HalSMCompiler_type),HalSMVariableType_HalSMFunctionC))
    },51);

    hsmc->sys_modules=DictInitWithElements(hsmc->memory,HalSMMemory_malloc(hsmc->memory,0),0);
    hsmc->calcVars=HalSMCalculateVars_init();
    hsmc->line=1;
    hsmc->sys_variables=DictInitWithElements(hsmc->memory,(DictElement*[]){
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("copyright")),HalSMVariable_init_str(hsmc->memory,U8H("Copyright (C) 2022 Halwarsing.net")))
    },1);
    hsmc->variables=DictCopy(hsmc->memory,hsmc->sys_variables);
    hsmc->modules=DictInit(hsmc->memory);
    hsmc->localFunctions=DictInit(hsmc->memory);
    hsmc->classes=DictInit(hsmc->memory);
    hsmc->groupFunctions=DictInitWithElements(hsmc->memory,(DictElement*[]){
        DictElementInit(hsmc->memory,
            HalSMVariable_init_str(hsmc->memory,U8H("if")),
            HalSMVariable_init(hsmc->memory,
                HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){
                    hsmc->functions->elements[8]->value,
                    hsmc->functions->elements[9]->value
                },2),
                HalSMVariableType_HalSMArray
            )
        ),
        DictElementInit(hsmc->memory,
            HalSMVariable_init_str(hsmc->memory,U8H("do")),
            HalSMVariable_init(hsmc->memory,
                HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){
                    hsmc->functions->elements[6]->value,
                    hsmc->functions->elements[5]->value
                },2),
                HalSMVariableType_HalSMArray
            )
        )
    },2);

    hsmc->sys_custom_variables=DictInitWithElements(hsmc->memory,(DictElement*[]){
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("FilePointer")),HalSMVariable_init(hsmc->memory,HalSMVariableTypeCustom_init(hsmc->memory,(HalSMVariableTypeCustom){
            U8H("FilePointer"),
            HFP_toString,
            HFP_add,
            HFP_sub,
            HFP_mul,
            HFP_div,
            HFP_pow,
            HFP_modulo,
            HFP_increment,
            HFP_decrement,
            HFP_isEqual,
            HFP_isMore,
            HFP_isLess,
            HFP_getBool,
            HFP_indexVariableByName,
            HFP_indexFunctionByName,
            HFP_getVariableByName,
            HFP_getFunctionByName,
            HFP_getItem,
            HFP_shiftRight,
            HFP_shiftLeft,
            HFP_convert,
            HFP_xor,
            HFP_and,
            HFP_or,
            HFP_bnot,
            DictInit(hsmc->memory),
            DictInitWithElements(hsmc->memory,(DictElement*[]){
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("close")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_close),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("read")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_read),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("write")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_write),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("eof")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_eof),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("seek")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_seek),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("tell")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_tell),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("rewind")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_rewind),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("flush")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_flush),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("readUTF8")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_readUTF8),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("writeUTF8")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_writeUTF8),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("readHSF")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_readHSF),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("writeHSF")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_writeHSF),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("readUTF16")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_readUTF16),HalSMVariableType_HalSMFunctionC)),
                DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("writeUTF16")),HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,&HFP_writeUTF16),HalSMVariableType_HalSMFunctionC)),
            },14)
        }),HalSMVariableType_custom)),
        DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,U8H("ByteArray")),HalSMVariable_init(hsmc->memory,HalSMVariableTypeCustom_init(hsmc->memory,(HalSMVariableTypeCustom){
            U8H("ByteArray"),
            HBA_toString,
            HBA_add,
            HBA_sub,
            HBA_mul,
            HBA_div,
            HBA_pow,
            HBA_modulo,
            HBA_increment,
            HBA_decrement,
            HBA_isEqual,
            HBA_isMore,
            HBA_isLess,
            HBA_getBool,
            HBA_indexVariableByName,
            HBA_indexFunctionByName,
            HBA_getVariableByName,
            HBA_getFunctionByName,
            HBA_getItem,
            HBA_shiftRight,
            HBA_shiftLeft,
            HBA_convert,
            HBA_xor,
            HBA_and,
            HBA_or,
            HBA_bnot,
            DictInit(hsmc->memory),
            DictInit(hsmc->memory)
        }),HalSMVariableType_custom)),
    },2);
    hsmc->GOTOSectors=DictInit(hsmc->memory);
    hsmc->HalSMLoadModule=HalSMLoadModule;
    hsmc->ind=HalSMErrorIndificator_init(hsmc,U8H("MAIN"),0,NOTHALSMNULLPOINTER);
    hsmc->curInd=hsmc->ind;
    hsmc->isClose=0;
    return hsmc;
}

ui64 HalSMCompiler_getTabs(HalSMCompiler* hsmc,HalStringFormatChar* l)
{
    i64 s=string_len(l);
    for (int i=0;i<s;i++) {
        if (HSFCharCompare(l[i],ASCIIH(' '))==0)return i;
    }
    return s;
}

ui64 HalSMCompiler_getIndexTabs(ui64* tabsArr,ui64 tabsS,ui64 tabsC)
{
    ui64 i;
    for (i=0;i<tabsS;i++) {
        if (tabsC<=tabsArr[i])return i;
    }
    return i;
}

HalSMArray* HalSMCompiler_getLines(HalSMCompiler* hsmc,HalStringFormatChar* text)
{
    HalSMArray* out=HalSMArray_init(hsmc->memory);
    HalStringFormatChar* o=HalSMMemory_malloc(hsmc->memory,0);
    ui64 s=0;
    HalStringFormatChar isS=ASCIIH('n');
    ui64 isF=0,isA=0,isD=0;
    HalStringFormatChar i;
    ui64 lt=string_len(text);
    ui64 tabsI=0;
    ui64 d;
    ui64* tabsArr=HalSMMemory_malloc(hsmc->memory,0);
    ui64 lti=lt-1;
    ui8 isComment=0;

    ui8 isParams=0;

    for (d=0;d<lt;d++) {
        i=text[d];
        if(isParams&&HSFCharCompare(i,ASCIIH(' '))==0&&HSFCharCompare(i,ASCIIH('{'))==0&&HSFCharCompare(i,ASCIIH('\n'))==0&&HSFCharCompare(i,ASCIIH(';'))==0) {
            isParams=0;
        }

        if (isComment) {
            if(HSFCharCompare(i,ASCIIH('\n'))||HSFCharCompare(i,ASCIIH(';'))) {
                isComment=0;
            }
        }

        if (HSFCharCompare(i,ASCIIH('\r'))) {

        } else if(isComment) {

        } else if (HSFCharCompare(isS,ASCIIH('n'))==0) {
            if (HSFCharCompare(i,isS)) {
                isS=ASCIIH('n');
            }
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=i;
        } else if (isF>0) {
            if (HSFCharCompare(i,ASCIIH(')'))) {
                isF--;
                if(isF==0) {
                    isParams=1;
                }
            } else if (HSFCharCompare(i,ASCIIH('('))) {
                if(isParams){isParams=0;}
                isF++;
            }
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=i;
        } else if (isA>0) {
            if (HSFCharCompare(i,ASCIIH(']'))) {
                isA--;
            } else if (HSFCharCompare(i,ASCIIH('['))) {
                isA++;
            }
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=i;
        } else if (HSFCharCompare(i,ASCIIH('\n'))||HSFCharCompare(i,ASCIIH(';'))) {
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=ASCIIH('\0');

            HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,o),HalSMVariable_init_unsigned_int(hsmc->memory,tabsI)},2),HalSMVariableType_HalSMArray));
            s=0;
            o=HalSMMemory_malloc(hsmc->memory,0);
        } else if (isD>0) {
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=i;
            if (HSFCharCompare(i,ASCIIH('}'))) {
                if(tabsI>0&&tabsArr[tabsI-1]==isD) {
                    tabsArr=HalSMMemory_realloc(hsmc->memory,tabsArr,tabsI*sizeof(ui64));

                    if(s>1) {
                        o[s-1]=ASCIIH('\0');
                        if (HalSMCompiler_isNull(o)==0) {
                            HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,o),HalSMVariable_init_unsigned_int(hsmc->memory,tabsI)},2),HalSMVariableType_HalSMArray));
                        }
                    }
                    s=0;
                    o=HalSMMemory_malloc(hsmc->memory,0);
                    tabsI--;
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,U8H("}")),HalSMVariable_init_unsigned_int(hsmc->memory,tabsI)},2),HalSMVariableType_HalSMArray));
                }
                isD--;
            } else if (HSFCharCompare(i,ASCIIH('{'))) {
                isD++;

                o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
                o[s-1]=U8CH('\0');

                isParams=StringEndsWith(hsmc->memory,HalSMCompiler_removeEndSpaces(hsmc->memory,o),U8H(")"))&&StringIndexOf(hsmc->memory,o,U8H("("))!=-1;

                o[s-1]=i;

                if(isParams) {
                    isParams=0;
                    tabsArr=HalSMMemory_realloc(hsmc->memory,tabsArr,tabsI*sizeof(ui64)+sizeof(ui64));
                    tabsArr[tabsI++]=isD;

                    s++;
                    o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
                    o[s-1]=ASCIIH('\0');

                    if (HalSMCompiler_isNull(o)==0) {
                        HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,o),HalSMVariable_init_unsigned_int(hsmc->memory,tabsI-1)},2),HalSMVariableType_HalSMArray));
                    }

                    s=0;
                    o=HalSMMemory_malloc(hsmc->memory,0);
                } //is Function Body
            }
        } else if (HSFCharCompare(i,ASCIIH('"'))||HSFCharCompare(i,ASCIIH('\''))) {
            isS=i;
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=i;
        } else if(HSFCharCompare(i,ASCIIH(' '))&&s==0) {

        } else if (HSFCharCompare(i,ASCIIH('#'))&&s==0) {
            isComment=1;
        } else if (HSFCharCompare(i,ASCIIH('('))) {
            isF++;
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=i;
        } else if (HSFCharCompare(i,ASCIIH('\\'))) {
            if (d<lti) {
                d++;
                s+=2;
                o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
                o[s-2]=i;
                o[s-1]=text[d+1];
            } else {
                s++;
                o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
                o[s-1]=i;
            }
        } else if (HSFCharCompare(i,ASCIIH('['))) {
            isA++;
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=i;
        } else if (HSFCharCompare(i,ASCIIH('{'))) {
            isD++;
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=U8CH('\0');
            if(StringStartsWith(hsmc->memory,o,U8H("class "))==1){isParams=1;}
            o[s-1]=i;
            if(isParams){
                isParams=0;
                tabsArr=HalSMMemory_realloc(hsmc->memory,tabsArr,tabsI*sizeof(ui64)+sizeof(ui64));
                tabsArr[tabsI++]=isD;

                s++;
                o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
                o[s-1]=ASCIIH('\0');

                if (HalSMCompiler_isNull(o)==0) {
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,o),HalSMVariable_init_unsigned_int(hsmc->memory,tabsI-1)},2),HalSMVariableType_HalSMArray));
                }
                s=0;
                o=HalSMMemory_malloc(hsmc->memory,0);
            } //is Function Body
        } else {
            s++;
            o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
            o[s-1]=i;
        }
    }
    if (s>0) {
        s++;
        o=HalSMMemory_realloc(hsmc->memory,o,s*sizeof(HalStringFormatChar));
        o[s-1]=ASCIIH('\0');
        if (HalSMCompiler_isNull(o)==0) {
            /*tabsC=HalSMCompiler_getTabs(hsmc,o);
            tabsI=HalSMCompiler_getIndexTabs(tabsArr,tabsS,tabsC);
            if (tabsC<tabsF){tabsI--;}
            HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,SubString(hsmc->memory,o,tabsC,string_len(o))),HalSMVariable_init_unsigned_int(hsmc->memory,tabsI)},2),HalSMVariableType_HalSMArray));*/

            HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,o),HalSMVariable_init_unsigned_int(hsmc->memory,tabsI)},2),HalSMVariableType_HalSMArray));
        }
        s=0;
    }
    HalSMMemory_free(hsmc->memory,o);
    return out;
}

HalStringFormatChar* HalSMCompiler_removeSpaces(HalSMMemory* memory,HalStringFormatChar* text)
{
    ui64 l,i,s;
    l=string_len(text);
    if (l==0){return text;}
    HalStringFormatChar* out=HalSMMemory_malloc(memory,0);
    HalStringFormatChar space=ASCIIH(' ');
    for (i=0,s=0;i<l;i++) {
        if (HSFCharCompare(text[i],space)==0) {
            s++;
            out=HalSMMemory_realloc(memory,out,s*sizeof(HalStringFormatChar));
            out[s-1]=text[i];
        }
    }
    s++;
    out=HalSMMemory_realloc(memory,out,s*sizeof(HalStringFormatChar));
    out[s-1]=ASCIIH('\0');
    return out;
}

HalStringFormatChar* HalSMCompiler_removeStartSpaces(HalSMMemory* memory,HalStringFormatChar* text) {
    ui64 l,i,s;
    l=string_len(text);
    if (l==0){return text;}
    HalStringFormatChar space=ASCIIH(' ');
    for (i=0,s=0;i<l;i++) {
        if (HSFCharCompare(text[i],space)==0) {
            break;
        }
        s++;
    }
    if(s==l){return U8H("");}
    return SubString(memory,text,s,l);
}

HalStringFormatChar* HalSMCompiler_removeEndSpaces(HalSMMemory* memory,HalStringFormatChar* text) {
    ui64 l,i,s;
    l=string_len(text);
    if (l==0){return text;}
    HalStringFormatChar space=ASCIIH(' ');
    for (i=l-1,s=0;;i--) {
        if (HSFCharCompare(text[i],space)==0) {
            break;
        }
        s++;
        if(i==0)break;
    }
    if(s==l){return U8H("");}
    return SubString(memory,text,0,l-s);
}

void HalSMCompiler_ThrowError(HalSMCompiler* hsmc,HalSMError* err)
{
    HalStringFormatChar* sl;
    HalStringFormatChar* out=U8H("");
    //string_cpy(out,U8H("Error at line "));
    //string_cat(out,sl);
    HalSMErrorIndificator* ind=err->ind;
    while (1) {
        sl=HalSMInteger_toString(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,ind->line+1),0);
        out=ConcatenateStrings(hsmc->memory,out,ConcatenateStrings(hsmc->memory,U8H("["),ConcatenateStrings(hsmc->memory,ind->name,ConcatenateStrings(hsmc->memory,U8H("] at line "),sl))));
        out=ConcatenateStrings(hsmc->memory,out,U8H(": "));

        //Print Code
        HalSMArray* allLinesTemp;
        HalStringFormatStr l;
        if(ind->line<hsmc->allLines->size) {
            allLinesTemp=(HalSMArray*)hsmc->allLines->arr[ind->line]->value;
            l=*(HalStringFormatChar**)allLinesTemp->arr[0]->value;
        } else {
            l=U8H("[NO CODE]");
        }

        out=ConcatenateStrings(hsmc->memory,out,l);

        out=ConcatenateStrings(hsmc->memory,out,U8H("\n"));

        if (ind->prev==NOTHALSMNULLPOINTER){break;}
        ind=(HalSMErrorIndificator*)ind->prev;
    }
    out=ConcatenateStrings(hsmc->memory,out,U8H("Error: "));
    out=ConcatenateStrings(hsmc->memory,out,err->error);
    hsmc->printErrorf(out);
}

ui8 HalSMCompiler_isGet(HalSMCompiler* hsmc,HalStringFormatChar* l,HalSMVariable* outvar,ui8 isNewVar,Dict* vars)
{
    ui8 out;
    HalSMVariable* outv=HalSMCompiler_isGetU(hsmc,l,&out,isNewVar,vars);
    outvar->value=outv->value;
    outvar->type=outv->type;
    return out;
}

HalSMVariable* HalSMCompiler_isGetU(HalSMCompiler* hsmc,HalStringFormatChar* l,ui8* isOut,ui8 isNewVar,Dict* vars)
{
    HalSMVariable* out=&null;
    ui64 isInd=0;
    ui64 isA=0;
    ui64 isD=0;
    HalSMArray* o=HalSMArray_init(hsmc->memory);
    HalSMVariable* module=&null;
    ui64 indF=0;
    HalStringFormatChar isCovichki=ASCIIH('n');
    ui64 index=0;
    HalStringFormatChar i;
    HalSMVariable* obuff;
    HalStringFormatChar* obuffs;
    ui64 ll=string_len(l);
    ui64 li=ll-1;
    ui8 isIncrementDecrement=0;
    ui64 ignore=0;
    ui8 isNotNull=0;
    HalSMVariable* vartemp=&null;

    l=HalSMCompiler_removeStartSpaces(hsmc->memory,HalSMCompiler_removeEndSpaces(hsmc->memory,l));

    for (ui64 ii=0;ii<ll;ii++) {
        i=l[ii];
        if (ignore>0) {
            ignore--;
            continue;
        }

        if (HSFCharCompare(isCovichki,ASCIIH('n'))==0) {
            if (HSFCharCompare(i,isCovichki)) {
                isCovichki=ASCIIH('n');
                out=HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,o));
                isNotNull=1;
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            } else {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (indF>0) {
            if (HSFCharCompare(i,ASCIIH('('))) {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('(')));
                indF++;
            } else if (HSFCharCompare(i,ASCIIH(')'))) {
                indF--;
                if (indF==0) {
                    if (out->type==HalSMVariableType_HalSMFunctionC) {
                        HalSMArray* args=HalSMCompiler_getArgs(hsmc,HalSMArray_chars_to_str(hsmc->memory,o),vars);
                        if (module->type!=HalSMVariableType_HalSMNull&&module->type!=HalSMVariableType_HalSMModule) {HalSMArray_insert(hsmc->memory,args,module,0);}
                        out=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)out->value,args,vars);
                        if (out->type==HalSMVariableType_HalSMError) {
                            return out;
                        }
                        isNotNull=1;
                        o=HalSMArray_init(hsmc->memory);
                    } else if (out->type==HalSMVariableType_HalSMLocalFunction) {
                        HalSMArray* args=HalSMCompiler_getArgs(hsmc,HalSMArray_chars_to_str(hsmc->memory,o),vars);
                        if (module->type!=HalSMVariableType_HalSMNull&&module->type!=HalSMVariableType_HalSMModule) {
                            HalSMArray_insert(hsmc->memory,args,module,0);
                        }
                        out=HalSMLocalFunction_run((HalSMLocalFunction*)out->value,args,vars);
                        if (out->type==HalSMVariableType_HalSMError){return out;}
                        isNotNull=1;
                        o=HalSMArray_init(hsmc->memory);
                    } else if (out->type==HalSMVariableType_HalSMClassC) {
                        HalSMArray* args=HalSMCompiler_getArgs(hsmc,HalSMArray_chars_to_str(hsmc->memory,o),vars);
                        out=HalSMVariable_init(hsmc->memory,HalSMClassC_run(hsmc,(HalSMClassC*)out->value,args),HalSMVariableType_HalSMRunClassC);
                        isNotNull=1;
                        o=HalSMArray_init(hsmc->memory);
                    } else if (out->type==HalSMVariableType_HalSMClass) {
                        HalSMArray* args=HalSMCompiler_getArgs(hsmc,HalSMArray_chars_to_str(hsmc->memory,o),vars);
                        out=HalSMVariable_init(hsmc->memory,HalSMClass_run((HalSMClass*)out->value,hsmc,args),HalSMVariableType_HalSMRunClass);
                        isNotNull=1;
                        o=HalSMArray_init(hsmc->memory);
                    } else if (out->type==HalSMVariableType_HalSMNull||out->type==HalSMVariableType_void) {
                        obuff=HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,o));
                        out=HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)obuff->value,vars)->arr[0];
                        isNotNull=1;
                        o=HalSMArray_init(hsmc->memory);
                    } else {
                        HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(')')));
                    }
                    module=&null;
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                } else {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(')')));
                }
            } else {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (isInd>0) {
            if (HSFCharCompare(i,ASCIIH(']'))) {
                isInd--;
                if (isInd==0) {
                    obuffs=HalSMArray_chars_to_str(hsmc->memory,o);
                    out=HalSMCompiler_getArrayByIndex(hsmc,obuffs,out,&isNotNull,vars);
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                } else {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                }
            } else {
                if (HSFCharCompare(i,ASCIIH('['))) {
                    isInd++;
                }
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (isA>0) {
            if (HSFCharCompare(i,ASCIIH(']'))) {
                isA--;
                if (isA==0) {
                    out=HalSMVariable_init(hsmc->memory,HalSMCompiler_getArgs(hsmc,HalSMArray_chars_to_str(hsmc->memory,o),vars),HalSMVariableType_HalSMArray);
                    isNotNull=1;
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                }
            } else if (HSFCharCompare(i,ASCIIH('['))) {
                isA++;
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            } else {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (isD>0) {
            if (HSFCharCompare(i,ASCIIH('}'))) {
                isD--;
                if (isD==0) {
                    out=HalSMVariable_init(hsmc->memory,HalSMCompiler_getArgsDict(hsmc,HalSMArray_chars_to_str(hsmc->memory,o),vars),HalSMVariableType_HalSMDict);
                    isNotNull=1;
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                } else {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                }
            } else if (HSFCharCompare(i,ASCIIH('{'))) {
                isD++;
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            } else {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (HSFCharCompare(i,ASCIIH('\\'))) {
            if (ii<li) {
                HalStringFormatChar ichr=l[ii+1];
                ignore=1;
                if (HSFCharCompare(ichr,ASCIIH('a'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x07)));
                } else if (HSFCharCompare(ichr,ASCIIH('b'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x08)));
                } else if (HSFCharCompare(ichr,ASCIIH('e'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x1b)));
                } else if (HSFCharCompare(ichr,ASCIIH('f'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x0c)));
                } else if (HSFCharCompare(ichr,ASCIIH('n'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x0a)));
                } else if (HSFCharCompare(ichr,ASCIIH('r'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x0d)));
                } else if (HSFCharCompare(ichr,ASCIIH('t'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x09)));
                } else if (HSFCharCompare(ichr,ASCIIH('v'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x0b)));
                } else if (HSFCharCompare(ichr,ASCIIH('\\'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x5c)));
                } else if (HSFCharCompare(ichr,ASCIIH('"'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x22)));
                } else if (HSFCharCompare(ichr,ASCIIH('\''))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x27)));
                } else if (HSFCharCompare(ichr,ASCIIH('?'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(0x3f)));
                } else if (HSFCharCompare(ichr,ASCIIH('0'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('\0')));
                } else if (HSFCharCompare(ichr,ASCIIH('1'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('\1')));
                } else if (HSFCharCompare(ichr,ASCIIH('2'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('\2')));
                } else if (HSFCharCompare(ichr,ASCIIH('3'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('\3')));
                } else if (HSFCharCompare(ichr,ASCIIH('4'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('\4')));
                } else if (HSFCharCompare(ichr,ASCIIH('5'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('\5')));
                } else if (HSFCharCompare(ichr,ASCIIH('6'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('\6')));
                } else if (HSFCharCompare(ichr,ASCIIH('7'))) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('\7')));
                } else {
                    ignore=0;
                }
            } else {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (HSFCharCompare(i,ASCIIH('"'))||HSFCharCompare(i,ASCIIH('\''))) {
            isCovichki=i;
        } else if (HSFCharCompare(i,ASCIIH('['))) {
            if (o->size>0) {
                obuff=HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,o));
                out=HalSMCompiler_isGetValue(hsmc,obuff,isOut,isNewVar,vars,&isNotNull,&module,out,U8H("["));
                o=HalSMArray_init(hsmc->memory);
                isInd++;
            } else if (isNotNull) {
                isInd++;
            } else {
                isA++;
            }
        } else if (HSFCharCompare(i,ASCIIH('{'))) {
            isD++;
        } else if (HSFCharCompare(i,ASCIIH('('))) {
            obuff=HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,o));
            if (string_len(*(HalStringFormatChar**)obuff->value)>0) {
                if (out->type==HalSMVariableType_HalSMModule) {
                    if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->variables,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->variables,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->localFunctions,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->localFunctions,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->classes,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->classes,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->functions,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->functions,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->modules,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->modules,obuff)->value;
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class or Module Not Found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (out->type==HalSMVariableType_HalSMCModule) {
                    if (DictElementIndexByKey(hsmc,((HalSMCModule*)out->value)->lfuncs,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMCModule*)out->value)->lfuncs,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMCModule*)out->value)->vrs,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMCModule*)out->value)->vrs,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMCModule*)out->value)->classes,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMCModule*)out->value)->classes,obuff)->value;
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class or Module Not Found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (out->type==HalSMVariableType_HalSMRunClass) {
                    if (DictElementIndexByKey(hsmc,((HalSMRunClass*)out->value)->funcs,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMRunClass*)out->value)->funcs,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMRunClass*)out->value)->vars,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMRunClass*)out->value)->vars,obuff)->value;
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function Not Found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (out->type==HalSMVariableType_HalSMClass) {
                    if (DictElementIndexByKey(hsmc,((HalSMClass*)out->value)->funcs,obuff)!=-1) {
                        out=DictElementFindByKey(hsmc,((HalSMClass*)out->value)->funcs,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMClass*)out->value)->vars,obuff)!=-1) {
                        out=DictElementFindByKey(hsmc,((HalSMClass*)out->value)->vars,obuff)->value;
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function Not Found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (out->type==HalSMVariableType_HalSMRunClassC) {
                    if (DictElementIndexByKey(hsmc,((HalSMRunClassC*)out->value)->funcs,obuff)!=-1) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMRunClassC*)out->value)->funcs,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMRunClassC*)out->value)->vrs,obuff)!=-1&&DictElementFindByKey(hsmc,((HalSMRunClassC*)out->value)->vrs,obuff)->value->type==HalSMVariableType_HalSMFunctionC) {
                        module=out;
                        out=DictElementFindByKey(hsmc,((HalSMRunClassC*)out->value)->vrs,obuff)->value;
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function Not Found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (out->type==HalSMVariableType_HalSMClassC) {
                    if (DictElementIndexByKey(hsmc,((HalSMClassC*)out->value)->funcs,obuff)!=-1) {
                        out=DictElementFindByKey(hsmc,((HalSMClassC*)out->value)->funcs,obuff)->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,((HalSMClassC*)out->value)->vrs,obuff)!=-1&&DictElementFindByKey(hsmc,((HalSMClassC*)out->value)->vrs,obuff)->value->type==HalSMVariableType_HalSMFunctionC) {
                        out=DictElementFindByKey(hsmc,((HalSMClassC*)out->value)->vrs,obuff)->value;
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function Not Found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (out->type==HalSMVariableType_custom) {
                    if (((HalSMVariableCustom*)out->value)->type->indexFunctionByName(hsmc,out,*(HalStringFormatChar**)obuff->value)!=-1) {
                        module=out;
                        out=((HalSMVariableCustom*)out->value)->type->getFunctionByName(hsmc,out,*(HalStringFormatChar**)obuff->value);
                        isNotNull=1;
                    } else if (((HalSMVariableCustom*)out->value)->type->indexVariableByName(hsmc,out,*(HalStringFormatChar**)obuff->value)!=-1) {
                        out=((HalSMVariableCustom*)out->value)->type->getVariableByName(hsmc,out,*(HalStringFormatChar**)obuff->value);
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Function or Variable of Custom Variable not found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (out->type==HalSMVariableType_str) {
                    HalStringFormatChar* strFunc=*(HalStringFormatChar**)obuff->value;
                    if (HSFCompare(strFunc,U8H("replace"))) {
                        module=out;
                        out=hsmc->functions->elements[13]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("index"))) {
                        module=out;
                        out=hsmc->functions->elements[14]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("rindex"))) {
                        module=out;
                        out=hsmc->functions->elements[15]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("split"))) {
                        module=out;
                        out=hsmc->functions->elements[16]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("substring"))) {
                        module=out;
                        out=hsmc->functions->elements[17]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("len"))) {
                        module=out;
                        out=hsmc->functions->elements[19]->value;
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Function not found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (out->type==HalSMVariableType_HalSMArray) {
                    HalStringFormatChar* strFunc=*(HalStringFormatChar**)obuff->value;
                    if (HSFCompare(strFunc,U8H("append"))) {
                        module=out;
                        out=hsmc->functions->elements[18]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("len"))) {
                        module=out;
                        out=hsmc->functions->elements[19]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("clear"))) {
                        module=out;
                        out=hsmc->functions->elements[20]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("index"))) {
                        module=out;
                        out=hsmc->functions->elements[14]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("insert"))) {
                        module=out;
                        out=hsmc->functions->elements[21]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("pop"))) {
                        module=out;
                        out=hsmc->functions->elements[22]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("remove"))) {
                        module=out;
                        out=hsmc->functions->elements[23]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("reverse"))) {
                        module=out;
                        out=hsmc->functions->elements[24]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("copy"))) {
                        module=out;
                        out=hsmc->functions->elements[25]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("appendArray"))) {
                        module=out;
                        out=hsmc->functions->elements[26]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("slice"))) {
                        module=out;
                        out=hsmc->functions->elements[27]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("set"))) {
                        module=out;
                        out=hsmc->functions->elements[31]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("join"))) {
                        module=out;
                        out=hsmc->functions->elements[39]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("rindex"))) {
                        module=out;
                        out=hsmc->functions->elements[15]->value;
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Function not found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (out->type==HalSMVariableType_HalSMDict) {
                    HalStringFormatChar* strFunc=*(HalStringFormatChar**)obuff->value;
                    if (HSFCompare(strFunc,U8H("append"))) {
                        module=out;
                        out=hsmc->functions->elements[18]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("appendDict"))) {
                        module=out;
                        out=hsmc->functions->elements[28]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("len"))) {
                        module=out;
                        out=hsmc->functions->elements[19]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("clear"))) {
                        module=out;
                        out=hsmc->functions->elements[20]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("indexByKey"))) {
                        module=out;
                        out=hsmc->functions->elements[29]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("indexByValue"))) {
                        module=out;
                        out=hsmc->functions->elements[30]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("set"))) {
                        module=out;
                        out=hsmc->functions->elements[31]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("pop"))) {
                        module=out;
                        out=hsmc->functions->elements[22]->value;
                        isNotNull=1;
                    } else if (DictElementIndexByKey(hsmc,(Dict*)out->value,obuff)!=-1) {
                        out=DictElementFindByKey(hsmc,(Dict*)out->value,obuff)->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("keyByIndex"))) {
                        module=out;
                        out=hsmc->functions->elements[43]->value;
                        isNotNull=1;
                    } else if (HSFCompare(strFunc,U8H("valueByIndex"))) {
                        module=out;
                        out=hsmc->functions->elements[44]->value;
                        isNotNull=1;
                    } else {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Function not found")));
                        *isOut=0;
                        return &hvoid;
                    }
                } else if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1) {
                    out=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value;
                    isNotNull=1;
                } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,obuff)!=-1) {
                    out=DictElementFindByKey(hsmc,hsmc->localFunctions,obuff)->value;
                    isNotNull=1;
                } else if (DictElementIndexByKey(hsmc,hsmc->classes,obuff)!=-1) {
                    out=DictElementFindByKey(hsmc,hsmc->classes,obuff)->value;
                    isNotNull=1;
                } else if (DictElementIndexByKey(hsmc,hsmc->variables,obuff)!=-1) {
                    out=DictElementFindByKey(hsmc,hsmc->variables,obuff)->value;
                    isNotNull=1;
                } else if ((vartemp=hsmc->getVariable(*(HalStringFormatChar**)obuff->value))->type!=HalSMVariableType_HalSMNull) {
                    out=vartemp;
                    isNotNull=1;
                    vartemp=&null;
                } else if (DictElementIndexByKey(hsmc,vars,obuff)!=-1) {
                    out=DictElementFindByKey(hsmc,vars,obuff)->value;
                    isNotNull=1;
                } else {
                    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Is not function")));
                    *isOut=0;
                    return &hvoid;
                }
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            } else if (out->type==HalSMVariableType_HalSMNull||out->type==HalSMVariableType_void) {

            } else {
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            }
            indF++;
        } else if (HSFCharCompare(i,ASCIIH(')'))) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(')')));
        } else if (HSFCharCompare(i,ASCIIH('.'))) {
            obuff=HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,o));
            out=HalSMCompiler_isGetValue(hsmc,obuff,isOut,isNewVar,vars,&isNotNull,&module,out,U8H("."));
            o=HalSMArray_init(hsmc->memory);
        /*}   else if (HSFCharCompare(i,ASCIIH('+'))&&ii<li&&HSFCharCompare(l[ii+1],ASCIIH('='))&&indF==0) {
            if (o->size==0) {
                if (out->type==HalSMVariableType_HalSMNull||out->type==HalSMVariableType_void) {
                    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("+= can't be in start of variable")));
                    *isOut=0;
                    return &hvoid;
                } else {
                    HalSMVariable* temp=out;
                    out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
                    out->type=temp->type;
                    out->value=temp->value;
                    HalSMVariable* inc=HalSMCompiler_addSelfVariable(hsmc,out);
                    temp->value=inc->value;
                    temp->type=inc->type;
                    isNotNull=1;
                    //halwarsing()+=
                }
            } else {
                obuff=HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,o));
                //halwarsing.net+=

                out=HalSMCompiler_isGetValue(hsmc,obuff,isOut,isNewVar,vars,&isNotNull,&module,out,U8H("+="));
                HalSMVariable* temp=out;
                out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
                out->type=temp->type;
                out->value=temp->value;
                HalSMVariable* inc=HalSMCompiler_addSelfVariable(hsmc,out);
                temp->value=inc->value;
                temp->type=inc->type;
                isNotNull=1;
                o=HalSMArray_init(hsmc->memory);
            }
            ignore=1;*/
        } else if (HSFCharCompare(i,ASCIIH('+'))&&ii<li&&HSFCharCompare(l[ii+1],ASCIIH('+'))&&indF==0) {
            if (o->size==0) {
                if (out->type==HalSMVariableType_HalSMNull||out->type==HalSMVariableType_void) {
                    //++halwarsing
                    isIncrementDecrement=1;
                    o=HalSMArray_init(hsmc->memory);
                } else {
                    HalSMVariable* temp=out;
                    out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
                    out->type=temp->type;
                    out->value=temp->value;
                    HalSMVariable* inc=HalSMCompiler_incrementVariable(hsmc,out);
                    temp->value=inc->value;
                    temp->type=inc->type;
                    isNotNull=1;
                    //halwarsing()++
                }
            } else {
                obuff=HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,o));
                //halwarsing.net++

                out=HalSMCompiler_isGetValue(hsmc,obuff,isOut,isNewVar,vars,&isNotNull,&module,out,U8H("++"));
                HalSMVariable* temp=out;
                out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
                out->type=temp->type;
                out->value=temp->value;
                HalSMVariable* inc=HalSMCompiler_incrementVariable(hsmc,out);
                temp->value=inc->value;
                temp->type=inc->type;
                isNotNull=1;
                o=HalSMArray_init(hsmc->memory);
            }
            ignore=1;
        } else if (HSFCharCompare(i,ASCIIH('-'))&&ii<li&&HSFCharCompare(l[ii+1],ASCIIH('-'))) {
            if (o->size==0) {
                if (out->type==HalSMVariableType_HalSMNull||out->type==HalSMVariableType_void) {
                    //--halwarsing
                    isIncrementDecrement=2;
                    o=HalSMArray_init(hsmc->memory);
                } else {
                    HalSMVariable* temp=out;
                    out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
                    out->type=temp->type;
                    out->value=temp->value;
                    HalSMVariable* inc=HalSMCompiler_decrementVariable(hsmc,out);
                    temp->value=inc->value;
                    temp->type=inc->type;
                    isNotNull=1;
                    //halwarsing()--
                }
            } else {
                obuff=HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,o));
                //halwarsing.net--

                out=HalSMCompiler_isGetValue(hsmc,obuff,isOut,isNewVar,vars,&isNotNull,&module,out,U8H("--"));
                HalSMVariable* temp=out;
                out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
                out->type=temp->type;
                out->value=temp->value;
                HalSMVariable* inc=HalSMCompiler_decrementVariable(hsmc,out);
                temp->value=inc->value;
                temp->type=inc->type;
                isNotNull=1;
                o=HalSMArray_init(hsmc->memory);
            }
            ignore=1;
        } else {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
        }
        index++;
    }

    if (o->size!=0) {
        obuff=HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,o));
        out=HalSMCompiler_isGetValue(hsmc,obuff,isOut,isNewVar,vars,&isNotNull,&module,out,U8H(""));
    }

    if (out->type!=HalSMVariableType_HalSMNull&&isIncrementDecrement==1) {
        HalSMVariable* inc=HalSMCompiler_incrementVariable(hsmc,out);
        out->value=inc->value;
        out->type=inc->type;
    } else if (out->type!=HalSMVariableType_HalSMNull&&isIncrementDecrement==2) {
        HalSMVariable* inc=HalSMCompiler_decrementVariable(hsmc,out);
        out->value=inc->value;
        out->type=inc->type;
    }


    if (isNotNull) {
        *isOut=1;
        return out;
    }
    *isOut=0;
    return &hvoid;
}

HalSMVariable* HalSMCompiler_isGetValue(HalSMCompiler* hsmc,HalSMVariable* obuff,ui8* isOut,ui8 isNewVar,Dict* vars,ui8* isNotNullOut,HalSMVariable** moduleOut,HalSMVariable* out,HalStringFormatChar* type)
{
    ui8 isNotNull=*isNotNullOut;
    HalSMVariable* vartemp,*module=*moduleOut;
    HalStringFormatChar* buffs=*(HalStringFormatChar**)obuff->value;
    if (string_len(buffs)==0){return out;}
    if (out->type==HalSMVariableType_HalSMNull||out->type==HalSMVariableType_void) {
        if (DictElementIndexByKey(hsmc,hsmc->variables,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,hsmc->variables,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,vars,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,vars,obuff)->value;
            isNotNull=1;
        } else if ((vartemp=hsmc->getVariable(buffs))->type!=HalSMVariableType_HalSMNull) {
            out=vartemp;
            isNotNull=1;
            vartemp=&null;
        } else if (isNewVar) {
            out=HalSMVariable_init(hsmc->memory,&hnull,HalSMVariableType_HalSMNull);
            if (vars->elements!=NOTHALSMNULLPOINTER) {
                PutDictElementToDict(hsmc,vars,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,buffs),out));
            } else {
                PutDictElementToDict(hsmc,hsmc->variables,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,buffs),out));
            }
            isNotNull=1;
        } else if (HSFCompare(buffs,U8H("true"))) {
            out=HalSMVariable_init_bool(hsmc->memory,1);
            isNotNull=1;
        } else if (HSFCompare(buffs,U8H("false"))) {
            out=HalSMVariable_init_bool(hsmc->memory,0);
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,hsmc->localFunctions,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,hsmc->modules,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,hsmc->modules,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,hsmc->classes,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,hsmc->classes,obuff)->value;
            isNotNull=1;
        } else if (HalSMIsInt(hsmc->memory,buffs)) {
            if (HSFCompare(type,U8H("."))) {
                out=HalSMVariable_init(hsmc->memory,HalSMDoubleGet_init(hsmc->memory,*(HalStringFormatChar**)obuff->value),HalSMVariableType_HalSMDoubleGet);
            } else {
                out=HalSMVariable_init(hsmc->memory,HalSMInteger_parse(hsmc->memory,buffs),HalSMVariableType_int);
            }
            isNotNull=1;
        } else if (HalSMIsDouble(hsmc->memory,buffs)) {
            out=HalSMVariable_init(hsmc->memory,HalSMDouble_parse(hsmc->memory,buffs),HalSMVariableType_double);
            isNotNull=1;
        } else if (HalSMIsHex(hsmc->memory,buffs)) {
            out=HalSMVariable_init(hsmc->memory,HalSMInteger_parseHex(hsmc->memory,buffs,1),HalSMVariableType_int);
            isNotNull=1;
        } else if (HalSMIsBin(hsmc->memory,buffs)) {
            out=HalSMVariable_init(hsmc->memory,HalSMInteger_parseBin(hsmc->memory,buffs),HalSMVariableType_int);
            isNotNull=1;
        } else if (HSFCompare(buffs,U8H("NULL"))||HSFCompare(buffs,U8H("null"))){
            out=&null;
            isNotNull=1;
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable Not Found")));
        }
    } else if (out->type==HalSMVariableType_HalSMModule) {
        if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->variables,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->variables,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->localFunctions,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->localFunctions,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->classes,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->classes,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->functions,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->functions,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->modules,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->modules,obuff)->value;
            isNotNull=1;
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class or Module Not Found")));
        }
    } else if (out->type==HalSMVariableType_HalSMCModule) {
        if (DictElementIndexByKey(hsmc,((HalSMCModule*)out->value)->vrs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMCModule*)out->value)->vrs,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMCModule*)out->value)->lfuncs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMCModule*)out->value)->lfuncs,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMCModule*)out->value)->classes,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMCModule*)out->value)->classes,obuff)->value;
            isNotNull=1;
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class Not Found")));
        }
    } else if (out->type==HalSMVariableType_HalSMRunClass) {
        if (DictElementIndexByKey(hsmc,((HalSMRunClass*)out->value)->vars,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMRunClass*)out->value)->vars,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMRunClass*)out->value)->funcs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMRunClass*)out->value)->funcs,obuff)->value;
            isNotNull=1;
        } else if (isNewVar) {
            module=out;
            out=HalSMVariable_init(hsmc->memory,&hnull,HalSMVariableType_HalSMNull);
            PutDictElementToDict(hsmc,((HalSMRunClass*)module->value)->vars,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,*(HalStringFormatChar**)obuff->value),out));
            isNotNull=1;
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class Not Found")));
        }
    } else if (out->type==HalSMVariableType_HalSMClass) {
        if (DictElementIndexByKey(hsmc,((HalSMClass*)out->value)->vars,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMClass*)out->value)->vars,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMClass*)out->value)->funcs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMClass*)out->value)->funcs,obuff)->value;
            isNotNull=1;
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class Not Found")));
        }
    } else if (out->type==HalSMVariableType_HalSMClassC) {
        if (DictElementIndexByKey(hsmc,((HalSMClassC*)out->value)->vrs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMClassC*)out->value)->vrs,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMClassC*)out->value)->funcs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMClassC*)out->value)->funcs,obuff)->value;
            isNotNull=1;
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class Not Found")));
        }
    } else if (out->type==HalSMVariableType_HalSMRunClassC) {
        if (DictElementIndexByKey(hsmc,((HalSMRunClassC*)out->value)->vrs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMRunClassC*)out->value)->vrs,obuff)->value;
            isNotNull=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMRunClassC*)out->value)->funcs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMRunClassC*)out->value)->funcs,obuff)->value;
            isNotNull=1;
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class Not Found")));
        }
    } else if (out->type==HalSMVariableType_HalSMDoubleGet) {
        if (HalSMIsInt(hsmc->memory,buffs)) {
            HalStringFormatChar* outfs=HalSMMemory_malloc(hsmc->memory,(string_len(((HalSMDoubleGet*)out->value)->st)+2+string_len(*(HalStringFormatChar**)obuff->value))*sizeof(HalStringFormatChar));
            string_cpy(outfs,((HalSMDoubleGet*)out->value)->st);
            string_cat(outfs,U8H("."));
            string_cat(outfs,*(HalStringFormatChar**)obuff->value);
            outfs[string_len(((HalSMDoubleGet*)out->value)->st)+1+string_len(*(HalStringFormatChar**)obuff->value)]=ASCIIH('\0');
            out=HalSMVariable_init(hsmc->memory,HalSMDouble_parse(hsmc->memory,outfs),HalSMVariableType_double);
            isNotNull=1;
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("This is not Double")));
        }
    } else if (out->type==HalSMVariableType_custom) {
        if (((HalSMVariableCustom*)out->value)->type->indexVariableByName(hsmc,out,*(HalStringFormatChar**)obuff->value)!=-1) {
            out=((HalSMVariableCustom*)out->value)->type->getVariableByName(hsmc,out,*(HalStringFormatChar**)obuff->value);
            isNotNull=1;
        } else if (((HalSMVariableCustom*)out->value)->type->indexFunctionByName(hsmc,out,*(HalStringFormatChar**)obuff->value)!=-1) {
            module=out;
            out=((HalSMVariableCustom*)out->value)->type->getFunctionByName(hsmc,out,*(HalStringFormatChar**)obuff->value);
            isNotNull=1;
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function Not Found")));
        }
    } else if (out->type==HalSMVariableType_HalSMDict) {
        if (DictElementIndexByKey(hsmc,(Dict*)out->value,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,(Dict*)out->value,obuff)->value;
            isNotNull=1;
        }
    } else {
        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Unknown Variable")));
    }
    *isNotNullOut=isNotNull;
    *moduleOut=module;
    return out;
}

HalSMVariable* HalSMCompiler_getArrayByIndex(HalSMCompiler* hsmc,HalStringFormatChar* obuffs,HalSMVariable* out,ui8* isNotNullOut,Dict* vars)
{
    ui8 isNotNull=*isNotNullOut;
    ui64 indexa;
    HalSMArray* argsbuff;
    /*HalSMArray* argsbuff,*n;
    n=HalSMArray_init(hsmc->memory);
    ui64 indexa,ll,li;
    ll=string_len(l);
    li=ll-1;
    ui64 isA=0,isF=0,isD=0,isIf=0;
    HalStringFormatChar isS=ASCIIH('n'),i,*obuffs;

    for (ui64 ind=0;ind<ll;ind++) {
        i=l[ind];
        
        if (isA) {
            if (HSFCharCompare(i,ASCIIH('['))) {
                isA++;
            } else if (HSFCharCompare(i,ASCIIH(']'))) {
                isA--;
            }
        } else if (isF) {
            if (HSFCharCompare(i,ASCIIH('('))) {
                isF++;
            } else if (HSFCharCompare(i,ASCIIH(')'))) {
                isF--;
            }

        } else if (HSFCharCompare(isS,ASCIIH('n'))==0) {
            if (HSFCharCompare(i,isS)){isS=ASCIIH('n');}
        } else if (isD) {
            if (HSFCharCompare(i,ASCIIH('{'))) {
                isD++;
            } else if (HSFCharCompare(i,ASCIIH('}'))) {
                isD--;
            }
        } else if (HSFCharCompare(i,ASCIIH('\''))||HSFCharCompare(i,ASCIIH('"'))) {
            isS=i;
        } else if (HSFCharCompare(i,ASCIIH('['))) {
            isA++;
        } else if (HSFCharCompare(i,ASCIIH('{'))) {
            isD++;
        } else if (HSFCharCompare(i,ASCIIH('('))) {
            isF++;
        } else if (HSFCharCompare(i,ASCIIH('?'))) {
            isIf++;
        } else if (isIf>0&&HSFCharCompare(i,ASCIIH(':'))) {
            isIf--;
        }
        if (ind==li||HSFCharCompare(i,ASCIIH(':'))) {
            obuffs=HalSMArray_chars_to_str(hsmc->memory,n);
            HalSMArray* argsindex=HalSMCompiler_getArgs(hsmc,obuffs,vars);
            for (ui64 indexspl=0;indexspl<argsindex->size;indexspl++) {
                if (out->type==HalSMVariableType_HalSMArray) {
                    if (argsindex->arr[indexspl]->type!=HalSMVariableType_int) {
                        HalSMCompiler_ThrowError(hsmc,hsmc->line,U8H("Value from array can only be by integer"));
                    }
                    indexa=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)argsindex->arr[indexspl]->value);
                    HalSMArray* rcls=(HalSMArray*)out->value;
                    if (indexa>=rcls->size) {
                        HalSMCompiler_ThrowError(hsmc,hsmc->line,U8H("Index is equal to or more than size of array"));
                    } else {
                        out=rcls->arr[indexa];
                        isNotNull=1;
                    }
                } else if (out->type==HalSMVariableType_HalSMRunClassC) {
                    HalSMRunClassC* rcls=(HalSMRunClassC*)out->value;
                    if (DictElementIndexByKey(hsmc,rcls->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__getitem__")))==-1) {
                        HalSMCompiler_ThrowError(hsmc,hsmc->line,U8H("This class does not contain function __getitem__"));
                    }
                    argsbuff=HalSMArray_init(hsmc->memory);
                    HalSMArray_add(hsmc->memory,argsbuff,HalSMVariable_init(hsmc->memory,rcls,HalSMVariableType_HalSMRunClassC));
                    HalSMArray_add(hsmc->memory,argsbuff,argsindex->arr[indexspl]);
                    out=(*(HalSMFunctionCTypeDef*)DictElementFindByKey(hsmc,rcls->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__getitem__")))->value->value)(hsmc,argsbuff,vars);
                    if (out->type==HalSMVariableType_HalSMError){
                        HalSMCompiler_ThrowError(hsmc,hsmc->line,((HalSMError*)out->value)->error);
                    }
                    isNotNull=1;
                } else if (out->type==HalSMVariableType_HalSMRunClass) {
                    HalSMRunClass* rcls=(HalSMRunClass*)out->value;
                    if (DictElementIndexByKey(hsmc,rcls->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__getitem__")))==-1) {
                        HalSMCompiler_ThrowError(hsmc,hsmc->line,U8H("This class does not contain function __getitem__"));
                    }
                    argsbuff=HalSMArray_init(hsmc->memory);
                    HalSMArray_add(hsmc->memory,argsbuff,HalSMVariable_init(hsmc->memory,rcls,HalSMVariableType_HalSMRunClass));
                    HalSMArray_add(hsmc->memory,argsbuff,argsindex->arr[indexspl]);
                    out=(*(HalSMFunctionCTypeDef*)DictElementFindByKey(hsmc,rcls->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__getitem__")))->value->value)(hsmc,argsbuff,vars);
                    if (out->type==HalSMVariableType_HalSMError){
                        HalSMCompiler_ThrowError(hsmc,hsmc->line,((HalSMError*)out->value)->error);
                    }
                    isNotNull=1;
                } else if (out->type==HalSMVariableType_custom) {
                    HalSMVariableCustom* hvc=(HalSMVariableCustom*)out->value;
                    out=hvc->type->getItem(hsmc,out,argsindex->arr[indexspl]);
                    isNotNull=1;
                } else if (out->type==HalSMVariableType_HalSMDict) {
                    Dict* rcls=(Dict*)out->value;
                    HalSMVariable* indexv=argsindex->arr[indexspl];
                    if (DictElementIndexByKey(hsmc,rcls,indexv)==-1) {
                        HalSMCompiler_ThrowError(hsmc,hsmc->line,U8H("Not Found Value By Key Dict"));
                    } else {
                        out=DictElementFindByKey(hsmc,rcls,indexv)->value;
                        isNotNull=1;
                    }
                } else {
                    HalSMCompiler_ThrowError(hsmc,hsmc->line,U8H("This object is not array"));
                }
            }
        }
        HalSMArray_add(hsmc->memory,n,HalSMVariable_init_char(hsmc->memory,i));
    }*/
    HalSMArray* argsindex=HalSMCompiler_getArgs(hsmc,obuffs,vars);
    for (ui64 indexspl=0;indexspl<argsindex->size;indexspl++) {
        if (out->type==HalSMVariableType_HalSMArray) {
            if (argsindex->arr[indexspl]->type!=HalSMVariableType_int) {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Value from array can only be by integer")));
            }
            indexa=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)argsindex->arr[indexspl]->value);
            HalSMArray* rcls=(HalSMArray*)out->value;
            if (indexa>=rcls->size) {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Index is equal to or more than size of array")));
            } else {
                out=rcls->arr[indexa];
                isNotNull=1;
            }
        } else if (out->type==HalSMVariableType_HalSMRunClassC) {
            HalSMRunClassC* rcls=(HalSMRunClassC*)out->value;
            if (DictElementIndexByKey(hsmc,rcls->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__getitem__")))==-1) {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("This class does not contain function __getitem__")));
            }
            argsbuff=HalSMArray_init(hsmc->memory);
            HalSMArray_add(hsmc->memory,argsbuff,HalSMVariable_init(hsmc->memory,rcls,HalSMVariableType_HalSMRunClassC));
            HalSMArray_add(hsmc->memory,argsbuff,argsindex->arr[indexspl]);
            out=(*(HalSMFunctionCTypeDef*)DictElementFindByKey(hsmc,rcls->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__getitem__")))->value->value)(hsmc,argsbuff,vars);
            if (out->type==HalSMVariableType_HalSMError){
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,((HalSMError*)out->value)->error));
            }
            isNotNull=1;
        } else if (out->type==HalSMVariableType_HalSMRunClass) {
            HalSMRunClass* rcls=(HalSMRunClass*)out->value;
            if (DictElementIndexByKey(hsmc,rcls->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__getitem__")))==-1) {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("This class does not contain function __getitem__")));
            }
            argsbuff=HalSMArray_init(hsmc->memory);
            HalSMArray_add(hsmc->memory,argsbuff,HalSMVariable_init(hsmc->memory,rcls,HalSMVariableType_HalSMRunClass));
            HalSMArray_add(hsmc->memory,argsbuff,argsindex->arr[indexspl]);
            out=(*(HalSMFunctionCTypeDef*)DictElementFindByKey(hsmc,rcls->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__getitem__")))->value->value)(hsmc,argsbuff,vars);
            if (out->type==HalSMVariableType_HalSMError){
                HalSMCompiler_ThrowError(hsmc,(HalSMError*)out->value);
            }
            isNotNull=1;
        } else if (out->type==HalSMVariableType_custom) {
            HalSMVariableCustom* hvc=(HalSMVariableCustom*)out->value;
            out=hvc->type->getItem(hsmc,out,argsindex->arr[indexspl]);
            isNotNull=1;
        } else if (out->type==HalSMVariableType_HalSMDict) {
            Dict* rcls=(Dict*)out->value;
            HalSMVariable* indexv=argsindex->arr[indexspl];
            if (DictElementIndexByKey(hsmc,rcls,indexv)==-1) {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Not Found Value By Key Dict")));
            } else {
                out=DictElementFindByKey(hsmc,rcls,indexv)->value;
                isNotNull=1;
            }
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("This object is not array")));
        }
    }
    *isNotNullOut=isNotNull;
    return out;
}

HalSMVariable* HalSMCompiler_additionVariables(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1)
{
    if (v0->type==HalSMVariableType_str||v1->type==HalSMVariableType_str) {
        return HalSMVariable_init_str(hsmc->memory,hsmc->calcVars.addStr(hsmc,v0,v1));
    } else if (v0->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v0->value)->type->add(hsmc,v0,v1);
    } else if (v1->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v1->value)->type->add(hsmc,v0,v1);
    } else if (v0->type==HalSMVariableType_double||v1->type==HalSMVariableType_double) {
        return HalSMVariable_init(hsmc->memory,hsmc->calcVars.addDouble(hsmc->memory,v0,v1),HalSMVariableType_double);
    } else if (v0->type==HalSMVariableType_int||v1->type==HalSMVariableType_int) {
        return HalSMVariable_init(hsmc->memory,hsmc->calcVars.addInt(hsmc->memory,v0,v1),HalSMVariableType_int);
    }
    return &null;
}

HalSMVariable* HalSMCompiler_subtractionVariables(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1)
{
    if (v0->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v0->value)->type->sub(hsmc,v0,v1);
    } else if (v1->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v1->value)->type->sub(hsmc,v0,v1);
    } else if ((v0->type==HalSMVariableType_double||v1->type==HalSMVariableType_double)&&v0->type!=HalSMVariableType_str&&v1->type!=HalSMVariableType_str) {
        return HalSMVariable_init(hsmc->memory,hsmc->calcVars.subDouble(hsmc->memory,v0,v1),HalSMVariableType_double);
    } else if (v0->type==HalSMVariableType_int) {
        if (v1->type==HalSMVariableType_str) {
            v1=HalSMVariable_init(hsmc->memory,HalSMInteger_parse(hsmc->memory,*(HalStringFormatChar**)v1->value),HalSMVariableType_int);
        }
        return HalSMVariable_init(hsmc->memory,hsmc->calcVars.subInt(hsmc->memory,v0,v1),HalSMVariableType_int);
    } else if (v1->type==HalSMVariableType_int) {
        if (v0->type==HalSMVariableType_str) {
            v0=HalSMVariable_init(hsmc->memory,HalSMInteger_parse(hsmc->memory,*(HalStringFormatChar**)v0->value),HalSMVariableType_int);
        }
        return HalSMVariable_init(hsmc->memory,hsmc->calcVars.subInt(hsmc->memory,v0,v1),HalSMVariableType_int);
    }
    return HalSMVariable_init_str(hsmc->memory,hsmc->calcVars.subStr(hsmc,v0,v1));
}

HalSMVariable* HalSMCompiler_multiplyVariables(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1)
{
    if (v0->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v0->value)->type->mul(hsmc,v0,v1);
    } else if (v1->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v1->value)->type->mul(hsmc,v0,v1);
    } else if (v0->type==HalSMVariableType_str&&v1->type==HalSMVariableType_str) {
        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("This is not possible to multiply a string by a string")));
    } else if (v0->type==HalSMVariableType_str||v1->type==HalSMVariableType_str) {
        return HalSMVariable_init_str(hsmc->memory,hsmc->calcVars.mulStr(hsmc->memory,v0,v1));
    } else if (v0->type==HalSMVariableType_double||v1->type==HalSMVariableType_double) {
        return HalSMVariable_init(hsmc->memory,hsmc->calcVars.mulDouble(hsmc->memory,v0,v1),HalSMVariableType_double);
    } else if (v0->type==HalSMVariableType_int||v1->type==HalSMVariableType_int) {
        return HalSMVariable_init(hsmc->memory,hsmc->calcVars.mulInt(hsmc->memory,v0,v1),HalSMVariableType_int);
    }
    return &null;
}

HalSMVariable* HalSMCompiler_divideVariables(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1)
{
    if (v0->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v0->value)->type->div(hsmc,v0,v1);
    } else if (v1->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v1->value)->type->div(hsmc,v0,v1);
    } else if (v0->type==HalSMVariableType_str&&v1->type==HalSMVariableType_str) {
        return HalSMVariable_init_str(hsmc->memory,StringReplace(hsmc->memory,*(HalStringFormatChar**)v0->value,*(HalStringFormatChar**)v1->value,U8H("")));
    } else if (v0->type==HalSMVariableType_str||v1->type==HalSMVariableType_str) {
        return HalSMVariable_init_str(hsmc->memory,hsmc->calcVars.divStr(hsmc,v0,v1));
    } else if (v0->type==HalSMVariableType_double||v1->type==HalSMVariableType_double) {
        return HalSMVariable_init(hsmc->memory,hsmc->calcVars.divDouble(hsmc->memory,v0,v1),HalSMVariableType_double);
    } else if (v0->type==HalSMVariableType_int||v1->type==HalSMVariableType_int) {
        return HalSMVariable_init(hsmc->memory,hsmc->calcVars.divInt(hsmc->memory,v0,v1),HalSMVariableType_int);
    }
    return &null;
}

HalSMVariable* HalSMCompiler_incrementVariable(HalSMCompiler* hsmc,HalSMVariable* v)
{
    if (v->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v->value)->type->increment(hsmc,v);
    } else if (v->type==HalSMVariableType_int) {
        return HalSMVariable_init_int(hsmc->memory,HalSMInteger_Add(hsmc->memory,(HalSMInteger*)v->value,&HalSMIntegerOne));
    } else if (v->type==HalSMVariableType_double) {
        return HalSMVariable_init_double(hsmc->memory,HalSMDouble_Add(hsmc->memory,(HalSMDouble*)v->value,&HalSMDoubleOne));
    } else if (v->type==HalSMVariableType_str) {
        return HalSMVariable_init_str(hsmc->memory,hsmc->calcVars.addStr(hsmc,v,v));
    }
    return &null;
}

HalSMVariable* HalSMCompiler_decrementVariable(HalSMCompiler* hsmc,HalSMVariable* v)
{
    if (v->type==HalSMVariableType_custom) {
        return ((HalSMVariableCustom*)v->value)->type->decrement(hsmc,v);
    } else if (v->type==HalSMVariableType_int) {
        return HalSMVariable_init_int(hsmc->memory,HalSMInteger_Sub(hsmc->memory,(HalSMInteger*)v->value,&HalSMIntegerOne));
    } else if (v->type==HalSMVariableType_double) {
        return HalSMVariable_init_double(hsmc->memory,HalSMDouble_Sub(hsmc->memory,(HalSMDouble*)v->value,&HalSMDoubleOne));
    } else if (v->type==HalSMVariableType_str) {
        return HalSMVariable_init_str(hsmc->memory,hsmc->calcVars.subStr(hsmc,v,v));
    }
    return &null;
}

HalSMVariable* HalSMCompiler_increment(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2) {
        if (args->arr[0]->type==HalSMVariableType_HalSMVar) {
            HalSMVariable* vartemp=&null;
            if (DictElementIndexByKey(hsmc,hsmc->variables,((HalSMVar*)args->arr[0]->value)->name)!=-1) {
                args->arr[0]=DictElementFindByKey(hsmc,hsmc->variables,((HalSMVar*)args->arr[0]->value)->name)->value;
            } else if ((vartemp=hsmc->getVariable(*(HalStringFormatChar**)((HalSMVar*)args->arr[0]->value)->name->value))->type!=HalSMVariableType_HalSMNull) {
                args->arr[0]=vartemp;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable Not Found")));
            }
        }

        HalSMVariable* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
        out->value=args->arr[0]->value;
        out->type=args->arr[0]->type;
        HalSMVariable* res=HalSMCompiler_incrementVariable(hsmc,args->arr[0]);
        args->arr[0]->value=res->value;
        args->arr[0]->type=res->type;
        if ((*(ui8*)args->arr[1]->value)==1) {
            return args->arr[0];
        }
        return out;
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_decrement(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[1]->type==HalSMVariableType_HalSMBool) {
        if (args->arr[0]->type==HalSMVariableType_HalSMVar) {
            HalSMVariable* vartemp=&null;
            if (DictElementIndexByKey(hsmc,hsmc->variables,((HalSMVar*)args->arr[0]->value)->name)!=-1) {
                args->arr[0]=DictElementFindByKey(hsmc,hsmc->variables,((HalSMVar*)args->arr[0]->value)->name)->value;
            } else if ((vartemp=hsmc->getVariable(*(HalStringFormatChar**)((HalSMVar*)args->arr[0]->value)->name->value))->type!=HalSMVariableType_HalSMNull) {
                args->arr[0]=vartemp;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable Not Found")));
            }
        }
        
        HalSMVariable* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
        out->value=args->arr[0]->value;
        out->type=args->arr[0]->type;
        HalSMVariable* res=HalSMCompiler_decrementVariable(hsmc,args->arr[0]);
        args->arr[0]->value=res->value;
        args->arr[0]->type=res->type;
        if ((*(ui8*)args->arr[1]->value)==1) {
            return args->arr[0];
        }
        return out;
    }
    return &hvoid;
}

void HalSMCompiler_getArgsSetVar(HalSMCompiler* hsmc,HalSMArray* value,HalSMVariable* pointer,Dict* vars)
{
    HalSMArray* out=HalSMArray_copy(hsmc->memory,value);
    //HalSMArray* out=value;
    i64 lout=out->size;
    i64 ou=0;

    while (ou<lout) {
        if (HalSMArray_get(out,ou)->type==HalSMVariableType_HalSMVar) {
            ui8 isOut;
            HalSMArray_set(out,HalSMCompiler_isGetU(hsmc,*(HalStringFormatChar**)((HalSMVar*)HalSMArray_get(out,ou)->value)->name->value,&isOut,0,vars),ou);
        }
        ou++;
    }

    HalSMCompiler_executeArgs(hsmc,&out,vars);
    pointer->type=out->arr[0]->type;
    pointer->value=out->arr[0]->value;
}

void HalSMCompiler_getArgsFunction(HalSMCompiler* hsmc,HalSMArray* value,HalSMArray* out,Dict* vars) {
    out->size=value->size;
    out->arr=HalSMMemory_realloc(hsmc->memory,out->arr,sizeof(DictElement*)*value->size);
    for (ui64 i=0;i<value->size;i++)
        out->arr[i]=value->arr[i];
    i64 lout=out->size;
    i64 ou=0;

    while (ou<lout) {
        if (HalSMArray_get(out,ou)->type==HalSMVariableType_HalSMVar) {
            //HalSMArray_set(out,DictElementFindByKey(hsmc,hsmc->variables,((HalSMVar*)HalSMArray_get(out,ou)->value)->name)->value,ou);
            ui8 isOut;
            HalSMArray_set(out,HalSMCompiler_isGetU(hsmc,*(HalStringFormatChar**)((HalSMVar*)HalSMArray_get(out,ou)->value)->name->value,&isOut,0,vars),ou);
        }
        ou++;
    }

    HalSMCompiler_executeArgs(hsmc,&out,vars);
}

Dict* HalSMCompiler_getArgsDict(HalSMCompiler* hsmc,HalStringFormatChar* l,Dict* vars) {
    HalStringFormatChar isS=ASCIIH('n');
    ui64 isD=0;
    ui64 isA=0;
    ui64 ll=string_len(l);
    HalStringFormatChar i;
    HalStringFormatChar* buffs;
    HalSMArray* o=HalSMArray_init(hsmc->memory);
    Dict* out=DictInit(hsmc->memory);
    ui8 isKey=0;
    HalSMVariable* key=&null;
    ui64 li=ll-1;
    ui64 ignore=0;
    for (ui64 indexi=0;indexi<ll;indexi++) {
        i=l[indexi];
        buffs=HalSMArray_chars_to_str(hsmc->memory,o);
        if (ignore>0) {
            ignore--;
            continue;
        }

        if (HSFCharCompare(isS,ASCIIH('n'))==0) {
            if (HSFCharCompare(i,isS)) {
                isS=ASCIIH('n');
            }
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
        } else if (isD>0) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            if (HSFCharCompare(i,ASCIIH('}'))) {
                isD--;
            } else if (HSFCharCompare(i,ASCIIH('{'))) {
                isD++;
            }
        } else if (isA>0) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            if (HSFCharCompare(i,ASCIIH(']'))) {
                isA--;
            } else if (HSFCharCompare(i,ASCIIH('['))) {
                isA++;
            }
        } else if (HSFCharCompare(i,ASCIIH('\\'))) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            if (indexi<li) {
                HalStringFormatChar ichr=l[indexi+1];
                ignore=1;
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ichr));
            }
        } else if (HSFCharCompare(i,ASCIIH('"'))||HSFCharCompare(i,ASCIIH('\''))) {
            isS=i;
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
        } else if (HSFCharCompare(i,ASCIIH('{'))) {
            isD++;
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
        } else if (HSFCharCompare(i,ASCIIH('['))) {
            isA++;
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
        } else if (HSFCharCompare(i,ASCIIH(':'))) {
            if (o->size>0) {
                if (isKey) {
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                } else {
                    key=HalSMCompiler_getArgs(hsmc,buffs,vars)->arr[0];
                    isKey=1;
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                }
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Zero Key Dict")));
                return &NULLDict;
            }
        } else if (HSFCharCompare(i,ASCIIH(','))) {
            if (isKey) {
                if (o->size>0) {
                    PutDictElementToDict(hsmc,out,DictElementInit(hsmc->memory,key,HalSMCompiler_getArgs(hsmc,buffs,vars)->arr[0]));
                    key=&null;
                    isKey=0;
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                } else {
                    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Zero Value Dict")));
                    return &NULLDict;
                }
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Zero Key Dict")));
                return &NULLDict;
            }
        } else if (HSFCharCompare(i,ASCIIH(' '))||HSFCharCompare(i,ASCIIH('\n'))||HSFCharCompare(i,ASCIIH('\t'))||HSFCharCompare(i,ASCIIH('\r'))) {
            
        } else {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
        }
    }
    if (o->size>0) {
        if (isKey) {
            buffs=HalSMArray_chars_to_str(hsmc->memory,o);
            PutDictElementToDict(hsmc,out,DictElementInit(hsmc->memory,key,HalSMCompiler_getArgs(hsmc,buffs,vars)->arr[0]));
            key=&null;
            isKey=0;
            HalSMArray_free(hsmc->memory,o);
        } else {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Zero Value Dict")));
            return &NULLDict;
        }
    }
    return out;
}

void HalSMCompiler_executeArgs(HalSMCompiler* hsmc,HalSMArray** outArr,Dict* vars)
{
    HalSMArray* out=*outArr;
    ui64 skip=0;
    ui64 lout=out->size;
    ui64 ou=0;
    HalSMVariable* iv;
    HalSMVariable* v;
    HalSMArray* temp;
    HalSMFunctionArray* tempf;

    while (ou<lout) {
        if (skip>0) {skip--;continue;}

        iv=HalSMArray_get(out,ou);

        if (iv->type==HalSMVariableType_HalSMFunctionArray) {
            tempf=(HalSMFunctionArray*)iv->value;
            temp=tempf->args;
            if (tempf->type==HalSMFunctionArrayType_function) {
                ui8 isOut;
                //Question
                v=HalSMCompiler_isGetU(hsmc,*(HalStringFormatChar**)temp->arr[0]->value,&isOut,0,vars);
                if (isOut==1) {
                    HalSMArray_set(out,v,ou);
                } else {
                    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Function Not Found")));
                }
            } else if (tempf->type==HalSMFunctionArrayType_array) {
                HalSMArray_set(out,HalSMVariable_init(hsmc->memory,HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)HalSMArray_get(temp,0)->value,vars),HalSMVariableType_HalSMArray),ou);
            } else if (tempf->type==HalSMFunctionArrayType_dict) {
                HalSMArray_set(out,HalSMVariable_init(hsmc->memory,HalSMCompiler_getArgsDict(hsmc,*(HalStringFormatChar**)HalSMArray_get(temp,0)->value,vars),HalSMVariableType_HalSMDict),ou);
            }
        } else if (iv->type==HalSMVariableType_HalSMMult) {
            HalSMArray_set(out,HalSMCompiler_multiplyVariables(hsmc,HalSMArray_get(out,ou-1),HalSMArray_get(out,ou+1)),ou-1);
            temp=HalSMArray_slice(hsmc->memory,out,0,ou);
            HalSMArray_appendArray(hsmc->memory,temp,HalSMArray_slice(hsmc->memory,out,ou+2,out->size));
            out=temp;
            ou--;
            lout-=2;
            skip=1;
        } else if (iv->type==HalSMVariableType_HalSMDivide) {
            HalSMArray_set(out,HalSMCompiler_divideVariables(hsmc,HalSMArray_get(out,ou-1),HalSMArray_get(out,ou+1)),ou-1);
            temp=HalSMArray_slice(hsmc->memory,out,0,ou);
            HalSMArray_appendArray(hsmc->memory,temp,HalSMArray_slice(hsmc->memory,out,ou+2,out->size));
            out=temp;
            ou--;
            lout-=2;
            skip=1;
        } else if (iv->type==HalSMVariableType_HalSMShift) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else if ((iv->type==HalSMVariableType_int||iv->type==HalSMVariableType_custom)&&(v->type==HalSMVariableType_int||v->type==HalSMVariableType_custom)) {
                HalSMArray_set(out,HalSMCompiler_shift(hsmc,iv,v,(HalSMShift*)HalSMArray_get(out,ou)->value),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                if (((HalSMShift*)HalSMArray_get(out,ou)->value)->c==0) {
                    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Shift Back (<<) cannot without numeric args")));
                } else {
                    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Shift Forward (>>) cannot without numeric args")));
                }
            }
        }
        ou++;
    }

    skip=0;
    lout=out->size;
    ou=0;

    while (ou<lout) {
        if (skip>0){skip--;continue;}

        iv=HalSMArray_get(out,ou);

        if (iv->type==HalSMVariableType_HalSMPlus) {
            HalSMArray_set(out,HalSMCompiler_additionVariables(hsmc,HalSMArray_get(out,ou-1),HalSMArray_get(out,ou+1)),ou-1);
            temp=HalSMArray_slice(hsmc->memory,out,0,ou);
            HalSMArray_appendArray(hsmc->memory,temp,HalSMArray_slice(hsmc->memory,out,ou+2,out->size));
            out=temp;
            ou--;
            lout-=2;
            skip=1;
        } else if (iv->type==HalSMVariableType_HalSMMinus) {
            HalSMArray_set(out,HalSMCompiler_subtractionVariables(hsmc,HalSMArray_get(out,ou-1),HalSMArray_get(out,ou+1)),ou-1);
            temp=HalSMArray_slice(hsmc->memory,out,0,ou);
            HalSMArray_appendArray(hsmc->memory,temp,HalSMArray_slice(hsmc->memory,out,ou+2,out->size));
            out=temp;
            ou--;
            lout-=2;
            skip=1;
        } else if (iv->type==HalSMVariableType_HalSMNegativeSign) {
            v=HalSMArray_get(out,ou+1);
            if (v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else {
                HalSMArray_set(out,HalSMCompiler_negate(hsmc,v),ou);
                HalSMArray_remove(hsmc->memory,out,ou+1);
                lout-=1;
            }
        }
        ou++;
    }

    skip=0;
    lout=out->size;
    ou=0;

    while (ou<lout) {
        if (skip>0){skip--;continue;}
        iv=HalSMArray_get(out,ou);

        if (iv->type==HalSMVariableType_HalSMEqual) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar){skip=1;}
            else {
                HalSMArray_set(out,HalSMVariable_init_bool(hsmc->memory,HalSMVariable_Compare(hsmc,iv,v)),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            }
        } else if (iv->type==HalSMVariableType_HalSMNotEqual) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar){skip=1;}
            else {
                HalSMArray_set(out,HalSMVariable_init_bool(hsmc->memory,HalSMVariable_Compare(hsmc,iv,v)==0),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            }
        } else if (iv->type==HalSMVariableType_HalSMMore) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar){skip=1;}
            else if ((iv->type==HalSMVariableType_int||iv->type==HalSMVariableType_double||iv->type==HalSMVariableType_custom)&&(v->type==HalSMVariableType_int||v->type==HalSMVariableType_double||v->type==HalSMVariableType_custom)) {
                HalSMArray_set(out,HalSMVariable_init_bool(hsmc->memory,HalSMCompiler_isMore(hsmc,iv,v)),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("More (>) cannot without numeric args")));
            }
        } else if (iv->type==HalSMVariableType_HalSMLess) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar){skip=1;}
            else if ((iv->type==HalSMVariableType_int||iv->type==HalSMVariableType_double||v->type==HalSMVariableType_custom)&&(v->type==HalSMVariableType_int||v->type==HalSMVariableType_double||v->type==HalSMVariableType_custom)) {
                HalSMArray_set(out,HalSMVariable_init_bool(hsmc->memory,HalSMCompiler_isLess(hsmc,iv,v)),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Less (<) cannot without numeric args")));
            }
        } else if (iv->type==HalSMVariableType_HalSMMoreEqual) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else if ((iv->type==HalSMVariableType_int||iv->type==HalSMVariableType_double||iv->type==HalSMVariableType_custom)&&(v->type==HalSMVariableType_int||v->type==HalSMVariableType_double||v->type==HalSMVariableType_custom)) {
                HalSMArray_set(out,HalSMVariable_init_bool(hsmc->memory,HalSMCompiler_isMore(hsmc,iv,v)||HalSMVariable_Compare(hsmc,iv,v)),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("More Equal (>=) cannot without numeric args")));
            }
        } else if (iv->type==HalSMVariableType_HalSMLessEqual) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else if ((iv->type==HalSMVariableType_int||iv->type==HalSMVariableType_double||iv->type==HalSMVariableType_custom)&&(v->type==HalSMVariableType_int||v->type==HalSMVariableType_double||v->type==HalSMVariableType_custom)) {
                HalSMArray_set(out,HalSMVariable_init_bool(hsmc->memory,HalSMCompiler_isLess(hsmc,iv,v)||HalSMVariable_Compare(hsmc,iv,v)),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Less Equal (<=) cannot without numeric args")));
            }
        } else if (iv->type==HalSMVariableType_HalSMAND) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else {
                HalSMArray_set(out,HalSMVariable_init_bool(hsmc->memory,HalSMCompiler_getBool(hsmc,iv)&&HalSMCompiler_getBool(hsmc,v)),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            }
        } else if (iv->type==HalSMVariableType_HalSMOR) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else {
                HalSMArray_set(out,HalSMVariable_init_bool(hsmc->memory,HalSMCompiler_getBool(hsmc,iv)||HalSMCompiler_getBool(hsmc,v)),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            }
        } else if (iv->type==HalSMVariableType_HalSMNegate) {
            v=HalSMArray_get(out,ou+1);
            if (v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else {
                HalSMArray_set(out,HalSMVariable_init_bool(hsmc->memory,HalSMCompiler_getBool(hsmc,v)==0),ou);
                HalSMArray_remove(hsmc->memory,out,ou+1);
                lout-=1;
            }
        } else if (iv->type==HalSMVariableType_HalSMInlineIf) {
            if (ou>0&&ou+3<lout&&HalSMArray_get(out,ou+2)->type==HalSMVariableType_HalSMInlineElse) {
                v=HalSMArray_get(out,ou-1);
                if (v->type==HalSMVariableType_HalSMVar) {
                    skip=1;
                } else {
                    if (HalSMCompiler_getBool(hsmc,v)) {
                        HalSMArray_set(out,HalSMArray_get(out,ou+1),ou-1);
                    } else {
                        HalSMArray_set(out,HalSMArray_get(out,ou+3),ou-1);
                    }
                    HalSMArray_remove(hsmc->memory,out,ou);
                    HalSMArray_remove(hsmc->memory,out,ou);
                    HalSMArray_remove(hsmc->memory,out,ou);
                    HalSMArray_remove(hsmc->memory,out,ou);
                    lout-=4;
                    ou--;
                }
            }
        } else if (iv->type==HalSMVariableType_HalSMSetArg) {
            if (ou<lout-1) {
                /*ui8 isGet=0;
                HalSMVariable* var=HalSMCompiler_isGetU(hsmc,*(HalStringFormatChar**)((HalSMSetArg*)iv->value)->name->value,&isGet,1,vars);
                if (isGet==1) {
                    var->value=out->arr[ou+1]->value;
                    var->type=out->arr[ou+1]->type;
                    out->arr[ou]=var;
                } else {
                    out->arr[ou]=&hvoid;
                }*/
                ((HalSMSetArg*)iv->value)->value=out->arr[ou+1];
                HalSMArray_remove(hsmc->memory,out,ou+1);
            }
            lout--;
        } else if (iv->type==HalSMVariableType_HalSMModulo) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else if ((iv->type==HalSMVariableType_int||iv->type==HalSMVariableType_custom)&&(v->type==HalSMVariableType_int||v->type==HalSMVariableType_custom)) {
                HalSMArray_set(out,HalSMCompiler_modulo(hsmc,iv,v),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Modulo (%) cannot without numeric args")));
            }
        }
        ou++;
    }
    

    skip=0;
    lout=out->size;
    ou=0;

    while (ou<lout) {
        if (skip>0){skip--;continue;}
        iv=HalSMArray_get(out,ou);

        if (iv->type==HalSMVariableType_HalSMPow) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else if ((iv->type==HalSMVariableType_int||iv->type==HalSMVariableType_custom||iv->type==HalSMVariableType_double)&&(v->type==HalSMVariableType_int||v->type==HalSMVariableType_custom)) {
                HalSMArray_set(out,HalSMCompiler_pow(hsmc,iv,v),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Pow (**) cannot without numeric args")));
            }
        } else if (iv->type==HalSMVariableType_HalSMXOR) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else if (iv->type==HalSMVariableType_int&&iv->type==HalSMVariableType_int) {
                HalSMArray_set(out,HalSMVariable_init(hsmc->memory,HalSMInteger_BitXOR(hsmc->memory,(HalSMInteger*)iv->value,(HalSMInteger*)v->value),HalSMVariableType_int),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("XOR (^) cannot without numeric args")));
            }
        } else if (iv->type==HalSMVariableType_HalSMBAND) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else if (iv->type==HalSMVariableType_int&&iv->type==HalSMVariableType_int) {
                HalSMArray_set(out,HalSMVariable_init(hsmc->memory,HalSMInteger_BitAND(hsmc->memory,(HalSMInteger*)iv->value,(HalSMInteger*)v->value),HalSMVariableType_int),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("BIT AND (&) cannot without numeric args")));
            }
        } else if (iv->type==HalSMVariableType_HalSMBOR) {
            iv=HalSMArray_get(out,ou-1);
            v=HalSMArray_get(out,ou+1);
            if (iv->type==HalSMVariableType_HalSMVar||v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else if (iv->type==HalSMVariableType_int&&iv->type==HalSMVariableType_int) {
                HalSMArray_set(out,HalSMVariable_init(hsmc->memory,HalSMInteger_BitOR(hsmc->memory,(HalSMInteger*)iv->value,(HalSMInteger*)v->value),HalSMVariableType_int),ou-1);
                HalSMArray_remove(hsmc->memory,out,ou);
                HalSMArray_remove(hsmc->memory,out,ou);
                lout-=2;
                ou--;
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("BIT OR (|) cannot without numeric args")));
            }
        } else if (iv->type==HalSMVariableType_HalSMBNOT) {
            v=HalSMArray_get(out,ou+1);
            if (v->type==HalSMVariableType_HalSMVar) {
                skip=1;
            } else {
                HalSMArray_set(out,HalSMVariable_init(hsmc->memory,HalSMInteger_BitNOT(hsmc->memory,(HalSMInteger*)v->value),HalSMVariableType_int),ou);
                HalSMArray_remove(hsmc->memory,out,ou+1);
                lout-=1;
            }
        }
        ou++;
    }
    *outArr=out;
}

HalSMArray* HalSMCompiler_getArgs(HalSMCompiler* hsmc,HalStringFormatChar* l,Dict* vars)
{
    HalSMArray* outarr=HalSMArray_init(hsmc->memory);
    HalSMArray* out=HalSMArray_init(hsmc->memory);
    HalStringFormatChar isS=ASCIIH('n');
    ui64 isF=0;
    HalSMArray* o=HalSMArray_init(hsmc->memory);
    ui64 isA=0;
    ui64 isAG=0;
    ui64 isD=0;
    ui64 ll=string_len(l);
    ui64 lind=ll-1;
    ui64 isNArgs=0;
    ui64 ignore=0;
    HalStringFormatChar i;
    HalSMVariable* buffo;
    HalStringFormatChar* buffs;
    ui64 skip=0;

    ui64 li=ll-1;

    for (ui64 indexi=0;indexi<ll;indexi++) {
        i=l[indexi];
        if (ignore>0) {
            ignore--;
            continue;
        }

        buffs=HalSMArray_chars_to_str(hsmc->memory,o);
        buffo=HalSMVariable_init_str(hsmc->memory,buffs);

        if (isNArgs>0&&HSFCharCompare(i,ASCIIH('('))) {
            isNArgs++;
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('(')));
        } else if (isNArgs>0&&HSFCharCompare(i,ASCIIH(')'))) {
            isNArgs--;
            if (isNArgs==0) {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(')')));
            } else {HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(')')));}
        } else if (isNArgs>0) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
        } else if (isA>0) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            if (HSFCharCompare(i,ASCIIH(']'))) {
                isA--;
            } else if (HSFCharCompare(i,ASCIIH('['))) {isA++;}
        } else if (isAG>0) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            if (HSFCharCompare(i,ASCIIH(']'))) {
                isAG--;
            } else if (HSFCharCompare(i,ASCIIH('['))) {isAG++;}
        } else if (isD>0) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            if (HSFCharCompare(i,ASCIIH('}'))) {
                isD--;
            } else if (HSFCharCompare(i,ASCIIH('{'))) {
                isD++;
            }
        } else if (HSFCharCompare(isS,ASCIIH('n'))==0) {
            if (HSFCharCompare(i,isS)) {
                isS=ASCIIH('n');
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            } else {HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));}
        } else if (HSFCharCompare(i,ASCIIH('\\'))) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            if (indexi<li) {
                HalStringFormatChar ichr=l[indexi+1];
                ignore=1;
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ichr));
            }
        } else if (HSFCharCompare(i,ASCIIH('['))) {
            if (o->size==0) {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                isA++;
            } else {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                isAG++;
            }
        } else if (HSFCharCompare(i,ASCIIH('{'))) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            isD++;
        } else if (isF>0) {
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            buffs=HalSMArray_chars_to_str(hsmc->memory,o);
            if (HSFCharCompare(i,ASCIIH(')'))) {
                isF--;
            } else if (HSFCharCompare(i,ASCIIH('('))) {isF++;}
        } else if (HSFCharCompare(i,ASCIIH('"'))||HSFCharCompare(i,ASCIIH('\''))) {
            isS=i;
            HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
        } else if (HSFCharCompare(i,ASCIIH('('))) {
            if (isF==0) {
                if (o->size==0) {isNArgs++;HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('(')));}
                else {
                    isF++;
                    HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                }
            } else {
                isF++;
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (HSFCharCompare(i,ASCIIH(' '))||HSFCharCompare(i,ASCIIH('\n'))) {

        } else if (HSFCharCompare(i,ASCIIH(','))) {
            if (o->size!=0) {
                if (StringIndexOf(hsmc->memory,buffs,U8H("["))!=-1&&StringIndexOf(hsmc->memory,buffs,U8H("]"))==-1) {HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH(',')));}
                else {
                    HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                }
            }

            HalSMCompiler_executeArgs(hsmc,&out,vars);
            HalSMArray_appendArray(hsmc->memory,outarr,out);
            out=HalSMArray_init(hsmc->memory);
        } else if (HSFCharCompare(i,ASCIIH('+'))) {
            if (o->size!=0&&indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('+'))==0&&HSFCharCompare(*(HalStringFormatChar*)o->arr[0]->value,ASCIIH('+'))==0&&HSFCharCompare(*(HalStringFormatChar*)o->arr[o->size-1]->value,ASCIIH('+'))==0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&plus,HalSMVariableType_HalSMPlus));
            } else if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('+'))) {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                ignore=1;
            } else {
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&plus,HalSMVariableType_HalSMPlus));
            }
        } else if (HSFCharCompare(i,ASCIIH('-'))) {
            if (o->size!=0&&indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('-'))==0&&HSFCharCompare(*(HalStringFormatChar*)o->arr[0]->value,ASCIIH('-'))==0&&HSFCharCompare(*(HalStringFormatChar*)o->arr[o->size-1]->value,ASCIIH('-'))==0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&minus,HalSMVariableType_HalSMMinus));
            } else if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('-'))) {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
                ignore=1;
            } else if (o->size==0&&indexi<lind) {
                HalSMArray_add(hsmc->memory,out,&vnegativesign);
            } else {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (HSFCharCompare(i,ASCIIH('*'))) {
            if (o->size!=0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
                if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('*'))) {
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&hpow,HalSMVariableType_HalSMPow));
                    ignore=1;
                } else {
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&mult,HalSMVariableType_HalSMMult));
                }
            } else {
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&mult,HalSMVariableType_HalSMMult));
            }
        } else if (HSFCharCompare(i,ASCIIH('/'))) {
            if (o->size!=0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            }
            HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&divide,HalSMVariableType_HalSMDivide));
        } else if (HSFCharCompare(i,ASCIIH('='))) {
            if (o->size>0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('='))){ignore=1;}
                    continue;
                }
                if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('='))) {
                    HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&equal,HalSMVariableType_HalSMEqual));
                    ignore=1;
                } else {
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMSetArg_init(hsmc->memory,HalSMVariable_init_str(hsmc->memory,buffs)),HalSMVariableType_HalSMSetArg));
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                }
            } else {HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,ASCIIH('=')));}
        } else if (HSFCharCompare(i,ASCIIH('!'))) {
            if (o->size>0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('='))){ignore=1;}
                    continue;
                }
                if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('='))) {
                    HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&notequal,HalSMVariableType_HalSMNotEqual));
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    ignore=1;
                }
            } else {
                HalSMArray_add(hsmc->memory,out,&vnegate);
            }
        } else if (HSFCharCompare(i,ASCIIH('>'))) {
            if (o->size!=0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    if (indexi<lind&&(HSFCharCompare(l[indexi+1],ASCIIH('='))||HSFCharCompare(l[indexi+1],ASCIIH('>')))) {
                        ignore=1;
                    }
                    continue;
                }
                if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('='))) {
                    HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&moreequal,HalSMVariableType_HalSMMoreEqual));
                    ignore=1;
                } else if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('>'))) {
                    HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&shift_right,HalSMVariableType_HalSMShift));
                    ignore=1;
                } else {
                    HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&more,HalSMVariableType_HalSMMore));
                }
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("More (>) cannot without numeric args")));
            }
        } else if (HSFCharCompare(i,ASCIIH('<'))) {
            if (o->size!=0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    if (indexi<lind&&(HSFCharCompare(l[indexi+1],ASCIIH('='))||HSFCharCompare(l[indexi+1],ASCIIH('<')))) {
                        ignore=1;
                    }
                    continue;
                }
                if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('='))) {
                    HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&lessequal,HalSMVariableType_HalSMLessEqual));
                    ignore=1;
                } else if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('<'))) {
                    HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&shift_left,HalSMVariableType_HalSMShift));
                    ignore=1;
                } else {
                    HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&less,HalSMVariableType_HalSMLess));
                }
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Less (<) cannot without numeric args")));
            }
        } else if (HSFCharCompare(i,ASCIIH('^'))) {
            if (o->size!=0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&hxor,HalSMVariableType_HalSMXOR));
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("XOR (^) cannot without numeric args")));
            }
        } else if (HSFCharCompare(i,ASCIIH('%'))) {
            if (o->size!=0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&modulo,HalSMVariableType_HalSMModulo));
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Modulo (%) cannot without numeric args")));
            }
        } else if (HSFCharCompare(i,ASCIIH('&'))) {
            if (o->size>0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('&'))){ignore=1;}
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
                if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('&'))) {
                    out->arr[out->size-1]=HalSMVariable_init_bool(hsmc->memory,HalSMCompiler_getBool(hsmc,out->arr[out->size-1]));
                    if ((*(ui8*)out->arr[out->size-1]->value)==0) {
                        skip++;
                    } else {
                        HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&and,HalSMVariableType_HalSMAND));
                    }
                    ignore=1;
                } else {
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&hband,HalSMVariableType_HalSMBAND));
                }
            } else {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (HSFCharCompare(i,ASCIIH('|'))) {
            if (o->size>0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('|'))){ignore=1;}
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
                if (indexi<lind&&HSFCharCompare(l[indexi+1],ASCIIH('|'))) {
                    HalSMVariable* temppointer=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
                    HalSMCompiler_getArgsSetVar(hsmc,out,temppointer,vars);
                    out=HalSMArray_init(hsmc->memory);
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init_bool(hsmc->memory,HalSMCompiler_getBool(hsmc,temppointer)));
                    if ((*(ui8*)out->arr[0]->value)==1) {
                        skip++;
                    } else {
                        HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&or,HalSMVariableType_HalSMOR));
                    }
                    ignore=1;
                } else {
                    HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&hbor,HalSMVariableType_HalSMBOR));
                }
            } else {
                HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));
            }
        } else if (HSFCharCompare(i,ASCIIH('?'))) {
            if (o->size>0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&inlineif,HalSMVariableType_HalSMInlineIf));
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Inline If (?) cannot without args")));
            }
        } else if (HSFCharCompare(i,ASCIIH(':'))) {
            if (o->size>0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    continue;
                }
                HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,&inlineelse,HalSMVariableType_HalSMInlineElse));
                HalSMArray_free(hsmc->memory,o);
                o=HalSMArray_init(hsmc->memory);
            } else {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Inline Else (:) cannot without args")));
            }
        } else if (HSFCharCompare(i,ASCIIH('~'))) {
            if (o->size>0) {
                if (skip>0) {
                    HalSMArray_free(hsmc->memory,o);
                    o=HalSMArray_init(hsmc->memory);
                    continue;
                }
            } else {
                HalSMArray_add(hsmc->memory,out,&vbnot);
            }
        } else {HalSMArray_add(hsmc->memory,o,HalSMVariable_init_char(hsmc->memory,i));}
    }

    if (o->size!=0) {
        if (skip==0) {
            buffs=HalSMArray_chars_to_str(hsmc->memory,o);
            buffo=HalSMVariable_init_str(hsmc->memory,buffs);
            HalSMCompiler_getArgsValue(hsmc,buffo,buffs,out,vars);
        }
        HalSMArray_free(hsmc->memory,o);
    }

    HalSMCompiler_executeArgs(hsmc,&out,vars);
    HalSMArray_appendArray(hsmc->memory,outarr,out);
    return outarr;
}

void HalSMCompiler_getArgsValue(HalSMCompiler* hsmc,HalSMVariable* buffo,HalStringFormatChar* buffs,HalSMArray* out,Dict* vars) {
    HalSMVariable* isGet,*vartemp;
    if (HSFCompare(buffs,U8H("true"))) {
        HalSMArray_add(hsmc->memory,out,HalSMVariable_init_bool(hsmc->memory,1));
    } else if (HSFCompare(buffs,U8H("false"))) {
        HalSMArray_add(hsmc->memory,out,HalSMVariable_init_bool(hsmc->memory,0));
    } else if (DictElementIndexByKey(hsmc,hsmc->variables,buffo)!=-1) {
        HalSMArray_add(hsmc->memory,out,DictElementFindByKey(hsmc,hsmc->variables,buffo)->value);
    } else if (DictElementIndexByKey(hsmc,vars,buffo)!=-1) {
        HalSMArray_add(hsmc->memory,out,DictElementFindByKey(hsmc,vars,buffo)->value);
    } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,buffo)!=-1) {
        HalSMArray_add(hsmc->memory,out,DictElementFindByKey(hsmc,hsmc->localFunctions,buffo)->value);
    } else if (DictElementIndexByKey(hsmc,hsmc->modules,buffo)!=-1) {
        HalSMArray_add(hsmc->memory,out,DictElementFindByKey(hsmc,hsmc->modules,buffo)->value);
    } else if (DictElementIndexByKey(hsmc,hsmc->classes,buffo)!=-1) {
        HalSMArray_add(hsmc->memory,out,DictElementFindByKey(hsmc,hsmc->classes,buffo)->value);
    } else if ((vartemp=hsmc->getVariable(*(HalStringFormatChar**)buffo->value))->type!=HalSMVariableType_HalSMNull) {
        HalSMArray_add(hsmc->memory,out,vartemp);
    } else if (HSFCompare(buffs,U8H("NULL"))||HSFCompare(buffs,U8H("null"))) {
        HalSMArray_add(hsmc->memory,out,&null);
    } else if (HalSMIsInt(hsmc->memory,buffs)) {
        HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMInteger_parse(hsmc->memory,buffs),HalSMVariableType_int));
    } else if (HalSMIsDouble(hsmc->memory,buffs)) {
        HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMDouble_parse(hsmc->memory,buffs),HalSMVariableType_double));
    } else if (HalSMIsHex(hsmc->memory,buffs)) {
        HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMInteger_parseHex(hsmc->memory,buffs,1),HalSMVariableType_int));
    } else if (HalSMIsBin(hsmc->memory,buffs)) {
        HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMInteger_parseBin(hsmc->memory,buffs),HalSMVariableType_int));
    } else if (StringStartsWith(hsmc->memory,buffs,U8H("["))&&StringEndsWith(hsmc->memory,buffs,U8H("]"))) {
        HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMCompiler_getArgs(hsmc,SubString(hsmc->memory,buffs,1,-1),vars),HalSMVariableType_HalSMArray));
    } else {
        isGet=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMVariable));
        if (HalSMCompiler_isGet(hsmc,buffs,isGet,0,vars)==1) {HalSMArray_add(hsmc->memory,out,isGet);}
        else {HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Variable Not Found")));}
    }
}

ui8 HalSMCompiler_isMore(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b)
{
    if (a->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)a->value)->type->isMore(hsmc,a,b);}
    else if (b->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)b->value)->type->isMore(hsmc,a,b);}
    else if (a->type==HalSMVariableType_int&&b->type==HalSMVariableType_int){return HalSMInteger_isMore((HalSMInteger*)a->value,(HalSMInteger*)b->value);}
    else if (a->type==HalSMVariableType_double&&b->type==HalSMVariableType_double){return HalSMDouble_isMore(hsmc->memory,(HalSMDouble*)a->value,(HalSMDouble*)b->value);}
    else if (a->type==HalSMVariableType_int&&b->type==HalSMVariableType_double){return HalSMDouble_isMore(hsmc->memory,HalSMDouble_FromHalSMInteger(hsmc->memory,(HalSMInteger*)a->value),(HalSMDouble*)b->value);}
    else if (a->type==HalSMVariableType_double&&b->type==HalSMVariableType_int){return HalSMDouble_isMore(hsmc->memory,(HalSMDouble*)a->value,HalSMDouble_FromHalSMInteger(hsmc->memory,(HalSMInteger*)b->value));}
    return 0;
}

ui8 HalSMCompiler_isLess(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b)
{
    if (a->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)a->value)->type->isLess(hsmc,a,b);}
    else if (b->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)b->value)->type->isLess(hsmc,a,b);}
    else if (a->type==HalSMVariableType_int&&b->type==HalSMVariableType_int){return HalSMInteger_isLess((HalSMInteger*)a->value,(HalSMInteger*)b->value);}
    else if (a->type==HalSMVariableType_double&&b->type==HalSMVariableType_double){return HalSMDouble_isLess(hsmc->memory,(HalSMDouble*)a->value,(HalSMDouble*)b->value);}
    else if (a->type==HalSMVariableType_int&&b->type==HalSMVariableType_double){return HalSMDouble_isLess(hsmc->memory,HalSMDouble_FromHalSMInteger(hsmc->memory,(HalSMInteger*)a->value),(HalSMDouble*)b->value);}
    else if (a->type==HalSMVariableType_double&&b->type==HalSMVariableType_int){return HalSMDouble_isLess(hsmc->memory,(HalSMDouble*)a->value,HalSMDouble_FromHalSMInteger(hsmc->memory,(HalSMInteger*)b->value));}
    return 0;
}

HalSMVariable* HalSMCompiler_shift(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b,HalSMShift* shift)
{
    if (shift->c==0) {
        if (a->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)a->value)->type->shiftLeft(hsmc,a,b);}
        else if (b->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)b->value)->type->shiftLeft(hsmc,a,b);}
        else if (a->type==HalSMVariableType_int&&b->type==HalSMVariableType_int){return HalSMVariable_init(hsmc->memory,HalSMInteger_shiftLeft(hsmc->memory,(HalSMInteger*)a->value,HalSMInteger_ToUnsignedInteger((HalSMInteger*)b->value)),HalSMVariableType_int);}
    } else if (shift->c==1) {
        if (a->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)a->value)->type->shiftRight(hsmc,a,b);}
        else if (b->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)b->value)->type->shiftRight(hsmc,a,b);}
        else if (a->type==HalSMVariableType_int&&b->type==HalSMVariableType_int){return HalSMVariable_init(hsmc->memory,HalSMInteger_shiftRight(hsmc->memory,(HalSMInteger*)a->value,HalSMInteger_ToUnsignedInteger((HalSMInteger*)b->value)),HalSMVariableType_int);}
    }
    return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Wrong arguments for shift operation")),HalSMVariableType_HalSMError);
}

HalSMVariable* HalSMCompiler_pow(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b)
{
    if (a->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)a->value)->type->pow(hsmc,a,b);}
    else if (b->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)b->value)->type->pow(hsmc,a,b);}
    else if (a->type==HalSMVariableType_int) {
        if (b->type==HalSMVariableType_int) {
            if (((HalSMInteger*)b->value)->negative) {
                return HalSMVariable_init(hsmc->memory,HalSMDouble_Pow(hsmc->memory,HalSMDouble_FromHalSMInteger(hsmc->memory,(HalSMInteger*)a->value),(HalSMInteger*)b->value),HalSMVariableType_double);
            }
            return HalSMVariable_init(hsmc->memory,HalSMInteger_Pow(hsmc->memory,(HalSMInteger*)a->value,(HalSMInteger*)b->value),HalSMVariableType_int);
        } else if (b->type==HalSMVariableType_double) {
            HalSMDouble* c=HalSMDouble_clear(hsmc->memory,(HalSMDouble*)b->value),*v=HalSMDouble_FromHalSMInteger(hsmc->memory,(HalSMInteger*)a->value);
            HalSMDouble_PowSelf(hsmc->memory,v,c->a);
            HalSMDouble_CropSelf(hsmc->memory,v,10);
            HalSMDouble_SquareRootSelf(hsmc->memory,v,c->b,10);
            if (c->negative) {
                return HalSMVariable_init(hsmc->memory,HalSMDouble_Div(hsmc->memory,&HalSMDoubleOne,v),HalSMVariableType_double);
            }
            return HalSMVariable_init(hsmc->memory,v,HalSMVariableType_double);
        }
    }
    else if (a->type==HalSMVariableType_double) {
        if (b->type==HalSMVariableType_int) {
            return HalSMVariable_init(hsmc->memory,HalSMDouble_Pow(hsmc->memory,(HalSMDouble*)a->value,(HalSMInteger*)b->value),HalSMVariableType_double);
        } else if (b->type==HalSMVariableType_double) {
            HalSMDouble* c=HalSMDouble_clear(hsmc->memory,(HalSMDouble*)b->value),*v=HalSMDouble_copy(hsmc->memory,(HalSMDouble*)a->value);
            HalSMDouble_PowSelf(hsmc->memory,v,c->a);
            HalSMDouble_SquareRootSelf(hsmc->memory,v,c->b,10);
            if (c->negative) {
                return HalSMVariable_init(hsmc->memory,HalSMDouble_Div(hsmc->memory,&HalSMDoubleOne,v),HalSMVariableType_double);
            }
            return HalSMVariable_init(hsmc->memory,v,HalSMVariableType_double);
        }
    }
    return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Wrong arguments for pow operation")),HalSMVariableType_HalSMError);
}

HalSMVariable* HalSMCompiler_powFunc(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_int||args->arr[0]->type==HalSMVariableType_double) {
            return HalSMCompiler_pow(hsmc,args->arr[0],HalSMVariable_init(hsmc->memory,&HalSMIntegerTwo,HalSMVariableType_int));
        }
    } else if (args->size==2) {
        if ((args->arr[0]->type==HalSMVariableType_int||args->arr[0]->type==HalSMVariableType_double)&&(args->arr[1]->type==HalSMVariableType_int||args->arr[1]->type==HalSMVariableType_double)) {
            return HalSMCompiler_pow(hsmc,args->arr[0],args->arr[1]);
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_bin(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_int) {
            HalSMInteger* hi=(HalSMInteger*)args->arr[0]->value;
            if (hi->size==0){return HalSMVariable_init_str(hsmc->memory,U8H("0b0"));}
            HalStringFormatChar* out=HalSMMemory_calloc(hsmc->memory,hi->size*8+3,sizeof(HalStringFormatChar));
            out[0]=ASCIIH('0');
            out[1]=ASCIIH('b');
            HalStringFormatChar* p=&out[2];
            ui64 i;
            i=0;
            if (hi->value[0]<2) {
                *p++=ASCIIH('0'+(hi->value[i]&1));
            } else if (hi->value[0]<4) {
                *p++=ASCIIH('0'+((hi->value[i]>>1)&1));
                *p++=ASCIIH('0'+(hi->value[i]&1));
            } else if (hi->value[0]<8) {
                *p++=ASCIIH('0'+((hi->value[i]>>2)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>1)&1));
                *p++=ASCIIH('0'+(hi->value[i]&1));
            } else if (hi->value[0]<16) {
                *p++=ASCIIH('0'+((hi->value[i]>>3)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>2)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>1)&1));
                *p++=ASCIIH('0'+(hi->value[i]&1));
            } else if (hi->value[0]<32) {
                *p++=ASCIIH('0'+((hi->value[i]>>4)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>3)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>2)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>1)&1));
                *p++=ASCIIH('0'+(hi->value[i]&1));
            } else if (hi->value[0]<64) {
                *p++=ASCIIH('0'+((hi->value[i]>>5)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>4)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>3)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>2)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>1)&1));
                *p++=ASCIIH('0'+(hi->value[i]&1));
            } else if (hi->value[0]<128) {
                *p++=ASCIIH('0'+((hi->value[i]>>6)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>5)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>4)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>3)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>2)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>1)&1));
                *p++=ASCIIH('0'+(hi->value[i]&1));
            } else {
                *p++=ASCIIH('0'+((hi->value[i]>>7)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>6)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>5)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>4)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>3)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>2)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>1)&1));
                *p++=ASCIIH('0'+(hi->value[i]&1));
            }
            
            for (i=1;i<hi->size;i++) {
                *p++=ASCIIH('0'+((hi->value[i]>>7)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>6)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>5)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>4)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>3)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>2)&1));
                *p++=ASCIIH('0'+((hi->value[i]>>1)&1));
                *p++=ASCIIH('0'+(hi->value[i]&1));
                //if (i==0){break;}
            }
            return HalSMVariable_init_str(hsmc->memory,out);
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_hex(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_int) {
            HalStringFormatChar* s=HalSMInteger_toString(hsmc->memory,(HalSMInteger*)args->arr[0]->value,1);
            if (string_len(s)>1&&HSFCharCompare(s[0],ASCIIH('0'))) {
                ui64 l=string_len(s);
                memory_cpy(s,&s[1],l*sizeof(HalStringFormatChar)-sizeof(HalStringFormatChar));
                s[l-1]=ASCIIH('\0');
            }
            return HalSMVariable_init_str(hsmc->memory,ConcatenateStrings(hsmc->memory,U8H("0x"),s));
        } else if (args->arr[0]->type==HalSMVariableType_double) {
            return HalSMVariable_init_str(hsmc->memory,ConcatenateStrings(hsmc->memory,U8H("0x"),HalSMDouble_toString(hsmc->memory,(HalSMDouble*)args->arr[0]->value,1)));
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_modulo(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b)
{
    if (a->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)a->value)->type->modulo(hsmc,a,b);}
    else if (b->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)b->value)->type->modulo(hsmc,a,b);}
    else if (a->type==HalSMVariableType_int&&b->type==HalSMVariableType_int){
        HalSMInteger* rem=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMInteger));
        HalSMInteger_Div(hsmc->memory,(HalSMInteger*)a->value,(HalSMInteger*)b->value,rem);
        return HalSMVariable_init(hsmc->memory,rem,HalSMVariableType_int);
    }
    return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Wrong arguments for modulo operation")),HalSMVariableType_HalSMError);
}

ui8 HalSMCompiler_getBool(HalSMCompiler* hsmc,HalSMVariable* v)
{
    if(v->type==HalSMVariableType_custom){return ((HalSMVariableCustom*)v->value)->type->getBool(hsmc,v);}
    else if(v->type==HalSMVariableType_int){return HalSMInteger_isEqual((HalSMInteger*)v->value,&HalSMIntegerZero)==0;}
    else if(v->type==HalSMVariableType_double){return HalSMDouble_isEqual((HalSMDouble*)v->value,&HalSMDoubleZero)==0;}
    else if(v->type==HalSMVariableType_char){return (*(HalStringFormatChar*)v->value).alphabet>0||(*(HalStringFormatChar*)v->value).symbol>0;}
    else if(v->type==HalSMVariableType_void){return 0;}
    else if(v->type==HalSMVariableType_HalSMArray){return ((HalSMArray*)v->value)->size>0;}
    else if(v->type==HalSMVariableType_str){return string_len(*(HalStringFormatChar**)v->value)>0;}
    else if(v->type==HalSMVariableType_int_array){return 1;}
    else if(v->type==HalSMVariableType_HalSMFunctionC){return 1;}
    else if(v->type==HalSMVariableType_HalSMClassC){return 1;}
    else if(v->type==HalSMVariableType_HalSMRunClassC){return 1;}
    else if(v->type==HalSMVariableType_HalSMSetArg){return 1;}
    else if(v->type==HalSMVariableType_HalSMError){return string_len(((HalSMError*)v->value)->error)>0;}
    else if(v->type==HalSMVariableType_HalSMNull){return 0;}
    else if(v->type==HalSMVariableType_HalSMRunFunc){return 1;}
    else if(v->type==HalSMVariableType_HalSMRunFuncC){return 1;}
    else if(v->type==HalSMVariableType_HalSMLocalFunction){return 1;}
    else if(v->type==HalSMVariableType_HalSMCModule){return 1;}
    else if(v->type==HalSMVariableType_HalSMModule){return 1;}
    else if(v->type==HalSMVariableType_HalSMCompiler){return 1;}
    else if(v->type==HalSMVariableType_HalSMRunClass){return 1;}
    else if(v->type==HalSMVariableType_HalSMDoubleGet){return string_len(((HalSMDoubleGet*)v->value)->st)>0;}
    else if(v->type==HalSMVariableType_HalSMClass){return 1;}
    else if(v->type==HalSMVariableType_HalSMVar){return 1;}
    else if(v->type==HalSMVariableType_HalSMPlus){return 1;}
    else if(v->type==HalSMVariableType_HalSMMinus){return 0;}
    else if(v->type==HalSMVariableType_HalSMMult){return 1;}
    else if(v->type==HalSMVariableType_HalSMDivide){return 1;}
    else if(v->type==HalSMVariableType_HalSMEqual){return 1;}
    else if(v->type==HalSMVariableType_HalSMNotEqual){return 1;}
    else if(v->type==HalSMVariableType_HalSMMore){return 1;}
    else if(v->type==HalSMVariableType_HalSMLess){return 1;}
    else if(v->type==HalSMVariableType_HalSMBool){return *(ui8*)v->value;}
    else if(v->type==HalSMVariableType_HalSMDict){return ((Dict*)v->value)->size>0;}
    else if(v->type==HalSMVariableType_HalSMSetVar){return 1;}
    else if(v->type==HalSMVariableType_HalSMReturn){return 1;}
    else if(v->type==HalSMVariableType_HalSMFunctionCTypeDef){return 1;}
    else if(v->type==HalSMVariableType_HalSMFunctionArray){return 1;}
    else if(v->type==HalSMVariableType_unsigned_int){return (*(ui64*)v->value)!=0;}
    else if(v->type==HalSMVariableType_HalSMMoreEqual){return 1;}
    else if(v->type==HalSMVariableType_HalSMLessEqual){return 1;}
    else if(v->type==HalSMVariableType_HalSMShift){return 1;}
    else if(v->type==HalSMVariableType_HalSMPow){return 1;}
    else if(v->type==HalSMVariableType_HalSMModulo){return 1;}
    else if(v->type==HalSMVariableType_HalSMAND){return 1;}
    else if(v->type==HalSMVariableType_HalSMOR){return 1;}
    else if(v->type==HalSMVariableType_HalSMNegate){return 1;}
    else if(v->type==HalSMVariableType_HalSMNegativeSign){return 1;}
    else if(v->type==HalSMVariableType_HalSMGOTO){return 1;}
    else if(v->type==HalSMVariableType_HalSMGOTOSector){return 1;}
    else if(v->type==HalSMVariableType_HalSMInlineIf){return 1;}
    else if(v->type==HalSMVariableType_HalSMInlineElse){return 1;}
    else if(v->type==HalSMVariableType_HalSMBreak){return 1;}
    else if(v->type==HalSMVariableType_HalSMXOR){return 1;}
    else if(v->type==HalSMVariableType_HalSMBAND){return 1;}
    else if(v->type==HalSMVariableType_HalSMBOR){return 1;}
    else if(v->type==HalSMVariableType_HalSMBNOT){return 1;}
    return 0;
}

HalSMVariable* HalSMCompiler_negate(HalSMCompiler* hsmc,HalSMVariable* v)
{
    if(v->type==HalSMVariableType_int){
        HalSMInteger* out=HalSMInteger_copy(hsmc->memory,(HalSMInteger*)v->value);
        out->negative=out->negative?0:1;
        return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_int);
    } else if (v->type==HalSMVariableType_double) {
        HalSMDouble* out=HalSMDouble_copy(hsmc->memory,(HalSMDouble*)v->value);
        out->negative=out->negative?0:1;
        return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_double);
    }
    return v;
}



HalSMVariable* HalSMCompiler_replace(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==3&&args->arr[0]->type==HalSMVariableType_str&&args->arr[1]->type==HalSMVariableType_str&&args->arr[2]->type==HalSMVariableType_str) {
        return HalSMVariable_init_str(hsmc->memory,StringReplace(hsmc->memory,*(HalStringFormatChar**)args->arr[0]->value,*(HalStringFormatChar**)args->arr[1]->value,*(HalStringFormatChar**)args->arr[2]->value));
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_index(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2) {
        if (args->arr[0]->type==HalSMVariableType_str&&args->arr[1]->type==HalSMVariableType_str) {
            i64 res=StringIndexOf(hsmc->memory,*(HalStringFormatChar**)args->arr[0]->value,*(HalStringFormatChar**)args->arr[1]->value);
            if (res==-1){return HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromSignedInteger(hsmc->memory,-1));}
            return HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,res));
        } else if (args->arr[0]->type==HalSMVariableType_HalSMArray) {
            i64 s=(i64)HalSMArray_index(hsmc,(HalSMArray*)args->arr[0]->value,args->arr[1]);
            return HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromSignedInteger(hsmc->memory,s));
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_rindex(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2) {
        if (args->arr[0]->type==HalSMVariableType_str&&args->arr[1]->type==HalSMVariableType_str) {
            i64 res=StringLastIndexOf(hsmc->memory,*(HalStringFormatChar**)args->arr[0]->value,*(HalStringFormatChar**)args->arr[1]->value);
            if (res==-1) {return HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromSignedInteger(hsmc->memory,-1));}
            return HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,res));
        } else if (args->arr[0]->type==HalSMVariableType_HalSMArray) {
            i64 res=HalSMArray_right_index(hsmc,(HalSMArray*)args->arr[0]->value,args->arr[1]);
            if (res==-1) {return HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromSignedInteger(hsmc->memory,-1));}
            return HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,res));
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_split(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_str&&args->arr[1]->type==HalSMVariableType_str) {
        return HalSMVariable_init(hsmc->memory,HalSMArray_split_str(hsmc->memory,*(HalStringFormatChar**)args->arr[0]->value,*(HalStringFormatChar**)args->arr[1]->value),HalSMVariableType_HalSMArray);
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_substring(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==3&&args->arr[0]->type==HalSMVariableType_str&&args->arr[1]->type==HalSMVariableType_int&&args->arr[2]->type==HalSMVariableType_int) {
        return HalSMVariable_init_str(hsmc->memory,SubStringHalSMInteger(hsmc->memory,*(HalStringFormatChar**)args->arr[0]->value,(HalSMInteger*)args->arr[1]->value,(HalSMInteger*)args->arr[2]->value));
    }
    return &hvoid;
}



HalSMVariable* HalSMCompiler_append(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMArray) {
        HalSMArray_add(hsmc->memory,(HalSMArray*)args->arr[0]->value,args->arr[1]);
        return args->arr[0];
    } else if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMDict&&args->arr[1]->type==HalSMVariableType_HalSMDict) {
        PutDictElementToDict(hsmc,(Dict*)args->arr[0]->value,(DictElement*)args->arr[1]->value);
        return args->arr[0];
    } else if (args->size==3&&args->arr[0]->type==HalSMVariableType_HalSMDict) {
        PutDictElementToDict(hsmc,(Dict*)args->arr[0]->value,DictElementInit(hsmc->memory,args->arr[1],args->arr[2]));
        return args->arr[0];
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_clear(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_HalSMArray) {
            ((HalSMArray*)args->arr[0]->value)->arr=HalSMMemory_malloc(hsmc->memory,0);
            ((HalSMArray*)args->arr[0]->value)->size=0;
            return args->arr[0];
        } else if (args->arr[0]->type==HalSMVariableType_HalSMDict) {
            ((Dict*)args->arr[0]->value)->elements=HalSMMemory_malloc(hsmc->memory,0);
            ((Dict*)args->arr[0]->value)->size=0;
            return args->arr[0];
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_insert(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==3&&args->arr[0]->type==HalSMVariableType_HalSMArray&&args->arr[2]->type==HalSMVariableType_int) {
        HalSMArray_insert(hsmc->memory,(HalSMArray*)args->arr[0]->value,args->arr[1],HalSMInteger_ToUnsignedInteger((HalSMInteger*)args->arr[2]->value));
        return args->arr[0];
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_pop(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMArray&&args->arr[1]->type==HalSMVariableType_int) {
        ui64 index=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value);
        if (index<((HalSMArray*)args->arr[0]->value)->size) {
            //HalSMVariable* temp=((HalSMArray*)args->arr[0]->value)->arr[index];
            HalSMArray_remove(hsmc->memory,(HalSMArray*)args->arr[0]->value,index);
            //return temp;
        }
        return args->arr[0];
    } else if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMDict&&args->arr[1]->type==HalSMVariableType_int) {
        ui64 index=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value);
        if (index<((Dict*)args->arr[0]->value)->size) {
            //Dict* temp=DictInitWithElements(hsmc->memory,(DictElement*[]){((Dict*)args->arr[0]->value)->elements[index]},1);
            DictRemove(hsmc->memory,(Dict*)args->arr[0]->value,index);
            //return HalSMVariable_init(hsmc->memory,temp,HalSMVariableType_HalSMDict);
        }
        return args->arr[0];
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_remove(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMArray) {
        HalSMArray* arrarg=(HalSMArray*)args->arr[0]->value;
        for (ui64 i=0;i<arrarg->size;i++) {
            if (HalSMVariable_Compare(hsmc,args->arr[1],arrarg->arr[i])) {
                HalSMArray_remove(hsmc->memory,arrarg,i);
                return args->arr[0];
            }
        }
        return args->arr[0];
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_reverse(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1&&args->arr[0]->type==HalSMVariableType_HalSMArray){return HalSMVariable_init(hsmc->memory,HalSMArray_reverse(hsmc->memory,(HalSMArray*)args->arr[0]->value),HalSMVariableType_HalSMArray);}
    return &hvoid;
}

HalSMVariable* HalSMCompiler_copy(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1&&args->arr[0]->type==HalSMVariableType_HalSMArray){return HalSMVariable_init(hsmc->memory,HalSMArray_copy(hsmc->memory,(HalSMArray*)args->arr[0]->value),HalSMVariableType_HalSMArray);}
    return &hvoid;
}

HalSMVariable* HalSMCompiler_appendArray(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMArray&&args->arr[1]->type==HalSMVariableType_HalSMArray){HalSMArray_appendArray(hsmc->memory,(HalSMArray*)args->arr[0]->value,(HalSMArray*)args->arr[1]->value);return args->arr[0];}
    return &hvoid;
}

HalSMVariable* HalSMCompiler_slice(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==3&&args->arr[0]->type==HalSMVariableType_HalSMArray&&args->arr[1]->type==HalSMVariableType_int&&args->arr[2]->type==HalSMVariableType_int) {
        HalSMInteger* start=(HalSMInteger*)args->arr[1]->value;
        HalSMInteger* end=(HalSMInteger*)args->arr[2]->value;
        if (HalSMInteger_isLess(start,&HalSMIntegerZero)) {
            start=HalSMInteger_Add(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,((HalSMArray*)args->arr[0]->value)->size),start);
        }
        
        if (HalSMInteger_isLess(start,&HalSMIntegerZero)) {
            return &null;
        }

        if (HalSMInteger_isLess(end,&HalSMIntegerZero)) {
            end=HalSMInteger_Add(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,((HalSMArray*)args->arr[0]->value)->size),end);
        }
        
        if (HalSMInteger_isLess(end,&HalSMIntegerZero)) {
            return &null;
        }
        return HalSMVariable_init(hsmc->memory,HalSMArray_slice(hsmc->memory,(HalSMArray*)args->arr[0]->value,HalSMInteger_ToUnsignedInteger(start),HalSMInteger_ToUnsignedInteger(end)),HalSMVariableType_HalSMArray);
    }
    return &hvoid;
}



HalSMVariable* HalSMCompiler_len(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size>0) {
        HalSMInteger* out=HalSMInteger_FromUnsignedInteger(hsmc->memory,0);
        ui64 i;
        HalSMVariable* tempk=HalSMVariable_init_str(hsmc->memory,U8H("__len__"));
        HalSMVariable* f;
        for (i=0;i<args->size;i++) {
            if (args->arr[i]->type==HalSMVariableType_HalSMArray){HalSMInteger_AddSelf(hsmc->memory,out,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,((HalSMArray*)args->arr[i]->value)->size));}
            else if (args->arr[i]->type==HalSMVariableType_str){HalSMInteger_AddSelf(hsmc->memory,out,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,string_len(*(HalStringFormatChar**)args->arr[i]->value)));}
            else if (args->arr[i]->type==HalSMVariableType_custom){
                if (((HalSMVariableCustom*)args->arr[i]->value)->type->indexFunctionByName(hsmc,args->arr[i],U8H("__len__"))!=-1) {
                    f=((HalSMVariableCustom*)args->arr[i]->value)->type->getFunctionByName(hsmc,args->arr[i],U8H("__len__"));
                    if (f->type==HalSMVariableType_HalSMLocalFunction) {
                        f=HalSMLocalFunction_run((HalSMLocalFunction*)f->value,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){args->arr[i]},1),vars);
                        if (f->type==HalSMVariableType_HalSMError){return f;}
                    } else if (f->type==HalSMVariableType_HalSMFunctionC) {
                        f=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)args->arr[i]->value,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){args->arr[i]},1),vars);
                        if (f->type==HalSMVariableType_HalSMError){return f;}
                    }
                    if (f->type==HalSMVariableType_int) {
                        HalSMInteger_AddSelf(hsmc->memory,out,(HalSMInteger*)f->value);
                    }
                }
            } else if (args->arr[i]->type==HalSMVariableType_HalSMClass) {
                if (DictElementIndexByKey(hsmc,((HalSMClass*)args->arr[i]->value)->funcs,tempk)!=-1) {
                    f=HalSMLocalFunction_run((HalSMLocalFunction*)DictElementFindByKey(hsmc,((HalSMClass*)args->arr[i]->value)->funcs,tempk)->value->value,HalSMArray_init(hsmc->memory),vars);
                    if (f->type==HalSMVariableType_int) {
                        HalSMInteger_AddSelf(hsmc->memory,out,(HalSMInteger*)f->value);
                    } else if (f->type==HalSMVariableType_HalSMError) {
                        return f;
                    }
                }
            } else if (args->arr[i]->type==HalSMVariableType_HalSMClassC) {
                if (DictElementIndexByKey(hsmc,((HalSMClassC*)args->arr[i]->value)->funcs,tempk)!=-1) {
                    f=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)DictElementFindByKey(hsmc,((HalSMClassC*)args->arr[i]->value)->funcs,tempk)->value->value,HalSMArray_init(hsmc->memory),vars);
                    if (f->type==HalSMVariableType_int) {
                        HalSMInteger_AddSelf(hsmc->memory,out,(HalSMInteger*)f->value);
                    } else if (f->type==HalSMVariableType_HalSMError) {
                        return f;
                    }
                }
            } else if (args->arr[i]->type==HalSMVariableType_HalSMRunClass) {
                if (DictElementIndexByKey(hsmc,((HalSMRunClass*)args->arr[i]->value)->funcs,tempk)!=-1) {
                    f=HalSMLocalFunction_run((HalSMLocalFunction*)DictElementFindByKey(hsmc,((HalSMRunClass*)args->arr[i]->value)->funcs,tempk)->value->value,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){args->arr[i]},1),vars);
                    if (f->type==HalSMVariableType_int) {
                        HalSMInteger_AddSelf(hsmc->memory,out,(HalSMInteger*)f->value);
                    } else if (f->type==HalSMVariableType_HalSMError) {
                        return f;
                    }
                }
            } else if (args->arr[i]->type==HalSMVariableType_HalSMRunClassC) {
                if (DictElementIndexByKey(hsmc,((HalSMRunClassC*)args->arr[i]->value)->funcs,tempk)!=-1) {
                    f=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)DictElementFindByKey(hsmc,((HalSMRunClassC*)args->arr[i]->value)->funcs,tempk)->value->value,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){args->arr[i]},1),vars);
                    if (f->type==HalSMVariableType_int) {
                        HalSMInteger_AddSelf(hsmc->memory,out,(HalSMInteger*)f->value);
                    } else if (f->type==HalSMVariableType_HalSMError) {
                        return f;
                    }
                }
            } else if (args->arr[i]->type==HalSMVariableType_HalSMModule) {
                if (DictElementIndexByKey(hsmc,((HalSMModule*)args->arr[i]->value)->hsmc->localFunctions,tempk)!=-1) {
                    f=HalSMLocalFunction_run((HalSMLocalFunction*)DictElementFindByKey(hsmc,((HalSMModule*)args->arr[i]->value)->hsmc->localFunctions,tempk)->value->value,HalSMArray_init(hsmc->memory),vars);
                    if (f->type==HalSMVariableType_int) {
                        HalSMInteger_AddSelf(hsmc->memory,out,(HalSMInteger*)f->value);
                    } else if (f->type==HalSMVariableType_HalSMError) {
                        return f;
                    }
                }
            } else if (args->arr[i]->type==HalSMVariableType_HalSMCModule) {
                if (DictElementIndexByKey(hsmc,((HalSMCModule*)args->arr[i]->value)->lfuncs,tempk)!=-1) {
                    f=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)DictElementFindByKey(hsmc,((HalSMCModule*)args->arr[i]->value)->lfuncs,tempk)->value->value,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){args->arr[i]},1),vars);
                    if (f->type==HalSMVariableType_int) {
                        HalSMInteger_AddSelf(hsmc->memory,out,(HalSMInteger*)f->value);
                    } else if (f->type==HalSMVariableType_HalSMError) {
                        return f;
                    }
                }
            } else if (args->arr[i]->type==HalSMVariableType_char) {
                HalSMInteger_AddSelf(hsmc->memory,out,&HalSMIntegerOne);
            } else if (args->arr[i]->type==HalSMVariableType_HalSMBool) {
                if ((*(ui8*)args->arr[i]->value)==1) {
                    HalSMInteger_AddSelf(hsmc->memory,out,&HalSMIntegerOne);
                }
            } else if (args->arr[i]->type==HalSMVariableType_HalSMDict) {
                HalSMInteger_AddSelf(hsmc->memory,out,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,((Dict*)args->arr[i]->value)->size));
            }
        }
        HalSMMemory_free(hsmc->memory,*(HalStringFormatChar**)tempk->value);
        HalSMMemory_free(hsmc->memory,tempk->value);
        HalSMMemory_free(hsmc->memory,tempk);
        return HalSMVariable_init_int(hsmc->memory,out);
    }
    return &hvoid;
}



HalSMVariable* HalSMCompiler_appendDict(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMDict&&args->arr[1]->type==HalSMVariableType_HalSMDict){DictAppendDict(hsmc->memory,(Dict*)args->arr[0]->value,(Dict*)args->arr[1]->value);return args->arr[0];}
    return &hvoid;
}

HalSMVariable* HalSMCompiler_indexByKey(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMDict) {
        i64 r=DictElementIndexByKey(hsmc,(Dict*)args->arr[0]->value,args->arr[1]);
        return HalSMVariable_init(hsmc->memory,r<0?HalSMInteger_FromSignedInteger(hsmc->memory,-1):HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,(ui64)r),HalSMVariableType_int);
    }
    return &hvoid;
}
HalSMVariable* HalSMCompiler_indexByValue(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMDict) {
        i64 r=DictElementIndexByValue(hsmc,(Dict*)args->arr[0]->value,args->arr[1]);
        return HalSMVariable_init(hsmc->memory,r<0?HalSMInteger_FromSignedInteger(hsmc->memory,(signed int)r):HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,(ui64)r),HalSMVariableType_int);
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_keyByIndex(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMDict&&args->arr[1]->type==HalSMVariableType_int) {
        i64 ind=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value);
        if (((HalSMInteger*)args->arr[1]->value)->negative){ind=-ind;}
        if (ind<0) {
            ind=((Dict*)args->arr[0]->value)->size+ind;
        }
        if (ind<0||ind>=((Dict*)args->arr[0]->value)->size){return &null;}
        return ((Dict*)args->arr[0]->value)->elements[ind]->key;
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_valueByIndex(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMDict&&args->arr[1]->type==HalSMVariableType_int) {
        i64 ind=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value);
        if (((HalSMInteger*)args->arr[1]->value)->negative){ind=-ind;}
        if (ind<0) {
            ind=((Dict*)args->arr[0]->value)->size+ind;
        }
        if (ind<0||ind>=((Dict*)args->arr[0]->value)->size){return &null;}
        return ((Dict*)args->arr[0]->value)->elements[ind]->value;
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_set(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==3&&args->arr[0]->type==HalSMVariableType_HalSMArray&&args->arr[1]->type==HalSMVariableType_int) {
        ui64 i=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value);
        if (i<((HalSMArray*)args->arr[0]->value)->size) {
            ((HalSMArray*)args->arr[0]->value)->arr[i]=args->arr[2];
        }
        return args->arr[0];
    } else if (args->size==3&&args->arr[0]->type==HalSMVariableType_HalSMDict&&args->arr[1]->type==HalSMVariableType_int&&args->arr[2]->type==HalSMVariableType_HalSMDict) {
        ui64 i=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value);
        if (i<((Dict*)args->arr[0]->value)->size&&((Dict*)args->arr[2]->value)->size>0) {
            ((Dict*)args->arr[0]->value)->elements[i]=((Dict*)args->arr[2]->value)->elements[0];
        }
        return args->arr[0];
    } else if (args->size==3&&args->arr[0]->type==HalSMVariableType_HalSMDict) {
        PutDictElementToDict(hsmc,(Dict*)args->arr[0]->value,DictElementInit(hsmc->memory,args->arr[1],args->arr[2]));
        return args->arr[0];
    } else if (args->size==2&&args->arr[0]->type==HalSMVariableType_HalSMDict&&args->arr[1]->type==HalSMVariableType_HalSMDict) {
        if (((Dict*)args->arr[1]->value)->size>0) {
            PutDictElementToDict(hsmc,(Dict*)args->arr[0]->value,((Dict*)args->arr[1]->value)->elements[0]);
        }
        return args->arr[0];
    }
    return &hvoid;
}



HalSMVariable* HalSMCompiler_sqrt(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if ((args->size>0)&&(args->arr[0]->type==HalSMVariableType_int||args->arr[0]->type==HalSMVariableType_double)) {
        HalSMDouble* a;
        HalSMInteger* q=&HalSMIntegerTwo;
        ui64 crop=10;
        if (args->arr[0]->type==HalSMVariableType_int) {
            a=HalSMDouble_FromHalSMInteger(hsmc->memory,(HalSMInteger*)args->arr[0]->value);
        } else {
            a=(HalSMDouble*)args->arr[0]->value;
        }
        if (args->size>1&&args->arr[1]->type==HalSMVariableType_int) {
            q=(HalSMInteger*)args->arr[1]->value;
        }

        if (args->size>2&&args->arr[2]->type==HalSMVariableType_int) {
            crop=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[2]->value);
        }

        return HalSMVariable_init(hsmc->memory,HalSMDouble_SquareRoot(hsmc->memory,a,q,crop),HalSMVariableType_double);
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_int(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_int) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_copy(hsmc->memory,(HalSMInteger*)args->arr[0]->value),HalSMVariableType_int);
        } else if (args->arr[0]->type==HalSMVariableType_double) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_FromHalSMDouble(hsmc->memory,(HalSMDouble*)args->arr[0]->value),HalSMVariableType_int);
        } else if (args->arr[0]->type==HalSMVariableType_str) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_parse(hsmc->memory,*(HalStringFormatChar**)args->arr[0]->value),HalSMVariableType_int);
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_double(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_double) {
            return HalSMVariable_init(hsmc->memory,HalSMDouble_copy(hsmc->memory,(HalSMDouble*)args->arr[0]->value),HalSMVariableType_double);
        } else if (args->arr[0]->type==HalSMVariableType_int) {
            return HalSMVariable_init(hsmc->memory,HalSMDouble_FromHalSMInteger(hsmc->memory,(HalSMInteger*)args->arr[0]->value),HalSMVariableType_double);
        } else if (args->arr[0]->type==HalSMVariableType_str) {
            return HalSMVariable_init(hsmc->memory,HalSMDouble_parse(hsmc->memory,*(HalStringFormatChar**)args->arr[0]->value),HalSMVariableType_double);
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_str(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size>0&&args->size<3) {
        if (args->arr[0]->type==HalSMVariableType_str) {
            return HalSMVariable_init_str(hsmc->memory,*(HalStringFormatChar**)args->arr[0]->value);
        } else if (args->arr[0]->type==HalSMVariableType_custom) {
            if (HSFCompare(((HalSMVariableCustom*)args->arr[0]->value)->type->name,U8H("ByteArray"))) {
                ui32 type=0;
                if (args->size==2&&args->arr[1]->type==HalSMVariableType_str) {
                    HalStringFormatChar* stype=*(HalStringFormatChar**)args->arr[1]->value;
                    if (HSFCompare(stype,U8H("utf8"))) {
                        type=1;
                    } else if (HSFCompare(stype,U8H("utf16"))) {
                        type=2;
                    }
                }
                
                HalSMByteArray* ba=(HalSMByteArray*)((HalSMVariableCustom*)args->arr[0]->value)->value;
                HalStringFormatChar* out=NOTHALSMNULLPOINTER;
                if (ba->size==0) {
                    out=U8H("");
                } else if (type==0) {
                    if (ba->size%4==0) {
                        out=(HalStringFormatChar*)ba->bytes;
                    }
                } else if (type==1) {
                    out=U8H((char*)ba->bytes);
                } else if (type==2) {
                    out=U16H((short*)ba->bytes);
                }
                if (out!=NOTHALSMNULLPOINTER) {
                    return HalSMVariable_init_str(hsmc->memory,out);
                }
            }
        }
        return HalSMVariable_init_str(hsmc->memory,HalSMVariable_to_str(hsmc,args->arr[0],0));
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_floor(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_int) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_copy(hsmc->memory,(HalSMInteger*)args->arr[0]->value),HalSMVariableType_int);
        } else if (args->arr[0]->type==HalSMVariableType_double) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_FloorHalSMDouble(hsmc->memory,(HalSMDouble*)args->arr[0]->value),HalSMVariableType_int);
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_ceil(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_int) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_copy(hsmc->memory,(HalSMInteger*)args->arr[0]->value),HalSMVariableType_int);
        } else if (args->arr[0]->type==HalSMVariableType_double) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_CeilHalSMDouble(hsmc->memory,(HalSMDouble*)args->arr[0]->value),HalSMVariableType_int);
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_round(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2) {
        if (args->arr[0]->type==HalSMVariableType_int) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_copy(hsmc->memory,(HalSMInteger*)args->arr[0]->value),HalSMVariableType_int);
        } else if (args->arr[0]->type==HalSMVariableType_double&&args->arr[1]->type==HalSMVariableType_int) {
            if (HalSMInteger_isEqual((HalSMInteger*)args->arr[1]->value,&HalSMIntegerZero)){return HalSMVariable_init(hsmc->memory,HalSMInteger_CeilHalSMDouble(hsmc->memory,(HalSMDouble*)args->arr[0]->value),HalSMVariableType_int);}
            return HalSMVariable_init(hsmc->memory,HalSMDouble_parse(hsmc->memory,HalSMDouble_toStringRound(hsmc->memory,(HalSMDouble*)args->arr[0]->value,HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value))),HalSMVariableType_double);
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_abs(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_int) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_copy(hsmc->memory,HalSMInteger_absolute((HalSMInteger*)args->arr[0]->value)),HalSMVariableType_int);
        } else if (args->arr[0]->type==HalSMVariableType_double) {
            return HalSMVariable_init(hsmc->memory,HalSMDouble_Abs(hsmc->memory,(HalSMDouble*)args->arr[0]->value),HalSMVariableType_double);
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_join(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2) {
        if (args->arr[0]->type==HalSMVariableType_HalSMArray&&args->arr[1]->type==HalSMVariableType_str) {
            HalSMArray* harr=(HalSMArray*)args->arr[0]->value;
            HalStringFormatChar* join=*(HalStringFormatChar**)args->arr[1]->value;
            if(harr->size==0) {
                return HalSMVariable_init_str(hsmc->memory,U8H(""));
            } else if (harr->size==1) {
                return HalSMVariable_init_str(hsmc->memory,HalSMVariable_to_str(hsmc,harr->arr[0]->value,1));
            }
            ui64 size=0;
            ui64 lj=string_len(join);
            HalStringFormatChar* out=HalSMMemory_calloc(hsmc->memory,1,sizeof(HalStringFormatChar));
            out[0]=ASCIIH('\0');
            ui64 i;
            size=0;
            for (i=0;i<harr->size;i++) {
                HalStringFormatChar* oc=HalSMVariable_to_str(hsmc,harr->arr[i],1);
                AdditionStrings(hsmc->memory,&out,oc,size,string_len(oc));
                size+=string_len(oc);
                if (i!=harr->size-1) {
                    AdditionStrings(hsmc->memory,&out,join,size,lj);
                    size+=lj;
                }
            }
            out[size]=ASCIIH('\0');
            return HalSMVariable_init_str(hsmc->memory,out);
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_array(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_HalSMArray) {
            return HalSMVariable_init(hsmc->memory,HalSMArray_copy(hsmc->memory,(HalSMArray*)args->arr[0]->value),HalSMVariableType_HalSMArray);
        } else if (args->arr[0]->type==HalSMVariableType_custom) {
            HalSMVariableCustom* vc=(HalSMVariableCustom*)args->arr[0]->value;
            return vc->type->convert(hsmc,args->arr[0],&HNULLArray);
        } else if (args->arr[0]->type==HalSMVariableType_str) {
            HalStringFormatChar* str=*(HalStringFormatChar**)args->arr[0]->value;
            if (StringStartsWith(hsmc->memory,str,U8H("["))&&StringEndsWith(hsmc->memory,str,U8H("]"))) {
                return HalSMVariable_init(hsmc->memory,HalSMCompiler_getArgs(hsmc,SubString(hsmc->memory,str,1,-1),vars),HalSMVariableType_HalSMArray);
            }
        }
    } else if(args->size==2) {
        if (args->arr[0]->type==HalSMVariableType_str&&args->arr[1]->type==HalSMVariableType_HalSMBool) {
            HalStringFormatChar* str=*(HalStringFormatChar**)args->arr[0]->value;
            ui8 b=*(ui8*)args->arr[1]->value;
            if(b) {
                return HalSMVariable_init(hsmc->memory,HalSMArray_from_str(hsmc->memory,str,string_len(str)),HalSMVariableType_HalSMArray);
            }
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_dir(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        HalSMArray* out;
        Dict* d;
        ui64 i;
        if (args->arr[0]->type==HalSMVariableType_str) {
            out=HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,U8H("replace")),HalSMVariable_init_str(hsmc->memory,U8H("index")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("rindex")),HalSMVariable_init_str(hsmc->memory,U8H("split")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("substring")),HalSMVariable_init_str(hsmc->memory,U8H("len"))},6);
        } else if (args->arr[0]->type==HalSMVariableType_HalSMArray) {
            out=HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,U8H("append")),HalSMVariable_init_str(hsmc->memory,U8H("len")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("clear")),HalSMVariable_init_str(hsmc->memory,U8H("index")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("insert")),HalSMVariable_init_str(hsmc->memory,U8H("pop")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("remove")),HalSMVariable_init_str(hsmc->memory,U8H("reverse")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("copy")),HalSMVariable_init_str(hsmc->memory,U8H("appendArray")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("slice")),HalSMVariable_init_str(hsmc->memory,U8H("set")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("join")),HalSMVariable_init_str(hsmc->memory,U8H("rindex"))},14);
        } else if (args->arr[0]->type==HalSMVariableType_HalSMDict) {
            out=HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init_str(hsmc->memory,U8H("append")),HalSMVariable_init_str(hsmc->memory,U8H("appendDict")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("len")),HalSMVariable_init_str(hsmc->memory,U8H("clear")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("indexByKey")),HalSMVariable_init_str(hsmc->memory,U8H("indexByValue")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("set")),HalSMVariable_init_str(hsmc->memory,U8H("pop")),
                                                                                              HalSMVariable_init_str(hsmc->memory,U8H("keyByIndex")),HalSMVariable_init_str(hsmc->memory,U8H("valueByIndex"))},10);
        } else if (args->arr[0]->type==HalSMVariableType_custom) {
            out=HalSMArray_init(hsmc->memory);
            d=((HalSMVariableCustom*)args->arr[0]->value)->type->funcs;
            for (i=0;i<d->size;i++) {
                HalSMArray_add(hsmc->memory,out,d->elements[i]->key);
            }
            d=((HalSMVariableCustom*)args->arr[0]->value)->type->vars;
            for (i=0;i<d->size;i++) {
                HalSMArray_add(hsmc->memory,out,d->elements[i]->key);
            }
        } else {
            out=HalSMArray_init(hsmc->memory);
        }
        return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_HalSMArray);
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_dict(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        if (args->arr[0]->type==HalSMVariableType_HalSMDict) {
            return HalSMVariable_init(hsmc->memory,DictCopy(hsmc->memory,(Dict*)args->arr[0]->value),HalSMVariableType_HalSMDict);
        } else if (args->arr[0]->type==HalSMVariableType_str) {
            HalStringFormatChar* temp=*(HalStringFormatChar**)args->arr[0]->value,*tempv;
            if (HSFCharCompare(temp[0],ASCIIH('{'))&&HSFCharCompare(temp[string_len(temp)-1],ASCIIH('}'))) {
                if (string_len(temp)==2) {
                    return HalSMVariable_init(hsmc->memory,DictInit(hsmc->memory),HalSMVariableType_HalSMDict);
                }
                tempv=HalSMMemory_malloc(hsmc->memory,string_len(temp)*sizeof(HalStringFormatChar)-sizeof(HalStringFormatChar));
                stringncpy(tempv,&temp[1],string_len(temp)-2);
                Dict* out=HalSMCompiler_getArgsDict(hsmc,tempv,vars);
                HalSMMemory_free(hsmc->memory,tempv);
                if (out==&NULLDict){return &null;}
                return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_HalSMDict);
            }
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_type(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if(args->size!=1)return &null;
    return HalSMVariable_init_str(hsmc->memory,HalSMVariableTypeName[args->arr[0]->type]);
}



HalSMVariable* HalSMCompiler_getNameFunction(HalSMCompiler* hsmc,HalStringFormatChar* l)
{
    HalSMArray* name=HalSMArray_init(hsmc->memory);
    HalStringFormatChar s;
    ui64 ll=string_len(l);
    l=HalSMCompiler_removeStartSpaces(hsmc->memory,l);
    l=HalSMCompiler_removeEndSpaces(hsmc->memory,l);
    for (ui64 i=0;i<ll;i++) {
        s=l[i];
        if (HSFCharCompare(s,ASCIIH('('))) {
            HalSMArray* out=HalSMArray_init(hsmc->memory);
            HalSMArray_add(hsmc->memory,out,HalSMVariable_init_str(hsmc->memory,HalSMCompiler_removeSpaces(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,name))));
            HalSMArray_add(hsmc->memory,out,HalSMVariable_init_str(hsmc->memory,SubString(hsmc->memory,l,i+1,string_len(l)-1)));
            return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_HalSMArray);
        }
        HalSMArray_add(hsmc->memory,name,HalSMVariable_init_char(hsmc->memory,s));
    }

    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("This is not Function")));
    return &null;
}

HalSMVariable* HalSMCompiler_isSetVar(HalSMCompiler* hsmc,HalStringFormatChar* l,Dict* vars)
{
    if (StringStartsWith(hsmc->memory,l,U8H("def "))||StringStartsWith(hsmc->memory,l,U8H("return "))||StringEndsWith(hsmc->memory,l,U8H("{"))){return &null;}
    HalStringFormatChar i;
    HalSMArray* n=HalSMArray_init(hsmc->memory);
    ui64 ll=string_len(l);
    /*if (StringIndexOf(hsmc->memory,l,U8H("++"))!=-1||StringIndexOf(hsmc->memory,l,U8H("--"))!=-1) {
        return &null;
    }*/

    ui64 isA=0,isF=0,isD=0;
    HalStringFormatChar isS=ASCIIH('n');

    for (ui64 ind=0;ind<ll;ind++) {
        i=l[ind];
        
        if (isA) {
            if (HSFCharCompare(i,ASCIIH('['))) {
                isA++;
            } else if (HSFCharCompare(i,ASCIIH(']'))) {
                isA--;
            }
        } else if (isF) {
            if (HSFCharCompare(i,ASCIIH('('))) {
                isF++;
            } else if (HSFCharCompare(i,ASCIIH(')'))) {
                isF--;
            }

        } else if (HSFCharCompare(isS,ASCIIH('n'))==0) {
            if (HSFCharCompare(i,isS)){isS=ASCIIH('n');}
        } else if (isD) {
            if (HSFCharCompare(i,ASCIIH('{'))) {
                isD++;
            } else if (HSFCharCompare(i,ASCIIH('}'))) {
                isD--;
            }
        } else if (HSFCharCompare(i,ASCIIH('\''))||HSFCharCompare(i,ASCIIH('"'))) {
            isS=i;
        } else if (HSFCharCompare(i,ASCIIH('['))) {
            isA++;
        } else if (HSFCharCompare(i,ASCIIH('{'))) {
            isD++;
        } else if (HSFCharCompare(i,ASCIIH('('))) {
            isF++;
        } else if (HSFCharCompare(i,ASCIIH('='))&&n->size>0) {
            if (ind>1&&HSFCharCompare(l[ind-1],ASCIIH('*'))&&HSFCharCompare(l[ind-2],ASCIIH('*'))) {
                HalSMArray_remove(hsmc->memory,n,n->size-1);
                HalSMArray_remove(hsmc->memory,n,n->size-1);
                l=ConcatenateStrings(hsmc->memory,ConcatenateStrings(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,n),U8H("**")),SubString(hsmc->memory,l,ind+1,string_len(l)));
            } else if (HSFCharCompare(l[ind-1],ASCIIH('+'))||HSFCharCompare(l[ind-1],ASCIIH('-'))||HSFCharCompare(l[ind-1],ASCIIH('*'))||HSFCharCompare(l[ind-1],ASCIIH('/'))||HSFCharCompare(l[ind-1],ASCIIH('%'))||HSFCharCompare(l[ind-1],ASCIIH('^'))||HSFCharCompare(l[ind-1],ASCIIH('|'))||HSFCharCompare(l[ind-1],ASCIIH('&'))) {
                HalStringFormatChar temp[2];
                temp[0]=l[ind-1];
                temp[1]=ASCIIH('\0');
                HalSMArray_remove(hsmc->memory,n,n->size-1);
                l=ConcatenateStrings(hsmc->memory,ConcatenateStrings(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,n),temp),SubString(hsmc->memory,l,ind+1,string_len(l)));
            } else {
                l=SubString(hsmc->memory,l,ind+1,string_len(l));
            }
            HalSMFunctionArray* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMFunctionArray));
            out->args=HalSMArray_init(hsmc->memory);
            out->type=HalSMFunctionArrayType_var;
            HalSMArray_add(hsmc->memory,out->args,HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,n)));
            HalSMArray_add(hsmc->memory,out->args,HalSMVariable_init_str(hsmc->memory,l));
            return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_HalSMFunctionArray);
        }
        HalSMArray_add(hsmc->memory,n,HalSMVariable_init_char(hsmc->memory,i));
    }
    return &null;
}

ui8 HalSMCompiler_isNull(HalStringFormatChar* text)
{
    HalStringFormatChar i;
    ui64 lt=string_len(text);
    for (ui64 ind=0;ind<lt;ind++) {
        i=text[ind];
        if (HSFCharCompare(i,ASCIIH(' '))==0&&HSFCharCompare(i,ASCIIH('\r'))==0){return 0;}
    }
    return 1;
}

HalStringFormatChar* HalSMCompiler_getPath(HalSMMemory* memory,HalStringFormatChar* a,HalStringFormatChar* b)
{
    if (string_len(a)==0){return b;}
    if (HSFCharCompare(a[string_len(a)-1],ASCIIH('/'))) {
        return ConcatenateStrings(memory,a,b);
    }
    return ConcatenateStrings(memory,a,ConcatenateStrings(memory,U8H("/"),b));
}


HalSMVariable* HalSMCompiler_compileList(HalSMCompiler* hsmc,ui8 isConsole,HalSMVariable* module,HalSMArray* allLines) {
    ui8 isFunc=0;
    ui8 isClass=0;
    HalSMVariable* func=&null;
    HalSMVariable* cls=&null;
    HalSMVariable* isSetVar;
    HalStringFormatChar* tabsS;
    ui64 tabsC;
    ui8 isNull=0;
    HalSMLocalFunction* f;
    HalSMVariable* fc=&null;
    HalSMArray* nameFunc;
    HalSMArray* spliti;
    HalStringFormatChar* joinSpliti;
    HalSMFunctionArray* resRunFunc=NOTHALSMNULLPOINTER;
    HalSMVariable* err;
    HalSMArray* allLinesTemp;
    HalStringFormatChar* l;
    HalSMVariable* funcColon=&null;
    HalSMArray* funcColonArgs=NOTHALSMNULLPOINTER;
    ui8 isFuncColon=0;
    HalSMVariable* groupFunctions=&null;
    HalSMVariable* groupFunc=&null;
    HalSMArray* arrayGroupFunc=HalSMArray_init(hsmc->memory);
    ui64 prevTabsC=0;

    //0.1.5
    HalSMVariable* isGOTO=&null;
    ui64 lastIndex=allLines->size-1;

    for (;hsmc->indexl<allLines->size&&hsmc->isRun==1;hsmc->indexl++) {
        hsmc->curInd=hsmc->ind;
        allLinesTemp=(HalSMArray*)allLines->arr[hsmc->indexl]->value;

        l=*(HalStringFormatChar**)allLinesTemp->arr[0]->value;
        tabsC=*(ui64*)allLinesTemp->arr[1]->value;
        tabsS=HalSMCompiler_removeEndSpaces(hsmc->memory,HalSMCompiler_removeStartSpaces(hsmc->memory,l));
        isSetVar=HalSMCompiler_isSetVar(hsmc,tabsS,&NULLDict);
        isNull=HalSMCompiler_isNull(tabsS);

        if(isNull) {
            goto NEXT_HALSM_COMPILE_LOOP;
        }

        if (tabsC!=prevTabsC&&!isNull) {
            groupFunctions=&null;
            groupFunc=&null;
            i64 sizeArrayGroupFunc=arrayGroupFunc->size;
            for (int indexGF=0;indexGF<sizeArrayGroupFunc;indexGF++) {
                if (*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value==tabsC||*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value+1==tabsC) {
                    groupFunctions=((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[0];
                    groupFunc=((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[1];
                    break;
                } else if (tabsC<*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value) {
                    HalSMArray_remove(hsmc->memory,arrayGroupFunc,arrayGroupFunc->size-1);
                }
            }
        }

        if (isClass) {
            if (tabsC==0&&!isNull) {
                if (isFunc) {
                    isFunc=0;
                    if (funcColon->type!=HalSMVariableType_HalSMNull) {
                        funcColon=&null;
                    }
                    PutDictElementToDict(hsmc,(((HalSMClass*)fc->value)->funcs),DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,((HalSMLocalFunction*)func->value)->name),func));
                    func=&null;

                    if (isFuncColon) {
                        isFuncColon=0;
                    }
                }
                isClass=0;
                isFunc=0;
                func=&null;
                cls=&null;
            } else if (tabsC==1&&!isNull) {
                if (isFunc) {
                    isFunc=0;
                    if (funcColon->type!=HalSMVariableType_HalSMNull) {
                        funcColon=&null;
                    }
                    PutDictElementToDict(hsmc,(((HalSMClass*)fc->value)->funcs),DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,((HalSMLocalFunction*)func->value)->name),func));
                    func=&null;

                    if (isFuncColon) {
                        isFuncColon=0;
                    }
                }

                if (StringStartsWith(hsmc->memory,tabsS,U8H("def "))) {
                    spliti=HalSMArray_split_str(hsmc->memory,tabsS,U8H("{"));
                    spliti=HalSMArray_slice(hsmc->memory,spliti,0,spliti->size-1);
                    joinSpliti=HalSMArray_join_str(hsmc->memory,spliti,U8H("{"));
                    spliti=HalSMArray_split_str(hsmc->memory,joinSpliti,U8H("def "));
                    spliti=HalSMArray_slice(hsmc->memory,spliti,1,spliti->size);
                    joinSpliti=HalSMArray_join_str(hsmc->memory,spliti,U8H("def "));
                    nameFunc=(HalSMArray*)HalSMCompiler_getNameFunction(hsmc,joinSpliti)->value;
                    f=HalSMLocalFunction_init(hsmc,*(HalStringFormatChar**)nameFunc->arr[0]->value,*(HalStringFormatChar**)nameFunc->arr[1]->value,module);
                    func=HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction);
                    PutDictElementToDict(hsmc,((HalSMClass*)cls->value)->funcs,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,f->name),func));
                    isFunc=1;
                } else if (isSetVar->type!=HalSMVariableType_HalSMNull) {
                    resRunFunc=(HalSMFunctionArray*)isSetVar->value;
                    ui8 isGet=0;
                    HalSMVariable* isGetV=HalSMCompiler_isGetU(hsmc,*(HalStringFormatChar**)resRunFunc->args->arr[0]->value,&isGet,1,((HalSMClass*)cls->value)->vars);
                    if (isGet) {
                        HalSMVariable* ret=HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)resRunFunc->args->arr[1]->value,((HalSMClass*)cls->value)->vars)->arr[0];
                        isGetV->value=ret->value;
                        isGetV->type=ret->type;
                    }
                }
            } else if (isFunc) {
                if (StringStartsWith(hsmc->memory,tabsS,U8H("return "))) {
                    spliti=HalSMArray_split_str(hsmc->memory,tabsS,U8H("return "));
                    joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,1,spliti->size),U8H("return "));
                    HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMReturn_init(hsmc->memory,joinSpliti,((HalSMLocalFunction*)func->value)->id),HalSMVariableType_HalSMReturn);
                    if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                        HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                        if (arrGF->size==0) {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                        } else {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                        }
                    } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                        HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                        HalSMVariable* funcElemVar;
                        for (ui64 i=0;i<tabsC-2;i++) {
                            funcElemVar=funcElem->arr[funcElem->size-1];
                            if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                            } else {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                            }
                        }
                        HalSMArray_add(hsmc->memory,funcElem,ret);
                    } else {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                    }
                } else if (StringStartsWith(hsmc->memory,tabsS,U8H("goto "))) {
                    joinSpliti=HalSMCompiler_removeSpaces(hsmc->memory,*(HalStringFormatChar**)HalSMArray_split_str(hsmc->memory,tabsS,U8H("goto "))->arr[1]->value);
                    HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMGOTO_init(hsmc->memory,joinSpliti),HalSMVariableType_HalSMGOTO);
                    if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                        HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                        if (arrGF->size==0) {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                        } else {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                        }
                    } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                        HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                        HalSMVariable* funcElemVar;
                        for (ui64 i=0;i<tabsC-1;i++) {
                            funcElemVar=funcElem->arr[funcElem->size-1];
                            if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                            } else {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                            }
                        }
                        HalSMArray_add(hsmc->memory,funcElem,ret);
                    } else {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                    }
                } else if (isSetVar->type!=HalSMVariableType_HalSMNull) {
                    resRunFunc=(HalSMFunctionArray*)isSetVar->value;
                    HalSMVariable* valueVar=HalSMVariable_init(hsmc->memory,&hnull,HalSMVariableType_HalSMNull);
                    HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMSetVar_init(hsmc->memory,*(HalStringFormatChar**)resRunFunc->args->arr[0]->value,*(HalStringFormatChar**)resRunFunc->args->arr[1]->value,valueVar),HalSMVariableType_HalSMSetVar);
                    if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                        if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,resRunFunc->args->arr[0])!=-1) {
                            HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){resRunFunc->args->arr[0],resRunFunc->args->arr[2]},2),HalSMVariableType_HalSMArray));
                        } else {
                            HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                            if (arrGF->size==0) {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                            }
                        }
                    } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                        HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                        HalSMVariable* funcElemVar;
                        for (ui64 i=0;i<tabsC-2;i++) {
                            funcElemVar=funcElem->arr[funcElem->size-1];
                            if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                            } else {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                            }
                        }
                        HalSMArray_add(hsmc->memory,funcElem,ret);
                    } else {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                    }
                } else if (HSFCharCompare(tabsS[string_len(tabsS)-1],ASCIIH('{'))) {
                    i64 index=StringIndexOf(hsmc->memory,tabsS,U8H("("));
                    if (index>0) {
                        //HalStringFormatChar* nameFunction=HalSMCompiler_removeSpaces(hsmc->memory,SubString(hsmc->memory,tabsS,0,index));
                        spliti=HalSMArray_split_str(hsmc->memory,tabsS,U8H("{"));
                        joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,0,spliti->size-1),U8H("{"));
                        nameFunc=(HalSMArray*)HalSMCompiler_getNameFunction(hsmc,joinSpliti)->value;
                        
                        HalSMVariable* obuff=nameFunc->arr[0];
                        if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1) {
                            HalSMFunctionC* fu=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value->value;
                            HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                            HalSMArray_add(hsmc->memory,funcArgs,nameFunc->arr[1]);
                            
                            f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction));
                            if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value==tabsC) {
                                if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,HalSMVariable_init(hsmc->memory,fu,HalSMVariableType_HalSMFunctionC))!=-1) {
                                    HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init(hsmc->memory,fu,HalSMVariableType_HalSMFunctionC),HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                                } else {
                                    err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                                    if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                    groupFunctions=&null;
                                    HalSMArray_remove(hsmc->memory,arrayGroupFunc,arrayGroupFunc->size-1);
                                }
                            } else if (DictElementIndexByKey(hsmc,hsmc->groupFunctions,obuff)!=-1) {
                                if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                                    HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC);
                                    HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                    if (arrGF->size==0) {
                                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                                    } else {
                                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                                    }
                                } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                                    HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                                    HalSMVariable* funcElemVar;
                                    for (ui64 i=0;i<tabsC-2;i++) {
                                        funcElemVar=funcElem->arr[funcElem->size-1];
                                        if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                            funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                        } else {
                                            funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                        }
                                    }
                                    HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                } else {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                }
                                groupFunctions=DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value;
                                groupFunc=HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init(hsmc->memory,fu,HalSMVariableType_HalSMFunctionC),HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray);
                                HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,HalSMArray_init(hsmc->memory),HalSMVariableType_HalSMArray));
                                HalSMArray_add(hsmc->memory,arrayGroupFunc,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){groupFunctions,groupFunc,HalSMVariable_init_unsigned_int(hsmc->memory,tabsC)},3),HalSMVariableType_HalSMArray));
                            } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                                HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                                HalSMVariable* funcElemVar;
                                for (ui64 i=0;i<tabsC-2;i++) {
                                    funcElemVar=funcElem->arr[funcElem->size-1];
                                    if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                    } else {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                    }
                                }
                                HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                                HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                if (arrGF->size==0) {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                } else {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                }
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            }
                            isFuncColon=1;
                        } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,obuff)!=-1) {
                            HalSMLocalFunction* fu=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value->value;
                            HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init_str(hsmc->memory,SubString(hsmc->memory,tabsS,index+1,string_len(tabsS)-2)));

                            f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction));
                            isFuncColon=1;
                            if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                                HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                                HalSMVariable* funcElemVar;
                                for (ui64 i=0;i<tabsC-2;i++) {
                                    funcElemVar=funcElem->arr[funcElem->size-1];
                                    if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                    } else {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                    }
                                }
                                HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                            } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                                HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                if (arrGF->size==0) {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                                } else {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                                }
                            } else {HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));}
                            isFuncColon=1;
                        }
                    } else {
                        HalStringFormatChar* nameFunction=HalSMCompiler_removeSpaces(hsmc->memory,SubString(hsmc->memory,tabsS,0,string_len(tabsS)-1));
                        HalSMVariable* obuff=HalSMVariable_init_str(hsmc->memory,nameFunction);
                        if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1) {
                            HalSMVariable* fuvar=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value;
                            HalSMFunctionC* fu=fuvar->value;
                            HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init_str(hsmc->memory,U8H("")));
                            
                            f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction));
                            if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value==tabsC) {
                                if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,fuvar)!=-1) {
                                    HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){fuvar,HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                                } else {
                                    //HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                    groupFunctions=&null;
                                    HalSMArray_remove(hsmc->memory,arrayGroupFunc,0);
                                }
                            } else if (DictElementIndexByKey(hsmc,hsmc->groupFunctions,obuff)!=-1) {
                                if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                                    HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC);
                                    HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                    if (arrGF->size==0) {
                                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                                    } else {
                                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                                    }
                                } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                                    HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                                    HalSMVariable* funcElemVar;
                                    for (ui64 i=0;i<tabsC-1;i++) {
                                        funcElemVar=funcElem->arr[funcElem->size-1];
                                        if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                            funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                        } else {
                                            funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                        }
                                    }
                                    HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                } else {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                }
                                groupFunctions=DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value;
                                groupFunc=HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init(hsmc->memory,fu,HalSMVariableType_HalSMFunctionC),HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray);
                                HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,HalSMArray_init(hsmc->memory),HalSMVariableType_HalSMArray));
                                HalSMArray_add(hsmc->memory,arrayGroupFunc,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){groupFunctions,groupFunc,HalSMVariable_init_unsigned_int(hsmc->memory,tabsC)},3),HalSMVariableType_HalSMArray));
                            } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                                HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                                HalSMVariable* funcElemVar;
                                for (ui64 i=0;i<tabsC-1;i++) {
                                    funcElemVar=funcElem->arr[funcElem->size-1];
                                    if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                    } else {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                    }
                                }
                                HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                                HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                if (arrGF->size==0) {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                } else {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                }
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            }
                            isFuncColon=1;
                        } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,obuff)!=-1) {
                            HalSMLocalFunction* fu=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value->value;
                            HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init_str(hsmc->memory,U8H("")));

                            f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction));
                            isFuncColon=1;
                            if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                                HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                                HalSMVariable* funcElemVar;
                                for (ui64 i=0;i<tabsC-1;i++) {
                                    funcElemVar=funcElem->arr[funcElem->size-1];
                                    if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                    } else {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                    }
                                }
                                HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                            } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                                HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                if (arrGF->size==0) {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                                } else {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                                }
                            } else {HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));}
                            isFuncColon=1;
                        } else {
                            HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMGOTOSector_init(hsmc->memory,nameFunction,0,NOTHALSMNULLPOINTER),HalSMVariableType_HalSMGOTOSector);
                            if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                                if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,resRunFunc->args->arr[0])!=-1) {
                                    HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){resRunFunc->args->arr[0],resRunFunc->args->arr[2]},2),HalSMVariableType_HalSMArray));
                                } else {
                                    HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                    if (arrGF->size==0) {
                                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                                    } else {
                                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                                    }
                                }
                            } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                                HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                                HalSMVariable* funcElemVar;
                                for (ui64 i=0;i<tabsC-1;i++) {
                                    funcElemVar=funcElem->arr[funcElem->size-1];
                                    if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                    } else {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                    }
                                }
                                HalSMArray_add(hsmc->memory,funcElem,ret);
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                            }
                        }
                    }
                } else if (HSFCompare(tabsS,U8H("break"))) {
                    HalSMVariable* ret=HalSMVariable_init(hsmc->memory,&hbreak,HalSMVariableType_HalSMBreak);
                    if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                        HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                        if (arrGF->size==0) {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                        } else {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                        }
                    } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                        HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                        HalSMVariable* funcElemVar;
                        for (ui64 i=0;i<tabsC-1;i++) {
                            funcElemVar=funcElem->arr[funcElem->size-1];
                            if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                            } else {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                            }
                        }
                        HalSMArray_add(hsmc->memory,funcElem,ret);
                    } else {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                    }
                } else if(HSFCompare(tabsS,U8H("}"))) {

                } else {
                    if (groupFunc->type!=HalSMVariableType_HalSMNull) {
                        if (HSFCharCompare(tabsS[string_len(tabsS)-1],ASCIIH(')'))&&StringIndexOf(hsmc->memory,tabsS,U8H("("))!=-1&&StringIndexOf(hsmc->memory,tabsS,U8H("["))==-1&&StringIndexOf(hsmc->memory,tabsS,U8H("]"))==-1&&StringIndexOf(hsmc->memory,tabsS,U8H("."))==-1) {
                            i64 index=StringIndexOf(hsmc->memory,tabsS,U8H("("));
                            HalStringFormatChar* nameFunction=HalSMCompiler_removeSpaces(hsmc->memory,SubString(hsmc->memory,tabsS,0,index));
                            HalSMVariable* obuff=HalSMVariable_init_str(hsmc->memory,nameFunction);
                            if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1&&groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value==tabsC) {
                                HalSMVariable* fuvar=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value;
                                HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                                HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init_str(hsmc->memory,SubString(hsmc->memory,tabsS,index+1,string_len(tabsS)-1)));
                                HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module),HalSMVariableType_HalSMLocalFunction));
                                if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,fuvar)!=-1) {
                                    HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){fuvar,HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                                    groupFunc=&null; 
                                    goto NEXT_HALSM_COMPILE_LOOP;
                                }
                            }
                        }
                        if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value==tabsC) {
                            groupFunc=&null;
                        }
                    }
                    HalSMVariable* ret;
                    HalSMArray* array=HalSMArray_init(hsmc->memory);
                    HalSMFunctionArray* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMFunctionArray));
                    out->type=HalSMFunctionArrayType_function;
                    HalSMArray_add(hsmc->memory,array,HalSMVariable_init(hsmc->memory,HalSMVar_init(hsmc->memory,l),HalSMVariableType_HalSMVar));
                    out->args=array;
                    ret=HalSMVariable_init(hsmc->memory,out,HalSMVariableType_HalSMFunctionArray);
                    if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                        HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                        if (arrGF->size==0) {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,ret);
                        } else {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                        }
                    } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                        HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                        HalSMVariable* funcElemVar;
                        for (ui64 i=0;i<tabsC-2;i++) {
                            funcElemVar=funcElem->arr[funcElem->size-1];
                            if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                            } else {
                                funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                            }
                        }
                        HalSMArray_add(hsmc->memory,funcElem,ret);
                    } else {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                    }
                }
            }
        }

        if (isFunc&&!isClass) {
            if (tabsC==0&&!isNull) {
                isFunc=0;
                /*if (funcColon->type!=HalSMVariableType_HalSMNull) {
                    if (funcColon->type==HalSMVariableType_HalSMFunctionC) {
                        if (groupFunctions->type==HalSMVariableType_HalSMNull) {
                            err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)funcColon->value,funcColonArgs,&NULLDict);
                            if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                            HalSMMemory_free(hsmc->memory,funcColon);
                        }
                    } else if (funcColon->type==HalSMVariableType_HalSMLocalFunction) {
                        err=HalSMLocalFunction_run((HalSMLocalFunction*)funcColon->value,funcColonArgs,&NULLDict);
                        if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                        HalSMMemory_free(hsmc->memory,funcColon);
                    }
                    funcColon=&null;
                }*/
                func=&null;

                /*if (isFuncColon) {
                    isFuncColon=0;
                }*/
            } else if (StringStartsWith(hsmc->memory,tabsS,U8H("return "))) {
                spliti=HalSMArray_split_str(hsmc->memory,tabsS,U8H("return "));
                joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,1,spliti->size),U8H("return "));
                HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMReturn_init(hsmc->memory,joinSpliti,((HalSMLocalFunction*)func->value)->id),HalSMVariableType_HalSMReturn);
                if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                    HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                    if (arrGF->size==0) {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                    } else {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                    }
                } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                    HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                    HalSMVariable* funcElemVar;
                    for (ui64 i=0;i<tabsC-1;i++) {
                        funcElemVar=funcElem->arr[funcElem->size-1];
                        if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                        } else {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                        }
                    }
                    HalSMArray_add(hsmc->memory,funcElem,ret);
                } else {
                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                }
            } else if (StringStartsWith(hsmc->memory,tabsS,U8H("goto "))) {
                joinSpliti=HalSMCompiler_removeSpaces(hsmc->memory,*(HalStringFormatChar**)HalSMArray_split_str(hsmc->memory,tabsS,U8H("goto "))->arr[1]->value);
                HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMGOTO_init(hsmc->memory,joinSpliti),HalSMVariableType_HalSMGOTO);
                if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                    HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                    if (arrGF->size==0) {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                    } else {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                    }
                } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                    HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                    HalSMVariable* funcElemVar;
                    for (ui64 i=0;i<tabsC-1;i++) {
                        funcElemVar=funcElem->arr[funcElem->size-1];
                        if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                        } else {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                        }
                    }
                    HalSMArray_add(hsmc->memory,funcElem,ret);
                } else {
                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                }
            } else if (isSetVar->type!=HalSMVariableType_HalSMNull) {
                resRunFunc=(HalSMFunctionArray*)isSetVar->value;
                HalSMVariable* valueVar=HalSMVariable_init(hsmc->memory,&hnull,HalSMVariableType_HalSMNull);
                HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMSetVar_init(hsmc->memory,*(HalStringFormatChar**)resRunFunc->args->arr[0]->value,*(HalStringFormatChar**)resRunFunc->args->arr[1]->value,valueVar),HalSMVariableType_HalSMSetVar);

                if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                    if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,resRunFunc->args->arr[0])!=-1) {
                        HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){resRunFunc->args->arr[0],resRunFunc->args->arr[2]},2),HalSMVariableType_HalSMArray));
                    } else {
                        HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                        if (arrGF->size==0) {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                        } else {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                        }
                    }
                } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                    HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                    HalSMVariable* funcElemVar;
                    for (ui64 i=0;i<tabsC-1;i++) {
                        funcElemVar=funcElem->arr[funcElem->size-1];
                        if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                        } else {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                        }
                    }
                    HalSMArray_add(hsmc->memory,funcElem,ret);
                } else {
                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                }
            } else if (HSFCharCompare(tabsS[string_len(tabsS)-1],ASCIIH('{'))) {
                i64 index=StringIndexOf(hsmc->memory,tabsS,U8H("("));
                if (index>0) {
                    //HalStringFormatChar* nameFunction=HalSMCompiler_removeSpaces(hsmc->memory,SubString(hsmc->memory,tabsS,0,index));
                    spliti=HalSMArray_split_str(hsmc->memory,tabsS,U8H("{"));
                    joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,0,spliti->size-1),U8H("{"));
                    nameFunc=(HalSMArray*)HalSMCompiler_getNameFunction(hsmc,joinSpliti)->value;

                    HalSMVariable* obuff=nameFunc->arr[0];
                    if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1) {
                        HalSMVariable* fuvar=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value;
                        HalSMFunctionC* fu=fuvar->value;
                        HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                        HalSMArray_add(hsmc->memory,funcArgs,nameFunc->arr[1]);

                        f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                        HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction));
                        if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value==tabsC) {
                            if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,fuvar)!=-1) {
                                HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){fuvar,HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                            } else {
                                //HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                groupFunctions=&null;
                                HalSMArray_remove(hsmc->memory,arrayGroupFunc,0);
                            }
                        } else if (DictElementIndexByKey(hsmc,hsmc->groupFunctions,obuff)!=-1) {
                            if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                                HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC);
                                HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                if (arrGF->size==0) {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                                } else {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                                }
                            } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                                HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                                HalSMVariable* funcElemVar;
                                for (ui64 i=0;i<tabsC-1;i++) {
                                    funcElemVar=funcElem->arr[funcElem->size-1];
                                    if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                    } else {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                    }
                                }
                                HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            }
                            groupFunctions=DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value;
                            groupFunc=HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init(hsmc->memory,fu,HalSMVariableType_HalSMFunctionC),HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,HalSMArray_init(hsmc->memory),HalSMVariableType_HalSMArray));
                            HalSMArray_add(hsmc->memory,arrayGroupFunc,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){groupFunctions,groupFunc,HalSMVariable_init_unsigned_int(hsmc->memory,tabsC)},3),HalSMVariableType_HalSMArray));
                        } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                            HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                            HalSMVariable* funcElemVar;
                            for (ui64 i=0;i<tabsC-1;i++) {
                                funcElemVar=funcElem->arr[funcElem->size-1];
                                if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                } else {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                }
                            }
                            HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                        } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                            HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                            if (arrGF->size==0) {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            }
                        } else {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                        }
                        isFuncColon=1;
                    } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,obuff)!=-1) {
                        HalSMLocalFunction* fu=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value->value;
                        HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                        HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init_str(hsmc->memory,SubString(hsmc->memory,tabsS,index+1,string_len(tabsS)-2)));

                        f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                        HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction));
                        isFuncColon=1;
                        if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                            HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                            HalSMVariable* funcElemVar;
                            for (ui64 i=0;i<tabsC-1;i++) {
                                funcElemVar=funcElem->arr[funcElem->size-1];
                                if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                } else {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                }
                            }
                            HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                        } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                            HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                            if (arrGF->size==0) {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                            }
                        } else {HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));}
                        isFuncColon=1;
                    }
                } else {
                    HalStringFormatChar* nameFunction=HalSMCompiler_removeSpaces(hsmc->memory,SubString(hsmc->memory,tabsS,0,string_len(tabsS)-1));
                    HalSMVariable* obuff=HalSMVariable_init_str(hsmc->memory,nameFunction);
                    if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1) {
                        HalSMVariable* fuvar=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value;
                        HalSMFunctionC* fu=fuvar->value;
                        HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                        HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init_str(hsmc->memory,U8H("")));

                        f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                        HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction));
                        if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value==tabsC) {
                            if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,fuvar)!=-1) {
                                HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){fuvar,HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                            } else {
                                //HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                                groupFunctions=&null;
                                HalSMArray_remove(hsmc->memory,arrayGroupFunc,0);
                            }
                        } else if (DictElementIndexByKey(hsmc,hsmc->groupFunctions,obuff)!=-1) {
                            if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                                HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC);
                                HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                if (arrGF->size==0) {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                                } else {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                                }
                            } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                                HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                                HalSMVariable* funcElemVar;
                                for (ui64 i=0;i<tabsC-1;i++) {
                                    funcElemVar=funcElem->arr[funcElem->size-1];
                                    if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                    } else {
                                        funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                    }
                                }
                                HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            }
                            groupFunctions=DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value;
                            groupFunc=HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init(hsmc->memory,fu,HalSMVariableType_HalSMFunctionC),HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,HalSMArray_init(hsmc->memory),HalSMVariableType_HalSMArray));
                            HalSMArray_add(hsmc->memory,arrayGroupFunc,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){groupFunctions,groupFunc,HalSMVariable_init_unsigned_int(hsmc->memory,tabsC)},3),HalSMVariableType_HalSMArray));
                        } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                            HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                            HalSMVariable* funcElemVar;
                            for (ui64 i=0;i<tabsC-1;i++) {
                                funcElemVar=funcElem->arr[funcElem->size-1];
                                if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                } else {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                }
                            }
                            HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                        } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                            HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                            if (arrGF->size==0) {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                            }
                        } else {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFuncC));
                        }
                        isFuncColon=1;
                    } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,obuff)!=-1) {
                        HalSMLocalFunction* fu=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value->value;
                        HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                        HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init_str(hsmc->memory,U8H("")));

                        f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                        HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction));
                        isFuncColon=1;
                        if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                            HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                            HalSMVariable* funcElemVar;
                            for (ui64 i=0;i<tabsC-1;i++) {
                                funcElemVar=funcElem->arr[funcElem->size-1];
                                if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                } else {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                }
                            }
                            HalSMArray_add(hsmc->memory,funcElem,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                        } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                            HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                            if (arrGF->size==0) {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                            } else {
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));
                            }
                        } else {HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFunc_init(hsmc->memory,fu,funcArgs),HalSMVariableType_HalSMRunFunc));}
                        isFuncColon=1;
                    } else {
                        HalSMVariable* ret=HalSMVariable_init(hsmc->memory,HalSMGOTOSector_init(hsmc->memory,nameFunction,0,NOTHALSMNULLPOINTER),HalSMVariableType_HalSMGOTOSector);
                        if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                            if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,resRunFunc->args->arr[0])!=-1) {
                                HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){resRunFunc->args->arr[0],resRunFunc->args->arr[2]},2),HalSMVariableType_HalSMArray));
                            } else {
                                HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                                if (arrGF->size==0) {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                                } else {
                                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                                }
                            }
                        } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                            HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                            HalSMVariable* funcElemVar;
                            for (ui64 i=0;i<tabsC-1;i++) {
                                funcElemVar=funcElem->arr[funcElem->size-1];
                                if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                                } else {
                                    funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                                }
                            }
                            HalSMArray_add(hsmc->memory,funcElem,ret);
                        } else {
                            HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                        }
                    }
                }
            } else if (HSFCompare(tabsS,U8H("break"))) {
                HalSMVariable* ret=HalSMVariable_init(hsmc->memory,&hbreak,HalSMVariableType_HalSMBreak);
                if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                    HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                    if (arrGF->size==0) {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value))->func,ret);
                    } else {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                    }
                } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                    HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                    HalSMVariable* funcElemVar;
                    for (ui64 i=0;i<tabsC-1;i++) {
                        funcElemVar=funcElem->arr[funcElem->size-1];
                        if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                        } else {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                        }
                    }
                    HalSMArray_add(hsmc->memory,funcElem,ret);
                } else {
                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                }
            } else if(HSFCompare(tabsS,U8H("}"))) {

            } else {
                if (groupFunc->type!=HalSMVariableType_HalSMNull) {
                    if (HSFCharCompare(tabsS[string_len(tabsS)-1],ASCIIH(')'))&&StringIndexOf(hsmc->memory,tabsS,U8H("("))!=-1&&StringIndexOf(hsmc->memory,tabsS,U8H("["))==-1&&StringIndexOf(hsmc->memory,tabsS,U8H("]"))==-1&&StringIndexOf(hsmc->memory,tabsS,U8H("."))==-1) {
                        i64 index=StringIndexOf(hsmc->memory,tabsS,U8H("("));
                        HalStringFormatChar* nameFunction=HalSMCompiler_removeSpaces(hsmc->memory,SubString(hsmc->memory,tabsS,0,index));
                        HalSMVariable* obuff=HalSMVariable_init_str(hsmc->memory,nameFunction);
                        if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1&&groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value==tabsC) {
                            HalSMVariable* fuvar=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value;
                            HalSMArray* funcArgs=HalSMArray_init(hsmc->memory);
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init_str(hsmc->memory,SubString(hsmc->memory,tabsS,index+1,string_len(tabsS)-1)));
                            HalSMArray_add(hsmc->memory,funcArgs,HalSMVariable_init(hsmc->memory,HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module),HalSMVariableType_HalSMLocalFunction));
                            if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,fuvar)!=-1) {
                                HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){fuvar,HalSMVariable_init(hsmc->memory,funcArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                                groupFunc=&null; 
                                goto NEXT_HALSM_COMPILE_LOOP;
                            }
                        }
                    }
                    if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value==tabsC) {
                        groupFunc=&null;
                    }
                }
                HalSMVariable* ret;
                HalSMArray* array=HalSMArray_init(hsmc->memory);
                HalSMFunctionArray* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMFunctionArray));
                out->type=HalSMFunctionArrayType_function;
                HalSMArray_add(hsmc->memory,array,HalSMVariable_init(hsmc->memory,HalSMVar_init(hsmc->memory,l),HalSMVariableType_HalSMVar));
                out->args=array;
                ret=HalSMVariable_init(hsmc->memory,out,HalSMVariableType_HalSMFunctionArray);
                if (groupFunctions->type!=HalSMVariableType_HalSMNull&&*(ui64*)((HalSMArray*)arrayGroupFunc->arr[arrayGroupFunc->size-1]->value)->arr[2]->value<tabsC) {
                    HalSMArray* arrGF=(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value;
                    if (arrGF->size==0) {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[1]->value)->func,ret);
                    } else {
                        HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)((HalSMArray*)((HalSMArray*)arrGF->arr[arrGF->size-1]->value)->arr[1]->value)->arr[1]->value)->func,ret);
                    }
                } else if (funcColon->type!=HalSMVariableType_HalSMNull||isFuncColon) {
                    HalSMArray* funcElem=((HalSMLocalFunction*)func->value)->func;
                    HalSMVariable* funcElemVar;
                    for (ui64 i=0;i<tabsC-1;i++) {
                        funcElemVar=funcElem->arr[funcElem->size-1];
                        if (funcElemVar->type==HalSMVariableType_HalSMRunFuncC) {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFuncC*)funcElemVar->value)->args->arr[1]->value)->func;
                        } else {
                            funcElem=((HalSMLocalFunction*)((HalSMRunFunc*)funcElemVar->value)->args->arr[1]->value)->func;
                        }
                    }
                    HalSMArray_add(hsmc->memory,funcElem,ret);
                } else {
                    HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,ret);
                }
            }
        }

        if (!isFunc&&!isClass) {
            if (isGOTO->type==HalSMVariableType_HalSMNull&&StringStartsWith(hsmc->memory,tabsS,U8H("import "))) {
                spliti=HalSMArray_split_str(hsmc->memory,l,U8H("import "));
                HalStringFormatChar* pathModule=StringReplace(hsmc->memory,StringReplace(hsmc->memory,HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,1,spliti->size),U8H("import ")),U8H("\n"),U8H("")),U8H("\r"),U8H(""));
                spliti=HalSMArray_split_str(hsmc->memory,pathModule,U8H("/"));
                HalSMVariable* nameModule=HalSMVariable_init_str(hsmc->memory,*(HalStringFormatChar**)spliti->arr[spliti->size-1]->value);
                HalSMVariable* module=hsmc->HalSMLoadModule(hsmc,*(HalStringFormatChar**)nameModule->value);
                if (module->type!=HalSMVariableType_HalSMCModule&&module->type!=HalSMVariableType_HalSMModule) {
                    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,ConcatenateStrings(hsmc->memory,U8H("Module with name "),ConcatenateStrings(hsmc->memory,*(HalStringFormatChar**)nameModule->value,U8H(" Not Found")))));
                    return &hvoid;
                } else {
                    PutDictElementToDict(hsmc,hsmc->modules,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,*(HalStringFormatChar**)nameModule->value),module));
                }
            } else if (isGOTO->type==HalSMVariableType_HalSMNull&&StringStartsWith(hsmc->memory,tabsS,U8H("from "))&&StringIndexOf(hsmc->memory,tabsS,U8H(" import "))!=-1) {
                spliti=HalSMArray_split_str(hsmc->memory,l,U8H("from "));
                joinSpliti=StringReplace(hsmc->memory,StringReplace(hsmc->memory,*(HalStringFormatChar**)spliti->arr[1]->value,U8H("\n"),U8H("")),U8H("\r"),U8H(""));
                spliti=HalSMArray_split_str(hsmc->memory,joinSpliti,U8H(" import "));
                HalStringFormatChar* pathModule=*(HalStringFormatChar**)spliti->arr[0]->value;
                HalSMVariable* nameImport=HalSMVariable_init_str(hsmc->memory,*(HalStringFormatChar**)spliti->arr[1]->value);
                spliti=HalSMArray_split_str(hsmc->memory,pathModule,U8H("/"));
                HalSMVariable* nameModule=HalSMVariable_init_str(hsmc->memory,*(HalStringFormatChar**)spliti->arr[spliti->size-1]->value);

                //TODO fix find module
                if (DictElementIndexByKey(hsmc,hsmc->modules,nameModule)==-1) {
                    HalSMVariable* module=hsmc->HalSMLoadModule(hsmc,*(HalStringFormatChar**)nameModule->value);
                    if (module->type!=HalSMVariableType_HalSMCModule&&module->type!=HalSMVariableType_HalSMModule) {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,ConcatenateStrings(hsmc->memory,U8H("Module with name "),ConcatenateStrings(hsmc->memory,*(HalStringFormatChar**)nameModule->value,U8H(" Not Found")))));
                        return &hvoid;
                    } else {
                        PutDictElementToDict(hsmc,hsmc->modules,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,*(HalStringFormatChar**)nameModule->value),module));
                    }
                }

                if (DictElementIndexByKey(hsmc,hsmc->modules,nameModule)!=-1) {
                    HalSMVariable* module=DictElementFindByKey(hsmc,hsmc->modules,nameModule)->value;
                    if (module->type==HalSMVariableType_HalSMCModule) {
                        HalSMCModule* cmodule=(HalSMCModule*)module->value;
                        if (HSFCompare(*(HalStringFormatChar**)nameImport->value,U8H("*"))) {
                            ui64 a;
                            for (a=0;a<cmodule->classes->size;a++) {
                                PutDictElementToDict(hsmc,hsmc->classes,cmodule->classes->elements[a]);
                            }
                            for (a=0;a<cmodule->lfuncs->size;a++) {
                                PutDictElementToDict(hsmc,hsmc->functions,cmodule->lfuncs->elements[a]);
                            }
                            for (a=0;a<cmodule->vrs->size;a++) {
                                PutDictElementToDict(hsmc,hsmc->variables,cmodule->vrs->elements[a]);
                            }
                        } else if (DictElementIndexByKey(hsmc,cmodule->classes,nameImport)!=-1) {
                            PutDictElementToDict(hsmc,hsmc->classes,DictElementFindByKey(hsmc,cmodule->classes,nameImport));
                        } else if (DictElementIndexByKey(hsmc,cmodule->lfuncs,nameImport)!=-1) {
                            PutDictElementToDict(hsmc,hsmc->functions,DictElementFindByKey(hsmc,cmodule->lfuncs,nameImport));
                        } else if (DictElementIndexByKey(hsmc,cmodule->vrs,nameImport)!=-1) {
                            PutDictElementToDict(hsmc,hsmc->variables,DictElementFindByKey(hsmc,cmodule->vrs,nameImport));
                        } else {
                            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,ConcatenateStrings(hsmc->memory,U8H("Not found element "),ConcatenateStrings(hsmc->memory,*(HalStringFormatChar**)nameImport->value,ConcatenateStrings(hsmc->memory,U8H(" from CModule "),cmodule->name)))));
                        }
                    } else if (module->type==HalSMVariableType_HalSMModule) {
                        HalSMModule* mmodule=(HalSMModule*)module->value;
                        if (HSFCompare(*(HalStringFormatChar**)nameImport->value,U8H("*"))) {
                            ui64 a;
                            for (a=0;a<mmodule->hsmc->classes->size;a++) {
                                PutDictElementToDict(hsmc,hsmc->classes,mmodule->hsmc->classes->elements[a]);
                            }
                            for (a=0;a<mmodule->hsmc->localFunctions->size;a++) {
                                PutDictElementToDict(hsmc,hsmc->localFunctions,mmodule->hsmc->localFunctions->elements[a]);
                            }
                            for (a=0;a<mmodule->hsmc->variables->size;a++) {
                                PutDictElementToDict(hsmc,hsmc->variables,mmodule->hsmc->variables->elements[a]);
                            }
                            for (a=0;a<mmodule->hsmc->functions->size;a++) {
                                PutDictElementToDict(hsmc,hsmc->functions,mmodule->hsmc->functions->elements[a]);
                            }
                            for (a=0;a<mmodule->hsmc->modules->size;a++) {
                                PutDictElementToDict(hsmc,hsmc->modules,mmodule->hsmc->modules->elements[a]);
                            }
                        } else if (DictElementIndexByKey(hsmc,mmodule->hsmc->classes,nameImport)!=-1) {
                            PutDictElementToDict(hsmc,hsmc->classes,DictElementFindByKey(hsmc,mmodule->hsmc->classes,nameImport));
                        } else if (DictElementIndexByKey(hsmc,mmodule->hsmc->localFunctions,nameImport)!=-1) {
                            PutDictElementToDict(hsmc,hsmc->localFunctions,DictElementFindByKey(hsmc,mmodule->hsmc->localFunctions,nameImport));
                        } else if (DictElementIndexByKey(hsmc,mmodule->hsmc->variables,nameImport)!=-1) {
                            PutDictElementToDict(hsmc,hsmc->variables,DictElementFindByKey(hsmc,mmodule->hsmc->variables,nameImport));
                        } else if (DictElementIndexByKey(hsmc,mmodule->hsmc->functions,nameImport)!=-1) {
                            PutDictElementToDict(hsmc,hsmc->functions,DictElementFindByKey(hsmc,mmodule->hsmc->functions,nameImport));
                        } else if (DictElementIndexByKey(hsmc,mmodule->hsmc->modules,nameImport)!=-1) {
                            PutDictElementToDict(hsmc,hsmc->modules,DictElementFindByKey(hsmc,mmodule->hsmc->modules,nameImport));
                        } else {
                            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,ConcatenateStrings(hsmc->memory,U8H("Not found element "),ConcatenateStrings(hsmc->memory,*(HalStringFormatChar**)nameImport->value,ConcatenateStrings(hsmc->memory,U8H(" from Module "),mmodule->name)))));
                        }
                    }
                }
            } else if (StringStartsWith(hsmc->memory,tabsS,U8H("def "))) {
                spliti=HalSMArray_split_str(hsmc->memory,tabsS,U8H("{"));
                joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,0,spliti->size-1),U8H("{"));
                spliti=HalSMArray_split_str(hsmc->memory,joinSpliti,U8H("def "));
                joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,1,spliti->size),U8H("def "));
                nameFunc=(HalSMArray*)HalSMCompiler_getNameFunction(hsmc,joinSpliti)->value;
                f=HalSMLocalFunction_init(hsmc,*(HalStringFormatChar**)nameFunc->arr[0]->value,*(HalStringFormatChar**)nameFunc->arr[1]->value,module);
                func=HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction);
                PutDictElementToDict(hsmc,hsmc->localFunctions,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,f->name),func));
                isFunc=1;
            } else if (StringStartsWith(hsmc->memory,tabsS,U8H("class "))) {
                spliti=HalSMArray_split_str(hsmc->memory,tabsS,U8H("{"));
                joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,0,spliti->size-1),U8H("{"));
                spliti=HalSMArray_split_str(hsmc->memory,joinSpliti,U8H("class "));
                joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,1,spliti->size),U8H("class "));
                joinSpliti=HalSMCompiler_removeSpaces(hsmc->memory,joinSpliti);
                fc=HalSMVariable_init(hsmc->memory,HalSMClass_init(hsmc->memory,joinSpliti,hsmc->variables),HalSMVariableType_HalSMClass);
                PutDictElementToDict(hsmc,hsmc->classes,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,((HalSMClass*)fc->value)->name),fc));
                cls=fc;
                isClass=1;
            } else if (isGOTO->type==HalSMVariableType_HalSMNull&&StringStartsWith(hsmc->memory,tabsS,U8H("goto "))) {
                joinSpliti=HalSMCompiler_removeSpaces(hsmc->memory,*(HalStringFormatChar**)HalSMArray_split_str(hsmc->memory,tabsS,U8H("goto "))->arr[1]->value);
                err=HalSMVariable_init_str(hsmc->memory,joinSpliti);
                if (DictElementIndexByKey(hsmc,hsmc->GOTOSectors,err)!=-1) {
                    //GOTO Function
                    hsmc->indexl=((HalSMGOTOSector*)DictElementFindByKey(hsmc,hsmc->GOTOSectors,err)->value->value)->index;
                } else {
                    isGOTO=HalSMVariable_init(hsmc->memory,HalSMGOTO_init(hsmc->memory,joinSpliti),HalSMVariableType_HalSMGOTO);
                }
            } else if (isGOTO->type==HalSMVariableType_HalSMNull&&StringStartsWith(hsmc->memory,tabsS,U8H("return "))) {
                spliti=HalSMArray_split_str(hsmc->memory,tabsS,U8H("return "));
                joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,1,spliti->size),U8H("return "));
                HalSMArray* outarr=HalSMCompiler_getArgs(hsmc,joinSpliti,&NULLDict);
                if (outarr->size==0) {
                    HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Return not wrong value")));
                    return &hvoid;
                } else {
                    return outarr->arr[0];
                }
            } else if (isGOTO->type==HalSMVariableType_HalSMNull&&isSetVar->type!=HalSMVariableType_HalSMNull) {
                resRunFunc=(HalSMFunctionArray*)isSetVar->value;
                ui8 isGet=0;
                HalSMVariable* isGetV=HalSMCompiler_isGetU(hsmc,*(HalStringFormatChar**)resRunFunc->args->arr[0]->value,&isGet,1,&NULLDict);
                if (isGet) {
                    HalSMVariable* ret=HalSMCompiler_getArgs(hsmc,*(HalStringFormatChar**)resRunFunc->args->arr[1]->value,&NULLDict)->arr[0];
                    isGetV->value=ret->value;
                    isGetV->type=ret->type;
                }
            } else if (HSFCharCompare(tabsS[string_len(tabsS)-1],ASCIIH('{'))) {
                i64 index=StringIndexOf(hsmc->memory,tabsS,U8H("("));
                if (index>-1) {
                    spliti=HalSMArray_split_str(hsmc->memory,tabsS,U8H("{"));
                    joinSpliti=HalSMArray_join_str(hsmc->memory,HalSMArray_slice(hsmc->memory,spliti,0,spliti->size-1),U8H("{"));
                    nameFunc=(HalSMArray*)HalSMCompiler_getNameFunction(hsmc,joinSpliti)->value;

                    //HalStringFormatChar* nameFunction=HalSMCompiler_removeSpaces(hsmc->memory,SubString(hsmc->memory,tabsS,0,index));
                    
                    HalSMVariable* obuff=nameFunc->arr[0];
                    if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1) {
                        HalSMFunctionC* fu=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value->value;
                        funcColonArgs=HalSMArray_init(hsmc->memory);
                        HalSMArray_add(hsmc->memory,funcColonArgs,nameFunc->arr[1]);
                        funcColon=HalSMVariable_init(hsmc->memory,fu,HalSMVariableType_HalSMFunctionC);
                        
                        f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                        func=HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction);
                        HalSMArray_add(hsmc->memory,funcColonArgs,func);
                        isFunc=1;
                        if (DictElementIndexByKey(hsmc,hsmc->groupFunctions,obuff)!=-1) {
                            if (groupFunc->type!=HalSMVariableType_HalSMNull) {
                                err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                                if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                                groupFunc=&null;
                            }
                            
                            groupFunc=HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){funcColon,HalSMVariable_init(hsmc->memory,funcColonArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray);
                            //groupFunctions=HalSMVariable_init(hsmc->memory,DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value->value,HalSMVariableType_HalSMFunctionC);
                            //groupFunctions=HalSMVariable_init(hsmc->memory,DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value->value,HalSMVariableType_HalSMArray);
                            groupFunctions=DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value;
                            HalSMArray_add(hsmc->memory,funcColonArgs,HalSMVariable_init(hsmc->memory,HalSMArray_init(hsmc->memory),HalSMVariableType_HalSMArray));
                            HalSMArray_add(hsmc->memory,arrayGroupFunc,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){groupFunctions,groupFunc,HalSMVariable_init_unsigned_int(hsmc->memory,tabsC)},3),HalSMVariableType_HalSMArray));
                        } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                            if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,funcColon)!=-1) {
                                //Sdes' problema
                                HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){funcColon,HalSMVariable_init(hsmc->memory,funcColonArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                            } else {
                                err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                                if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,funcColon->value,funcColonArgs),HalSMVariableType_HalSMRunFuncC));
                                groupFunctions=&null;
                                HalSMArray_remove(hsmc->memory,arrayGroupFunc,0);
                            }
                        }
                    } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,obuff)!=-1) {
                        HalSMLocalFunction* lf=(HalSMLocalFunction*)DictElementFindByKey(hsmc,hsmc->localFunctions,obuff)->value->value;
                        funcColonArgs=HalSMArray_init(hsmc->memory);
                        HalSMArray_add(hsmc->memory,funcColonArgs,nameFunc->arr[1]);
                        funcColon=HalSMVariable_init(hsmc->memory,lf,HalSMVariableType_HalSMLocalFunction);

                        f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                        func=HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction);
                        HalSMArray_add(hsmc->memory,funcColonArgs,func);
                        isFunc=1;
                    }
                } else {
                    HalStringFormatChar* nameFunction=HalSMCompiler_removeSpaces(hsmc->memory,SubString(hsmc->memory,tabsS,0,string_len(tabsS)-1));
                    HalSMVariable* obuff=HalSMVariable_init_str(hsmc->memory,nameFunction);
                    if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1) {
                        HalSMFunctionC* fu=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value->value;
                        funcColonArgs=HalSMArray_init(hsmc->memory);
                        HalSMArray_add(hsmc->memory,funcColonArgs,HalSMVariable_init_str(hsmc->memory,U8H("")));
                        funcColon=HalSMVariable_init(hsmc->memory,fu,HalSMVariableType_HalSMFunctionC);
                        
                        f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                        func=HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction);
                        HalSMArray_add(hsmc->memory,funcColonArgs,func);
                        isFunc=1;
                        if (DictElementIndexByKey(hsmc,hsmc->groupFunctions,obuff)!=-1) {
                            if (groupFunc->type!=HalSMVariableType_HalSMNull) {
                                err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                                if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                                groupFunc=&null;
                            }
                            
                            groupFunc=HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){funcColon,HalSMVariable_init(hsmc->memory,funcColonArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray);
                            //groupFunctions=HalSMVariable_init(hsmc->memory,DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value->value,HalSMVariableType_HalSMFunctionC);
                            //groupFunctions=HalSMVariable_init(hsmc->memory,DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value->value,HalSMVariableType_HalSMArray);
                            groupFunctions=DictElementFindByKey(hsmc,hsmc->groupFunctions,obuff)->value;
                            HalSMArray_add(hsmc->memory,funcColonArgs,HalSMVariable_init(hsmc->memory,HalSMArray_init(hsmc->memory),HalSMVariableType_HalSMArray));
                            HalSMArray_add(hsmc->memory,arrayGroupFunc,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){groupFunctions,groupFunc,HalSMVariable_init_unsigned_int(hsmc->memory,tabsC)},3),HalSMVariableType_HalSMArray));
                        } else if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                            if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,funcColon)!=-1) {
                                //Sdes' problema
                                HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){funcColon,HalSMVariable_init(hsmc->memory,funcColonArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                            } else {
                                err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                                if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                                HalSMArray_add(hsmc->memory,((HalSMLocalFunction*)func->value)->func,HalSMVariable_init(hsmc->memory,HalSMRunFuncC_init(hsmc->memory,funcColon->value,funcColonArgs),HalSMVariableType_HalSMRunFuncC));
                                groupFunctions=&null;
                                HalSMArray_remove(hsmc->memory,arrayGroupFunc,0);
                            }
                        }
                    } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,obuff)!=-1) {
                        HalSMLocalFunction* lf=(HalSMLocalFunction*)DictElementFindByKey(hsmc,hsmc->localFunctions,obuff)->value->value;
                        funcColonArgs=HalSMArray_init(hsmc->memory);
                        HalSMArray_add(hsmc->memory,funcColonArgs,HalSMVariable_init_str(hsmc->memory,U8H("")));
                        funcColon=HalSMVariable_init(hsmc->memory,lf,HalSMVariableType_HalSMLocalFunction);

                        f=HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module);
                        func=HalSMVariable_init(hsmc->memory,f,HalSMVariableType_HalSMLocalFunction);
                        HalSMArray_add(hsmc->memory,funcColonArgs,func);
                        isFunc=1;
                    } else {
                        if (isGOTO->type==HalSMVariableType_HalSMGOTO&&HSFCompare(nameFunction,((HalSMGOTO*)isGOTO->value)->sector)) {
                            HalSMVariable_free(hsmc->memory,isGOTO);
                            isGOTO=&null;
                        }
                        PutDictElementToDict(hsmc,hsmc->GOTOSectors,DictElementInit(hsmc->memory,obuff,HalSMVariable_init(hsmc->memory,HalSMGOTOSector_init(hsmc->memory,nameFunction,hsmc->indexl,NOTHALSMNULLPOINTER),HalSMVariableType_HalSMGOTOSector)));
                    }
                }
            } else if(HSFCompare(tabsS,U8H("}"))) {
                if (isFuncColon) {
                    isFuncColon=0;
                }

                if (funcColon->type!=HalSMVariableType_HalSMNull) {
                    if (funcColon->type==HalSMVariableType_HalSMFunctionC) {
                        if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                            if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,funcColon)!=-1) {
                                HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){funcColon,HalSMVariable_init(hsmc->memory,funcColonArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                            }
                            err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                            if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                            HalSMMemory_free(hsmc->memory,funcColon);
                            HalSMMemory_free(hsmc->memory,groupFunc);
                            groupFunc=&null;
                        } else {
                            err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)funcColon->value,funcColonArgs,&NULLDict);
                            if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                            HalSMMemory_free(hsmc->memory,funcColon);
                        }
                    } else if (funcColon->type==HalSMVariableType_HalSMLocalFunction) {
                        err=HalSMLocalFunction_run((HalSMLocalFunction*)funcColon->value,funcColonArgs,&NULLDict);
                        if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                        HalSMMemory_free(hsmc->memory,funcColon);
                    }
                    funcColon=&null;
                }
            } else if (isGOTO->type==HalSMVariableType_HalSMNull) {
                if (groupFunc->type!=HalSMVariableType_HalSMNull) {
                    if (HSFCharCompare(tabsS[string_len(tabsS)-1],ASCIIH(')'))&&StringIndexOf(hsmc->memory,tabsS,U8H("("))!=-1&&StringIndexOf(hsmc->memory,tabsS,U8H("["))==-1&&StringIndexOf(hsmc->memory,tabsS,U8H("]"))==-1&&StringIndexOf(hsmc->memory,tabsS,U8H("."))==-1) {
                        i64 index=StringIndexOf(hsmc->memory,tabsS,U8H("("));
                        HalStringFormatChar* nameFunction=HalSMCompiler_removeSpaces(hsmc->memory,SubString(hsmc->memory,tabsS,0,index));
                        HalSMVariable* obuff=HalSMVariable_init_str(hsmc->memory,nameFunction);
                        if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1&&groupFunctions->type!=HalSMVariableType_HalSMNull) {
                            HalSMFunctionC* fu=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value->value;
                            funcColonArgs=HalSMArray_init(hsmc->memory);
                            HalSMArray_add(hsmc->memory,funcColonArgs,HalSMVariable_init_str(hsmc->memory,SubString(hsmc->memory,tabsS,index+1,string_len(tabsS)-1)));
                            funcColon=HalSMVariable_init(hsmc->memory,fu,HalSMVariableType_HalSMFunctionC);
                            HalSMArray_add(hsmc->memory,funcColonArgs,HalSMVariable_init(hsmc->memory,HalSMLocalFunction_init(hsmc,U8H(""),U8H(""),module),HalSMVariableType_HalSMLocalFunction));

                            if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,funcColon)!=-1) {
                                HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){funcColon,HalSMVariable_init(hsmc->memory,funcColonArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                                funcColon=&null;

                                err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                                if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                                groupFunc=&null;
                                groupFunctions=&null;
                                goto NEXT_HALSM_COMPILE_LOOP;
                            }

                            funcColon=&null;

                            err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                            if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                            groupFunc=&null;
                            groupFunctions=&null;
                        }

                        err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                        if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                        groupFunc=&null;
                    } else {
                        err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                        if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                        groupFunc=&null;
                    }
                }
                if (isConsole==1) {
                    HalSMArray* argumentsi=HalSMCompiler_getArgs(hsmc,tabsS,&NULLDict);
                    if (argumentsi->size==0) {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Unknown Line")));
                        return &hvoid;
                    } else if (argumentsi->arr[0]->type!=HalSMVariableType_void) {
                        hsmc->print(HalSMVariable_to_str(hsmc,argumentsi->arr[0],1));
                    }
                } else {
                    ui8 isOut=0;
                    HalSMVariable* err=HalSMCompiler_isGetU(hsmc,l,&isOut,0,&NULLDict);
                    if (err->type==HalSMVariableType_HalSMError) {
                        HalSMCompiler_ThrowError(hsmc,(HalSMError*)err->value);
                        return &hvoid;
                    } else if (isOut==0) {
                        HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Unknown Line")));
                        return &hvoid;
                    }
                }
            }
        }
        

        NEXT_HALSM_COMPILE_LOOP:
        hsmc->line++;
        hsmc->ind->line=hsmc->line;
        if (hsmc->indexl==lastIndex) {
            if (isFunc&&tabsC>0) {
                isFunc=0;
                if (funcColon->type!=HalSMVariableType_HalSMNull) {
                    err=&null;
                    if (funcColon->type==HalSMVariableType_HalSMFunctionC) {
                        err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)funcColon->value,funcColonArgs,&NULLDict);
                        if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                        HalSMMemory_free(hsmc->memory,funcColon);
                    } else if (funcColon->type==HalSMVariableType_HalSMLocalFunction) {
                        err=HalSMLocalFunction_run((HalSMLocalFunction*)funcColon->value,funcColonArgs,&NULLDict);
                        if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                        HalSMMemory_free(hsmc->memory,funcColon);
                    }
                    if(err->type==HalSMVariableType_HalSMGOTOSector) {
                        tabsC=0;
                        prevTabsC=0;
                        hsmc->indexl=((HalSMGOTOSector*)err->value)->index;
                        HalSMArray_clear(hsmc->memory,arrayGroupFunc);
                        HalSMArray_clear(hsmc->memory,funcColonArgs);
                        groupFunc=&null;
                        groupFunctions=&null;
                    }
                    funcColon=&null;
                }
                func=&null;

                if (isFuncColon) {
                    isFuncColon=0;
                }
            } else if (funcColon->type!=HalSMVariableType_HalSMNull) {
                if (funcColon->type==HalSMVariableType_HalSMFunctionC) {
                    if (groupFunctions->type!=HalSMVariableType_HalSMNull) {
                        if (HalSMArray_find(hsmc,(HalSMArray*)groupFunctions->value,funcColon)!=-1) {
                            HalSMArray_add(hsmc->memory,(HalSMArray*)((HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value)->arr[2]->value,HalSMVariable_init(hsmc->memory,HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){funcColon,HalSMVariable_init(hsmc->memory,funcColonArgs,HalSMVariableType_HalSMArray)},2),HalSMVariableType_HalSMArray));
                        }
                        err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)((HalSMArray*)groupFunc->value)->arr[0]->value,(HalSMArray*)((HalSMArray*)groupFunc->value)->arr[1]->value,&NULLDict);
                        if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                    } else {
                        err=HalSMFunctionC_run(hsmc,(HalSMFunctionC*)funcColon->value,funcColonArgs,&NULLDict);
                        if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                    }
                } else if (funcColon->type==HalSMVariableType_HalSMLocalFunction) {
                    err=HalSMLocalFunction_run((HalSMLocalFunction*)funcColon->value,funcColonArgs,&NULLDict);
                    if (err->type==HalSMVariableType_HalSMError){return &hvoid;}
                }
                HalSMMemory_free(hsmc->memory,funcColon);
                funcColon=&null;
            }
        }
        if (!isNull) {
            prevTabsC=tabsC;
        }
    }
    return &hvoid;
}

HalSMVariable* HalSMCompiler_compile(HalSMCompiler* hsmc,HalStringFormatChar* text,ui8 isConsole,HalSMVariable* module)
{
    hsmc->line=1;
    HalSMArray* allLines=HalSMCompiler_getLines(hsmc,text);
    hsmc->indexl=0;
    hsmc->countLocalFunctions=0;
    hsmc->isRun=1;
    hsmc->isClose=0;
    hsmc->allLines=allLines;
    HalSMVariable* out=HalSMCompiler_compileList(hsmc,isConsole,module,allLines);
    hsmc->isRun=0;
    return out;
}


//HalSMOP
HalSMOP* HalSMOP_init(HalSMCompiler* hsmc,HalSMOPType type)
{
    HalSMOP* out=(HalSMOP*)HalSMMemory_malloc(hsmc->memory,sizeof(HalSMOP));
    out->type=type;
    out->args=HalSMArray_init(hsmc->memory);
    out->context=HalSMArray_init(hsmc->memory);
    return out;
}

HalSMFindValue* HalSMFindValue_init(HalSMCompiler* hsmc, HalStringFormatStr name) {
    HalSMFindValue* out=(HalSMFindValue*)HalSMMemory_malloc(hsmc->memory,sizeof(HalSMFindValue));
    out->name=SubString(hsmc->memory,name,0,string_len(name));
    return out;
}

//find value in environment
HalSMVariable* HalSMOP_findValue(HalSMCompiler* hsmc, HalSMVariable* context, HalSMFindValue* fv, Dict* vars)
{
    HalSMVariable *obuff, *vartemp;
    HalSMVariable* out=context;
    HalStringFormatStr fstr;
    ui8 isOut=0;

    fstr=fv->name;
    obuff=HalSMVariable_init_str(hsmc->memory,fstr);


    if (out->type==HalSMVariableType_HalSMModule) {
        if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->variables,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->variables,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->localFunctions,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->localFunctions,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->classes,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->classes,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->functions,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->functions,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMModule*)out->value)->hsmc->modules,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMModule*)out->value)->hsmc->modules,obuff)->value;
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class or Module Not Found")));
            return &hvoid;
        }
    } else if (out->type==HalSMVariableType_HalSMCModule) {
        if (DictElementIndexByKey(hsmc,((HalSMCModule*)out->value)->lfuncs,obuff)!=-1) {
            
            out=DictElementFindByKey(hsmc,((HalSMCModule*)out->value)->lfuncs,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMCModule*)out->value)->vrs,obuff)!=-1) {
            
            out=DictElementFindByKey(hsmc,((HalSMCModule*)out->value)->vrs,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMCModule*)out->value)->classes,obuff)!=-1) {
            
            out=DictElementFindByKey(hsmc,((HalSMCModule*)out->value)->classes,obuff)->value;
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function or Class or Module Not Found")));
            return &hvoid;
        }
    } else if (out->type==HalSMVariableType_HalSMRunClass) {
        if (DictElementIndexByKey(hsmc,((HalSMRunClass*)out->value)->funcs,obuff)!=-1) {
            
            out=DictElementFindByKey(hsmc,((HalSMRunClass*)out->value)->funcs,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMRunClass*)out->value)->vars,obuff)!=-1) {
            
            out=DictElementFindByKey(hsmc,((HalSMRunClass*)out->value)->vars,obuff)->value;
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function Not Found")));
            return &hvoid;
        }
    } else if (out->type==HalSMVariableType_HalSMClass) {
        if (DictElementIndexByKey(hsmc,((HalSMClass*)out->value)->funcs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMClass*)out->value)->funcs,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMClass*)out->value)->vars,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMClass*)out->value)->vars,obuff)->value;
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function Not Found")));
            return &hvoid;
        }
    } else if (out->type==HalSMVariableType_HalSMRunClassC) {
        if (DictElementIndexByKey(hsmc,((HalSMRunClassC*)out->value)->funcs,obuff)!=-1) {
            
            out=DictElementFindByKey(hsmc,((HalSMRunClassC*)out->value)->funcs,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMRunClassC*)out->value)->vrs,obuff)!=-1&&DictElementFindByKey(hsmc,((HalSMRunClassC*)out->value)->vrs,obuff)->value->type==HalSMVariableType_HalSMFunctionC) {
            
            out=DictElementFindByKey(hsmc,((HalSMRunClassC*)out->value)->vrs,obuff)->value;
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function Not Found")));
            return &hvoid;
        }
    } else if (out->type==HalSMVariableType_HalSMClassC) {
        if (DictElementIndexByKey(hsmc,((HalSMClassC*)out->value)->funcs,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,((HalSMClassC*)out->value)->funcs,obuff)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,((HalSMClassC*)out->value)->vrs,obuff)!=-1&&DictElementFindByKey(hsmc,((HalSMClassC*)out->value)->vrs,obuff)->value->type==HalSMVariableType_HalSMFunctionC) {
            out=DictElementFindByKey(hsmc,((HalSMClassC*)out->value)->vrs,obuff)->value;
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Variable or Function Not Found")));
            return &hvoid;
        }
    } else if (out->type==HalSMVariableType_custom) {
        if (((HalSMVariableCustom*)out->value)->type->indexFunctionByName(hsmc,out,*(HalStringFormatChar**)obuff->value)!=-1) {
            
            out=((HalSMVariableCustom*)out->value)->type->getFunctionByName(hsmc,out,*(HalStringFormatChar**)obuff->value);
            isOut=1;
        } else if (((HalSMVariableCustom*)out->value)->type->indexVariableByName(hsmc,out,*(HalStringFormatChar**)obuff->value)!=-1) {
            out=((HalSMVariableCustom*)out->value)->type->getVariableByName(hsmc,out,*(HalStringFormatChar**)obuff->value);
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Function or Variable of Custom Variable not found")));
            return &hvoid;
        }
    } else if (out->type==HalSMVariableType_str) {
        HalStringFormatChar* strFunc=*(HalStringFormatChar**)obuff->value;
        if (HSFCompare(strFunc,U8H("replace"))) {
            
            out=hsmc->functions->elements[13]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("index"))) {
            
            out=hsmc->functions->elements[14]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("rindex"))) {
            
            out=hsmc->functions->elements[15]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("split"))) {
            
            out=hsmc->functions->elements[16]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("substring"))) {
            
            out=hsmc->functions->elements[17]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("len"))) {
            
            out=hsmc->functions->elements[19]->value;
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Function not found")));
            return &hvoid;
        }
    } else if (out->type==HalSMVariableType_HalSMArray) {
        HalStringFormatChar* strFunc=*(HalStringFormatChar**)obuff->value;
        if (HSFCompare(strFunc,U8H("append"))) {
            
            out=hsmc->functions->elements[18]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("len"))) {
            
            out=hsmc->functions->elements[19]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("clear"))) {
            
            out=hsmc->functions->elements[20]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("index"))) {
            
            out=hsmc->functions->elements[14]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("insert"))) {
            
            out=hsmc->functions->elements[21]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("pop"))) {
            out=hsmc->functions->elements[22]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("remove"))) {
            
            out=hsmc->functions->elements[23]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("reverse"))) {
            
            out=hsmc->functions->elements[24]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("copy"))) {
            
            out=hsmc->functions->elements[25]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("appendArray"))) {
            
            out=hsmc->functions->elements[26]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("slice"))) {
            
            out=hsmc->functions->elements[27]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("set"))) {
            
            out=hsmc->functions->elements[31]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("join"))) {
            
            out=hsmc->functions->elements[39]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("rindex"))) {
            
            out=hsmc->functions->elements[15]->value;
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Function not found")));
            return &hvoid;
        }
    } else if (out->type==HalSMVariableType_HalSMDict) {
        HalStringFormatChar* strFunc=*(HalStringFormatChar**)obuff->value;
        if (HSFCompare(strFunc,U8H("append"))) {
            
            out=hsmc->functions->elements[18]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("appendDict"))) {
            
            out=hsmc->functions->elements[28]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("len"))) {
            
            out=hsmc->functions->elements[19]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("clear"))) {
            
            out=hsmc->functions->elements[20]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("indexByKey"))) {
            
            out=hsmc->functions->elements[29]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("indexByValue"))) {
            
            out=hsmc->functions->elements[30]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("set"))) {
            
            out=hsmc->functions->elements[31]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("pop"))) {
            
            out=hsmc->functions->elements[22]->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,(Dict*)out->value,obuff)!=-1) {
            out=DictElementFindByKey(hsmc,(Dict*)out->value,obuff)->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("keyByIndex"))) {
            
            out=hsmc->functions->elements[43]->value;
            isOut=1;
        } else if (HSFCompare(strFunc,U8H("valueByIndex"))) {
            out=hsmc->functions->elements[44]->value;
            isOut=1;
        } else {
            //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Function not found")));
            return &hvoid;
        }
    } else if (DictElementIndexByKey(hsmc,hsmc->functions,obuff)!=-1) {
        out=DictElementFindByKey(hsmc,hsmc->functions,obuff)->value;
        isOut=1;
    } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,obuff)!=-1) {
        out=DictElementFindByKey(hsmc,hsmc->localFunctions,obuff)->value;
        isOut=1;
    } else if (DictElementIndexByKey(hsmc,hsmc->classes,obuff)!=-1) {
        out=DictElementFindByKey(hsmc,hsmc->classes,obuff)->value;
        isOut=1;
    } else if (DictElementIndexByKey(hsmc,hsmc->variables,obuff)!=-1) {
        out=DictElementFindByKey(hsmc,hsmc->variables,obuff)->value;
        isOut=1;
    } else if(DictElementIndexByKey(hsmc,hsmc->modules,obuff)!=-1) {
        out=DictElementFindByKey(hsmc,hsmc->modules,obuff)->value;
        isOut=1;
    } else if ((vartemp=hsmc->getVariable(*(HalStringFormatChar**)obuff->value))->type!=HalSMVariableType_HalSMNull) {
        out=vartemp;
        isOut=1;
        vartemp=&null;
    } else if (DictElementIndexByKey(hsmc,vars,obuff)!=-1) {
        out=DictElementFindByKey(hsmc,vars,obuff)->value;
        isOut=1;
    } else {
        //HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->ind,U8H("Is not function")));
        return &hvoid;
    }

    return isOut?out:&null;
}


//process arguments for executing operation
HalSMVariable* HalSMOP_processVar(HalSMCompiler* hsmc,HalSMVariable* var,Dict* vars)
{
    HalSMFindValue* fv;
    HalSMOP* op;

    if(var->type==HalSMVariableType_fv) {
        fv=(HalSMFindValue*)var->value;
        var=HalSMOP_findValue(hsmc,&null,fv,vars);
    } else if(var->type==HalSMVariableType_HalSMOP) {
        op=(HalSMOP*)var->value;
        var=HalSMOP_execute(hsmc,op,vars);

        if(var->type==HalSMVariableType_HalSMArray) {
            var->value=HalSMOP_processArgs(hsmc,(HalSMArray*)var->value,vars);
        } else if(var->type==HalSMVariableType_HalSMDict) {
            var->value=HalSMOP_processDict(hsmc,(Dict*)var->value,vars);
        }
    }
    return var;
}

HalSMArray* HalSMOP_processArgs(HalSMCompiler* hsmc, HalSMArray* args, Dict* vars)
{
    HalSMArray* out=HalSMArray_init(hsmc->memory);
    HalSMVariable* var;
    ui64 i,l;
    l=args->size;

    for(i=0;i<l;i++) {
        var=args->arr[i];
        var=HalSMOP_processVar(hsmc,var,vars);
        HalSMArray_add(hsmc->memory,out,var);
    }

    return out;
}

Dict* HalSMOP_processDict(HalSMCompiler* hsmc,Dict* args, Dict* vars) {
    Dict* out=DictInit(hsmc->memory);

    HalSMVariable* keyVar,*valVar;
    ui64 i,l;

    l=args->size;

    for(i=0;i<l;i++) {
        keyVar=args->elements[i]->key;
        keyVar=HalSMOP_processVar(hsmc,keyVar,vars);

        valVar=args->elements[i]->key;
        valVar=HalSMOP_processVar(hsmc,valVar,vars);

        PutDictElementToDict(hsmc,out,DictElementInit(hsmc->memory,keyVar,valVar));
    }

    return out;
}

//Process Context
HalSMVariable* HalSMOP_processContext(HalSMCompiler* hsmc, HalSMArray* context, Dict* vars, HalSMVariable** module)
{
    HalSMVariable* out=&hvoid,*tempv;
    ui64 i,j,lastIndex,index;
    ui8 isNotNull=0;
    HalSMFindValue* fv;
    HalSMOP* op,*iop;
    HalSMArray* temparr;
    Dict* tempdict;

    if(context->size==0) {
        return out;
    } else if(context->size==1) {
        return HalSMOP_processVar(hsmc,context->arr[0],vars);
    }

    lastIndex=context->size-1;

    *module=HalSMOP_processVar(hsmc,context->arr[0],vars);

    for (i=1;i<context->size;i++) {
        out=context->arr[i];

        if(out->type==HalSMVariableType_fv) {
            fv=(HalSMFindValue*)out->value;

            out=HalSMOP_findValue(hsmc,*module,fv,vars);
        } else if(out->type==HalSMVariableType_HalSMOP) {
            op=(HalSMOP*)out->value;

            if(op->type==HalSMOPType_nothing) {
                out=*module;
                continue;
            }

            if(op->context->size==0) {
                //error
            }

            out=op->context->arr[0];

            iop=HalSMOP_init(hsmc,op->type);
            iop->args=HalSMArray_init(hsmc->memory);
            iop->context=op->context;

            if(out->type==HalSMVariableType_fv) {
                fv=(HalSMFindValue*)out->value;
                temparr=HalSMArray_init(hsmc->memory);

                HalSMArray_add(hsmc->memory,temparr,HalSMOP_findValue(hsmc,*module,fv,vars));
                HalSMArray_add(hsmc->memory,temparr,iop->context->arr[0]);
                iop->context=temparr;
            }

            HalSMArray_add(hsmc->memory,iop->args,*module);
            for(j=0;j<op->args->size;j++) {
                HalSMArray_add(hsmc->memory,iop->args,op->args->arr[j]);
            }

            out=HalSMOP_execute(hsmc,iop,vars);
        } else if(out->type==HalSMVariableType_HalSMArray) {
            //Get Elements
            temparr=(HalSMArray*)out->value;
            if(temparr->size==0) {
                //error
                continue;
            }
            temparr=HalSMOP_processArgs(hsmc,temparr,vars);
            tempv=temparr->arr[0];
            if((*module)->type==HalSMVariableType_HalSMArray) {
                if(tempv->type!=HalSMVariableType_int) {
                    //error
                    continue;
                }

                index=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)tempv->value);
                temparr=(HalSMArray*)(*module)->value;

                if(index>temparr->size-1) {
                    //error
                    continue;
                }
                out=HalSMOP_processVar(hsmc,temparr->arr[index],vars);
            } else if((*module)->type==HalSMVariableType_HalSMDict) {
                tempdict=(Dict*)(*module)->value;
                if(DictElementIndexByKey(hsmc,tempdict,tempv)!=-1) {
                    out=DictElementFindByKey(hsmc,tempdict,tempv)->value;
                }
            }
        }

        if(i<lastIndex) {
            *module=out;
        }
    }

    return out;
}

//get arguments for operation
HalSMArray* HalSMOP_getArgs(HalSMCompiler* hsmc, HalStringFormatStr data, Dict* vars)
{
    HalSMArray* args,*os;
    HalStringFormatStr obuff;
    HalStringFormatChar c;
    ui64 i,l;
    HalSMOP* op;

    HalStringFormatChar isS=ASCIIH('n');
    ui64 isA,isD,isF;

    args=HalSMArray_init(hsmc->memory);
    os=HalSMArray_init(hsmc->memory);

    isA=0;
    isD=0;
    isF=0;

    l=string_len(data);

    for(i=0;i<l;i++) {
        c=data[i];
        if(HSFCharCompare(isS,ASCIIH('n'))==0) {
            if(HSFCharCompare(isS,c)) {
                isS=ASCIIH('n');
            }
        } else if(isA>0) {
            if(HSFCharCompare(c,ASCIIH('['))) {
                isA++;
            } else if(HSFCharCompare(c,ASCIIH(']'))) {
                isA--;
            }
        } else if(isD>0) {
            if(HSFCharCompare(c,ASCIIH('{'))) {
                isD++;
            } else if(HSFCharCompare(c,ASCIIH('}'))) {
                isD--;
            }
        } else if(isF>0) {
            if(HSFCharCompare(c,ASCIIH('('))) {
                isF++;
            } else if(HSFCharCompare(c,ASCIIH(')'))) {
                isF--;
            }
        } else if(HSFCharCompare(c,ASCIIH('"'))||HSFCharCompare(c,ASCIIH('\''))) {
            isS=c;
        } else if(HSFCharCompare(c,ASCIIH('['))) {
            isA++;
        } else if(HSFCharCompare(c,ASCIIH(']'))) {
            //error
        } else if(HSFCharCompare(c,ASCIIH('{'))) {
            isD++;
        } else if(HSFCharCompare(c,ASCIIH('}'))) {
            //error
        } else if(HSFCharCompare(c,ASCIIH('('))) {
            isF++;
        } else if(HSFCharCompare(c,ASCIIH(')'))) {
            //error
        } else if(HSFCharCompare(c,ASCIIH(','))) {
            //Get Arg
            obuff=HalSMArray_chars_to_str(hsmc->memory,os);
            op=HalSMOP_compile(hsmc,obuff,vars);

            HalSMArray_add(hsmc->memory,args,HalSMVariable_init(hsmc->memory,op,HalSMVariableType_HalSMOP));

            HalSMArray_clear(hsmc->memory,os);

            continue;
        }

        HalSMArray_add(hsmc->memory,os,HalSMVariable_init_char(hsmc->memory,c));
    }

    if(os->size>0) {
        obuff=HalSMArray_chars_to_str(hsmc->memory,os);

        op=HalSMOP_compile(hsmc,obuff,vars);

        HalSMArray_add(hsmc->memory,args,HalSMVariable_init(hsmc->memory,op,HalSMVariableType_HalSMOP));

        HalSMArray_clear(hsmc->memory,os);
    }

    return args;
}

//[1,2,3].pop(1).seven(6)
//Мне самое главное определить инструкции и аргументы с чем совершается
//Compile line of code into HalSM Operation
HalSMOP* HalSMOP_compile(HalSMCompiler* hsmc,HalStringFormatStr line,Dict* vars)
{
    //Detect is import
    HalStringFormatStr obuff,zbuff;
    HalStringFormatChar c;
    ui64 i,l;
    HalSMOP* out,*argout,*tempop;
    HalSMArray* args,*os,*value,*contextArgs;

    ui8 isNSArg; //is need second arg
    ui8 isMInc; // is maybe increment

    HalStringFormatChar isS=ASCIIH('n');
    ui64 isA,isD,isF;

    isA=0;
    isD=0;
    isF=0;

    isNSArg=0;
    isMInc=0;

    out=NOTHALSMNULLPOINTER;
    args=HalSMArray_init(hsmc->memory);
    l=string_len(line);
    os=HalSMArray_init(hsmc->memory);
    argout=NOTHALSMNULLPOINTER;

    for(i=0;i<l;i++) {
        c=line[i];
        obuff=HalSMArray_chars_to_str(hsmc->memory, os);

        if(HSFCharCompare(isS,ASCIIH('n'))==0) {
            if(HSFCharCompare(isS,c)) {
                isS=ASCIIH('n');

                //is string

                zbuff=SubString(hsmc->memory,obuff,1,string_len(obuff));

                if(argout==NOTHALSMNULLPOINTER) {
                    argout=HalSMOP_init(hsmc,HalSMOPType_value);
                    contextArgs=argout->context;
                }

                HalSMArray_add(hsmc->memory,contextArgs,HalSMVariable_init_str(hsmc->memory,zbuff));

                HalSMArray_clear(hsmc->memory,os);

                continue;
            }
        } else if(isA>0) {
            if(HSFCharCompare(c,ASCIIH('['))) {
                isA++;
            } else if(HSFCharCompare(c,ASCIIH(']'))) {
                isA--;

                if(isA==0) {
                    zbuff=SubString(hsmc->memory,obuff,1,string_len(obuff));

                    if(argout==NOTHALSMNULLPOINTER) {
                        argout=HalSMOP_init(hsmc,HalSMOPType_value);
                        contextArgs=argout->context;
                    }

                    HalSMArray_add(hsmc->memory,contextArgs,HalSMVariable_init(hsmc->memory,HalSMOP_getArgs(hsmc,zbuff,vars),HalSMVariableType_HalSMArray));

                    HalSMArray_clear(hsmc->memory,os);

                    continue;
                }
            }
        } else if(isD>0) {
            if(HSFCharCompare(c,ASCIIH('{'))) {
                isD++;
            } else if(HSFCharCompare(c,ASCIIH('}'))) {
                isD--;
            }
        } else if(isF>0) {
            if(HSFCharCompare(c,ASCIIH('('))) {
                isF++;
            } else if(HSFCharCompare(c,ASCIIH(')'))) {
                isF--;
                if(isF==0&&argout!=NOTHALSMNULLPOINTER) {
                    if(contextArgs->size>0&&contextArgs->arr[contextArgs->size-1]->type==HalSMVariableType_HalSMOP) {
                        tempop=(HalSMOP*)contextArgs->arr[contextArgs->size-1]->value;

                        if(tempop->type==HalSMOPType_runFunction) {
                            tempop->args=HalSMOP_getArgs(hsmc,obuff,vars);
                        } else {
                            argout->args=HalSMOP_getArgs(hsmc,obuff,vars);
                        }
                    } else {
                        argout->args=HalSMOP_getArgs(hsmc,obuff,vars);
                    }

                    HalSMArray_clear(hsmc->memory,os);

                    continue;
                } else if(isF==0) {
                    argout=HalSMOP_compile(hsmc,obuff,vars);

                    contextArgs=argout->context;

                    HalSMArray_clear(hsmc->memory,os);

                    continue;
                }
            }
        } else if(HSFCharCompare(c,ASCIIH('"'))||HSFCharCompare(c,ASCIIH('\''))) {
            isS=c;
        } else if(HSFCharCompare(c,ASCIIH('['))) {
            //is get
            isA++;

            if(string_len(obuff)>0) {
                if(argout==NOTHALSMNULLPOINTER) {
                    argout=HalSMOP_init(hsmc,HalSMOPType_value);
                    contextArgs=argout->context;
                }

                HalSMArray_add(hsmc->memory,contextArgs,HalSMVariable_init(hsmc->memory,HalSMFindValue_init(hsmc,obuff),HalSMVariableType_fv));

                HalSMArray_clear(hsmc->memory,os);
            }
        } else if(HSFCharCompare(c,ASCIIH(']'))) {
            //error
        } else if(HSFCharCompare(c,ASCIIH('{'))) {
            isD++;
        } else if(HSFCharCompare(c,ASCIIH('}'))) {
            //error
        } else if(HSFCharCompare(c,ASCIIH('('))) {
            isF++;
            if(string_len(obuff)>0) {
                if(argout==NOTHALSMNULLPOINTER) {
                    argout=HalSMOP_init(hsmc,HalSMOPType_runFunction);

                    contextArgs=argout->context;

                    HalSMArray_add(hsmc->memory,contextArgs,HalSMVariable_init(hsmc->memory,HalSMFindValue_init(hsmc,obuff),HalSMVariableType_fv));
                } else {
                    tempop=HalSMOP_init(hsmc,HalSMOPType_runFunction);
                    HalSMArray_add(hsmc->memory,tempop->context,HalSMVariable_init(hsmc->memory,HalSMFindValue_init(hsmc,obuff),HalSMVariableType_fv));

                    HalSMArray_add(hsmc->memory,contextArgs,HalSMVariable_init(hsmc->memory,tempop,HalSMVariableType_HalSMOP));
                }


                HalSMArray_clear(hsmc->memory,os);
            }

            continue;
        } else if(HSFCharCompare(c,ASCIIH(')'))) {
            //error
        } else if(HSFCharCompare(c,ASCIIH('='))) {
            if(os->size>0) {
                if(argout==NOTHALSMNULLPOINTER) {
                    argout=HalSMOP_init(hsmc,HalSMOPType_setVar);
                    contextArgs=argout->args;
                }

                HalSMArray_add(hsmc->memory,argout->context,HalSMVariable_init(hsmc->memory,HalSMFindValue_init(hsmc,obuff),HalSMVariableType_fv));

                HalSMArray_clear(hsmc->memory,os);
            } else {
                argout->type=HalSMOPType_setVar;
                contextArgs=argout->args;

                HalSMArray_clear(hsmc->memory,os);
            }
            continue;
        } else if(HSFCharCompare(c,ASCIIH('.'))) {
            //next context
            if(os->size>0) {
                if (HalSMIsInt(hsmc->memory,obuff)) {
                    HalSMArray_add(hsmc->memory,os,HalSMVariable_init_char(hsmc->memory,c));
                    continue;
                }

                if(argout==NOTHALSMNULLPOINTER) {
                    argout=HalSMOP_init(hsmc,HalSMOPType_value);
                    contextArgs=argout->args;
                }

                HalSMArray_add(hsmc->memory,contextArgs,HalSMVariable_init(hsmc->memory,HalSMFindValue_init(hsmc,obuff),HalSMVariableType_fv));

                HalSMArray_clear(hsmc->memory,os);
            } else {
                continue;
            }
        } else if(HSFCharCompare(c,ASCIIH('+'))) {
            if(isMInc) {
                isNSArg=1;
                isMInc=0;
                if(argout==NOTHALSMNULLPOINTER) {
                    argout=HalSMOP_init(hsmc,HalSMOPType_inc);
                    contextArgs=argout->context;
                }
            } else if(os->size>0||argout!=NOTHALSMNULLPOINTER) {
                
            } else {
                isMInc=1;
            }
        }

        HalSMArray_add(hsmc->memory,os,HalSMVariable_init_char(hsmc->memory,c));
        HalSMMemory_free(hsmc->memory,obuff);
    }

    if(os->size>0) {
        ui8 isOut=0;
        HalSMVariable* valueOut=&null;
        HalSMVariable* buffo;
        HalSMVariable* vartemp;
        obuff=HalSMArray_chars_to_str(hsmc->memory,os);
        buffo=HalSMVariable_init_str(hsmc->memory,obuff);

        if (HSFCompare(obuff,U8H("true"))) {
            valueOut=HalSMVariable_init_bool(hsmc->memory,1);
            isOut=1;
        } else if (HSFCompare(obuff,U8H("false"))) {
            valueOut=HalSMVariable_init_bool(hsmc->memory,0);
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,hsmc->variables,buffo)!=-1) {
            valueOut=DictElementFindByKey(hsmc,hsmc->variables,buffo)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,vars,buffo)!=-1) {
            valueOut=DictElementFindByKey(hsmc,vars,buffo)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,hsmc->localFunctions,buffo)!=-1) {
            valueOut=DictElementFindByKey(hsmc,hsmc->localFunctions,buffo)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,hsmc->modules,buffo)!=-1) {
            valueOut=DictElementFindByKey(hsmc,hsmc->modules,buffo)->value;
            isOut=1;
        } else if (DictElementIndexByKey(hsmc,hsmc->classes,buffo)!=-1) {
            valueOut=DictElementFindByKey(hsmc,hsmc->classes,buffo)->value;
            isOut=1;
        } else if ((valueOut=hsmc->getVariable(*(HalStringFormatChar**)buffo->value))->type!=HalSMVariableType_HalSMNull) {
            isOut=1;
        } else if (HSFCompare(obuff,U8H("NULL"))||HSFCompare(obuff,U8H("null"))) {
            valueOut=&null;
            isOut=1;
        } else if (HalSMIsInt(hsmc->memory,obuff)) {
            valueOut=HalSMVariable_init(hsmc->memory,HalSMInteger_parse(hsmc->memory,obuff),HalSMVariableType_int);
            isOut=1;
        } else if (HalSMIsDouble(hsmc->memory,obuff)) {
            valueOut=HalSMVariable_init(hsmc->memory,HalSMDouble_parse(hsmc->memory,obuff),HalSMVariableType_double);
            isOut=1;
        } else if (HalSMIsHex(hsmc->memory,obuff)) {
            valueOut=HalSMVariable_init(hsmc->memory,HalSMInteger_parseHex(hsmc->memory,obuff,1),HalSMVariableType_int);
            isOut=1;
        } else if (HalSMIsBin(hsmc->memory,obuff)) {
            valueOut=HalSMVariable_init(hsmc->memory,HalSMInteger_parseBin(hsmc->memory,obuff),HalSMVariableType_int);
            isOut=1;
        } else if (StringStartsWith(hsmc->memory,obuff,U8H("["))&&StringEndsWith(hsmc->memory,obuff,U8H("]"))) {
            valueOut=HalSMVariable_init(hsmc->memory,HalSMOP_getArgs(hsmc,SubString(hsmc->memory,obuff,1,string_len(obuff)-1),vars),HalSMVariableType_HalSMArray);
            isOut=1;
        //parseDict
        } else {
            //error
        }

        if(argout==NOTHALSMNULLPOINTER) {
            argout=HalSMOP_init(hsmc,HalSMOPType_value);
            contextArgs=argout->context;
        }

        HalSMArray_add(hsmc->memory,contextArgs,valueOut);
    }

    if(argout!=NOTHALSMNULLPOINTER) {
        out=argout;
    }

    return out;
}

//Execute HalSM Operation
HalSMVariable* HalSMOP_execute(HalSMCompiler* hsmc, HalSMOP* op, Dict* vars)
{
    HalSMVariable* v,*module;
    HalSMArray* args,*temparr;
    ui64 tempi;

    module=HalSMVariable_init(hsmc->memory,NOTHALSMNULLPOINTER,HalSMVariableType_HalSMNull);

    if(op==NOTHALSMNULLPOINTER) {
        return &null;
    }

    v=HalSMOP_processContext(hsmc,op->context,vars,&module);

    if(op->type==HalSMOPType_value) {
        return v;
    } else if(op->type==HalSMOPType_runFunction) {
        args=HalSMOP_processArgs(hsmc,op->args,vars);

        if(v->type==HalSMVariableType_HalSMLocalFunction) {
            HalSMLocalFunction* lf=(HalSMLocalFunction*)v->value;
            return HalSMLocalFunction_run(lf,args,vars);
        } else if(v->type==HalSMVariableType_HalSMFunctionC) {
            HalSMFunctionC* fc=(HalSMFunctionC*)v->value;
            return HalSMFunctionC_run(hsmc,fc,args,vars);
        } else if(v->type==HalSMVariableType_HalSMClassC) {
            HalSMClassC* cc=(HalSMClassC*)v->value;
            return HalSMVariable_init(hsmc->memory,HalSMClassC_run(hsmc,cc,args),HalSMVariableType_HalSMRunClassC);
        } else if(v->type==HalSMVariableType_HalSMClass) {
            HalSMClass* cl=(HalSMClass*)v->value;
            return HalSMVariable_init(hsmc->memory,HalSMClass_run(cl,hsmc,args),HalSMVariableType_HalSMRunClass);
        }
    } else if(op->type==HalSMOPType_setVar) {
        args=HalSMOP_processArgs(hsmc,op->args,vars);
        if(args->size==0) {
            //error
        }

        if(module->type==HalSMVariableType_HalSMNull) {
            if(op->context->size==0||op->context->arr[op->context->size-1]->type!=HalSMVariableType_fv) {
                //error
            }


            HalSMFindValue* fv=(HalSMFindValue*)op->context->arr[op->context->size-1]->value;

            PutDictElementToDict(hsmc,hsmc->variables,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,fv->name),args->arr[0]));
        } else if(module->type==HalSMVariableType_HalSMArray) {
            if(op->context->arr[op->context->size-1]->type==HalSMVariableType_HalSMArray) {
                temparr=HalSMOP_processArgs(hsmc,(HalSMArray*)op->context->arr[op->context->size-1]->value,vars);

                if(temparr->size==0||temparr->arr[0]->type!=HalSMVariableType_int) {
                    //error
                } else {
                    tempi=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)temparr->arr[0]->value);
                    HalSMArray_set((HalSMArray*)module->value,args->arr[0],tempi);
                }
            } else {
                //error
            }
        }

        //ELSE MODULE OR CLASS AND ETC...
    }

    return &null;
}

//Free all environment
void HalSMCompiler_free(HalSMCompiler* hsmc)
{
    /*HalSMMemory_free(hsmc->memory,hsmc->versionName);
    HalSMMemory_free(hsmc->memory,hsmc->path);
    DictFreeE(hsmc->memory,hsmc->functions);
    DictFreeE(hsmc->memory,hsmc->sys_modules);
    HalSMMemory_free(hsmc->memory,hsmc->calcVars.version);
    DictFreeE(hsmc->memory,hsmc->variables);
    //DictFreeE(hsmc->memory,hsmc->sys_variables);
    DictFreeE(hsmc->memory,hsmc->modules);
    DictFreeE(hsmc->memory,hsmc->localFunctions);
    DictFreeE(hsmc->memory,hsmc->classes);
    DictFreeE(hsmc->memory,hsmc->groupFunctions);

    DictFreeE(hsmc->memory,hsmc->sys_custom_variables);
    DictFreeE(hsmc->memory,hsmc->GOTOSectors);

    HalSMMemory_free(hsmc->memory,hsmc->ind);
    HalSMMemory_free(hsmc->memory,hsmc->curInd);
    HalSMMemory_free(hsmc->memory,hsmc);*/

    HalSMMemory_freeSys(hsmc->memory);
}

void HalSMCompiler_exitApp(HalSMCompiler* hsmc)
{
    hsmc->isRun=0;
    hsmc->isClose=1;
    HalSMCompiler_free(hsmc);
}

HalSMDoubleGet* HalSMDoubleGet_init(HalSMMemory* memory,HalStringFormatChar* st)
{
    HalSMDoubleGet* hfg=HalSMMemory_malloc(memory,sizeof(HalSMDoubleGet));
    hfg->st=st;
    return hfg;
}

HalSMVar* HalSMVar_init(HalSMMemory* memory,HalStringFormatChar* name)
{
    HalSMVar* out=HalSMMemory_malloc(memory,sizeof(HalSMVar));
    out->name=HalSMVariable_init_str(memory,name);
    return out;
}

HalSMSetArg* HalSMSetArg_init(HalSMMemory* memory,HalSMVariable* name)
{
    HalSMSetArg* out=HalSMMemory_malloc(memory,sizeof(HalSMSetArg));
    out->name=name;
    out->value=HalSMVariable_init(memory,&null,HalSMVariableType_HalSMNull);
    return out;
}

HalSMSetArg* HalSMSetArg_initWithValue(HalSMMemory* memory,HalSMVariable* name,HalSMVariable* value)
{
    HalSMSetArg* out=HalSMMemory_malloc(memory,sizeof(HalSMSetArg));
    out->name=name;
    out->value=value;
    return out;
}

HalSMReturn* HalSMReturn_init(HalSMMemory* memory,HalStringFormatChar* val,ui64 idLocalFunction)
{
    HalSMReturn* out=HalSMMemory_malloc(memory,sizeof(HalSMReturn));
    out->value=val;
    out->idLocalFunction=idLocalFunction;
    return out;
}


//DICT


Dict* DictInit(HalSMMemory* memory)
{
    Dict* dict=HalSMMemory_malloc(memory,sizeof(Dict));
    dict->size=0;
    dict->elements=HalSMMemory_malloc(memory,0);
    return dict;
}

Dict* DictInitWithElements(HalSMMemory* memory,DictElement* elements[],ui64 size)
{
    Dict* dict=HalSMMemory_malloc(memory,sizeof(Dict));
    dict->size=size;
    dict->elements=HalSMMemory_malloc(memory,sizeof(DictElement*)*size);
    for (ui64 i=0;i<size;i++) {
        dict->elements[i]=elements[i];
    }
    return dict;
}

DictElement* DictElementInit(HalSMMemory* memory,HalSMVariable* key,HalSMVariable* value)
{
    DictElement* out=HalSMMemory_malloc(memory,sizeof(DictElement));
    out->key=key;
    out->value=value;
    return out;
}

DictElement* DictElementFindByIndex(Dict* dict,ui64 index)
{
    DictElement* temp=NOTHALSMNULLPOINTER;
    if (index<dict->size)temp=dict->elements[index];
    return temp;
}

DictElement* DictElementFindByKey(HalSMCompiler* hsmc,Dict* dict,HalSMVariable* key)
{
    DictElement* empty=NOTHALSMNULLPOINTER;
    if (dict->size==0){return empty;}
    for (ui64 i=0;i<dict->size;i++)
    {
        if (HalSMVariable_Compare(hsmc,dict->elements[i]->key,key)) return dict->elements[i];
    }
    return empty;
}

DictElement* DictElementFindByValue(HalSMCompiler* hsmc,Dict* dict,HalSMVariable* value)
{
    DictElement* empty=NOTHALSMNULLPOINTER;
    if (dict->size==0) {return empty;}
    for (ui64 i=0;i<dict->size;i++)
    {
        if (HalSMVariable_Compare(hsmc,dict->elements[i]->value,value)) return dict->elements[i];
    }
    return empty;
}

DictElement* DictElementCopy(HalSMMemory* memory,DictElement* elem)
{
    return DictElementInit(memory,HalSMVariable_init(memory,elem->key->value,elem->key->type),HalSMVariable_init(memory,elem->value->value,elem->value->type));
}

void DictElementFree(HalSMMemory* memory,DictElement* elem)
{
    HalSMVariable_free(memory,elem->key);
    HalSMVariable_free(memory,elem->value);
    HalSMMemory_free(memory,elem);
}

void PutDictElementToDict(HalSMCompiler* hsmc,Dict *dict,DictElement* elem)
{
    i64 index=DictElementIndexByKey(hsmc,dict,elem->key);
    if (index==-1) {
        dict->elements=HalSMMemory_realloc(hsmc->memory,dict->elements,(dict->size+1)*sizeof(DictElement*));
        dict->elements[dict->size]=elem;
        dict->size=dict->size+1;
    } else {
        dict->elements[index]->value->value=elem->value->value;
        dict->elements[index]->value->type=elem->value->type;
        //HalSMMemory_free(memory,elem);
    }
}

i64 DictElementIndexByKey(HalSMCompiler* hsmc,Dict* dict,HalSMVariable* key)
{
    if (dict->size==0)return -1;
    DictElement** p=&dict->elements[0];
    for (ui64 i=0;i<dict->size;i++)
    {
        if (HalSMVariable_Compare(hsmc,(*p++)->key,key)) {return i;}
    }
    return -1;
}

i64 DictElementIndexByValue(HalSMCompiler* hsmc,Dict* dict,HalSMVariable* value)
{
    if (dict->size==0)return -1;
    DictElement** p=&dict->elements[0];
    for (ui64 i=0;i<dict->size;i++)
    {
        if (HalSMVariable_Compare(hsmc,(*p++)->value,value)) {return i;}
    }
    return -1;
}

Dict* DictCopy(HalSMMemory* memory,Dict* dict)
{
    return DictInitWithElements(memory,dict->elements,dict->size);
}

ui8 DictCompare(HalSMCompiler* hsmc,Dict* a,Dict* b)
{
    if (a->size!=b->size){return 0;}
    for (ui64 i=0;i<a->size;i++) {
        if (HalSMVariable_Compare(hsmc,a->elements[i]->key,b->elements[i]->key)==0||HalSMVariable_Compare(hsmc,a->elements[i]->value,b->elements[i]->value)==0){return 0;}
    }
    return 1;
}

void DictFree(HalSMMemory* memory,Dict* dict)
{
    for (ui64 i=0;i<dict->size;i++)
        HalSMMemory_free(memory,dict->elements[i]);
    HalSMMemory_free(memory,dict->elements);
    HalSMMemory_free(memory,dict);
}

void DictFreeE(HalSMMemory* memory,Dict* dict)
{
    for(ui64 i=0;i<dict->size;i++)
        DictElementFree(memory,dict->elements[i]);
    HalSMMemory_free(memory,dict->elements);
    HalSMMemory_free(memory,dict);
}

void DictClear(HalSMMemory* memory,Dict* dict)
{
    for (ui64 i=0;i<dict->size;i++)
        HalSMMemory_free(memory,dict->elements[i]);
    dict->elements=HalSMMemory_realloc(memory,dict->elements,0);
    dict->size=0;
}

HalStringFormatChar* Dict_to_print(HalSMCompiler* hsmc,Dict* harr)
{
    HalSMArray* out=HalSMArray_init(hsmc->memory);
    if (harr->size>0) {
        DictElement** p=&harr->elements[0];
        HalStringFormatChar* keystr;
        HalStringFormatChar* valuestr;
        for (ui64 i=0;i<harr->size;i++,p++) {
            keystr=HalSMVariable_to_str(hsmc,(*p)->key,1);
            valuestr=HalSMVariable_to_str(hsmc,(*p)->value,1);
            HalSMArray_add(hsmc->memory,out,HalSMVariable_init_str(hsmc->memory,ConcatenateStrings(hsmc->memory,keystr,ConcatenateStrings(hsmc->memory,U8H(":"),valuestr))));
        }
    }
    HalStringFormatChar* joinout=HalSMArray_join_str(hsmc->memory,out,U8H(", "));
    ui64 l=string_len(joinout);
    HalStringFormatChar* outs=HalSMMemory_realloc(hsmc->memory,joinout,(l+3)*sizeof(HalStringFormatChar));
    HalStringFormatChar* po=&outs[l];
    HalStringFormatChar* pj=&outs[l-1];
    for (ui64 i=0;i<l;i++,po--,pj--){*po=*pj;}
    outs[0]=(HalStringFormatChar){0,97,0};
    outs[l+1]=(HalStringFormatChar){0,99,0};
    outs[l+2]=(HalStringFormatChar){0,0,0};
    return outs;
}

void DictAppendDict(HalSMMemory* memory,Dict* a,Dict* b)
{
    a->elements=HalSMMemory_realloc(memory,a->elements,(a->size+b->size)*sizeof(DictElement*));
    DictElement** p=&a->elements[a->size];
    DictElement** pn=&b->elements[0];
    for (ui64 i=0;i<b->size;i++){*p++=*pn++;}
    a->size=a->size+b->size;
}

void DictRemove(HalSMMemory* memory,Dict* a,ui64 index)
{
    DictElement** arr=HalSMMemory_malloc(memory,(a->size-1)*sizeof(DictElement*));   
    DictElement** ap=arr;
    DictElement** ae=a->elements;
    for (ui64 i=0;i<a->size;i++) {
        if(i!=index){*ap++=*ae++;}
    }
    a->size=a->size-1;
    HalSMMemory_free(memory,a->elements);
    a->elements=arr;
}


//HalSMVariable


HalSMVariable* HalSMVariable_init(HalSMMemory* memory,void* value,HalSMVariableType type)
{
    HalSMVariable* out=HalSMMemory_malloc(memory,sizeof(HalSMVariable));
    out->type=type;
    out->value=value;
    return out;
}

void HalSMVariable_AsVar(void* var,HalSMVariable* arg)
{
    HalSMVariableType type=arg->type;
    void* value=arg->value;
    if(type==HalSMVariableType_int){(*(HalSMInteger*)var)=*(HalSMInteger*)value;}
    else if(type==HalSMVariableType_char){(*(HalStringFormatChar*)var)=(*(HalStringFormatChar*)value);}
    else if(type==HalSMVariableType_double){(*(HalSMDouble*)var)=(*(HalSMDouble*)value);}
    else if(type==HalSMVariableType_void){(*(HalSMVoid*)var)=*(HalSMVoid*)value;}
    else if(type==HalSMVariableType_HalSMArray){*((HalSMArray*)var)=*((HalSMArray*)value);}
    else if(type==HalSMVariableType_str){(*(HalStringFormatChar**)var)=(*(HalStringFormatChar**)value);}
    else if(type==HalSMVariableType_int_array){*((int**)var)=*((int**)value);}
    else if(type==HalSMVariableType_HalSMFunctionC){(*(HalSMFunctionC*)var)=*(HalSMFunctionC*)value;}
    else if(type==HalSMVariableType_HalSMRunClassC){(*(HalSMRunClassC*)var)=*(HalSMRunClassC*)value;}
    else if(type==HalSMVariableType_HalSMError){(*(HalSMError*)var)=*(HalSMError*)value;}
    else if(type==HalSMVariableType_HalSMNull){(*(HalSMNull*)var)=*(HalSMNull*)value;}
    else if(type==HalSMVariableType_HalSMRunFunc){(*(HalSMRunFunc*)var)=*(HalSMRunFunc*)value;}
    else if(type==HalSMVariableType_HalSMRunFuncC){(*(HalSMRunFuncC*)var)=*(HalSMRunFuncC*)value;}
    else if(type==HalSMVariableType_HalSMLocalFunction){*((HalSMLocalFunction*)var)=*(HalSMLocalFunction*)value;}
    else if(type==HalSMVariableType_HalSMCModule){(*(HalSMCModule*)var)=*(HalSMCModule*)value;}
    else if(type==HalSMVariableType_HalSMModule){(*(HalSMModule*)var)=*(HalSMModule*)value;}
    else if(type==HalSMVariableType_HalSMClassC){(*(HalSMClassC*)var)=*(HalSMClassC*)value;}
    else if(type==HalSMVariableType_HalSMCompiler){(*(HalSMCompiler*)var)=*(HalSMCompiler*)value;}
    else if(type==HalSMVariableType_HalSMRunClass){(*(HalSMRunClass*)var)=*(HalSMRunClass*)value;}
    else if(type==HalSMVariableType_HalSMDoubleGet){(*(HalSMDoubleGet*)var)=*(HalSMDoubleGet*)value;}
    else if(type==HalSMVariableType_HalSMClass){(*(HalSMClass*)var)=*(HalSMClass*)value;}
    else if(type==HalSMVariableType_HalSMVar){(*(HalSMVar*)var)=*(HalSMVar*)value;}
    else if(type==HalSMVariableType_HalSMPlus){(*(HalSMPlus*)var)=*(HalSMPlus*)value;}
    else if(type==HalSMVariableType_HalSMMinus){(*(HalSMMinus*)var)=*(HalSMMinus*)value;}
    else if(type==HalSMVariableType_HalSMMult){(*(HalSMMult*)var)=*(HalSMMult*)value;}
    else if(type==HalSMVariableType_HalSMDivide){(*(HalSMDivide*)var)=*(HalSMDivide*)value;}
    else if(type==HalSMVariableType_HalSMEqual){(*(HalSMEqual*)var)=*(HalSMEqual*)value;}
    else if(type==HalSMVariableType_HalSMNotEqual){(*(HalSMNotEqual*)var)=*(HalSMNotEqual*)value;}
    else if(type==HalSMVariableType_HalSMMore){(*(HalSMMore*)var)=*(HalSMMore*)value;}
    else if(type==HalSMVariableType_HalSMLess){(*(HalSMLess*)var)=*(HalSMLess*)value;}
    else if(type==HalSMVariableType_HalSMBool){(*(ui8*)var)=*(ui8*)value;}
    else if(type==HalSMVariableType_HalSMDict){(*(Dict*)var)=*(Dict*)value;}
    else if(type==HalSMVariableType_HalSMSetVar){(*(HalSMSetVar*)var)=*(HalSMSetVar*)value;}
    else if(type==HalSMVariableType_HalSMReturn){(*(HalSMReturn*)var)=*(HalSMReturn*)value;}
    else if(type==HalSMVariableType_HalSMFunctionCTypeDef){(*(HalSMFunctionCTypeDef*)var)=*(HalSMFunctionCTypeDef*)value;}
    else if(type==HalSMVariableType_HalSMFunctionArray){(*(HalSMFunctionArray*)var)=*(HalSMFunctionArray*)value;}
    else if(type==HalSMVariableType_unsigned_int){(*(ui64*)var)=*(ui64*)value;}
    else if(type==HalSMVariableType_custom){(*(HalSMVariableCustom*)var)=*(HalSMVariableCustom*)value;}
    else if(type==HalSMVariableType_HalSMMoreEqual){(*(HalSMMoreEqual*)var)=*(HalSMMoreEqual*)value;}
    else if(type==HalSMVariableType_HalSMLessEqual){(*(HalSMLessEqual*)var)=*(HalSMLessEqual*)value;}
    else if(type==HalSMVariableType_HalSMShift){(*(HalSMShift*)var)=*(HalSMShift*)value;}
    else if(type==HalSMVariableType_HalSMPow){(*(HalSMPow*)var)=*(HalSMPow*)value;}
    else if(type==HalSMVariableType_HalSMModulo){(*(HalSMModulo*)var)=*(HalSMModulo*)value;}
    else if(type==HalSMVariableType_HalSMAND){(*(HalSMAND*)var)=*(HalSMAND*)value;}
    else if(type==HalSMVariableType_HalSMOR){(*(HalSMOR*)var)=*(HalSMOR*)value;}
    else if(type==HalSMVariableType_HalSMNegate){(*(HalSMNegate*)var)=*(HalSMNegate*)value;}
    else if(type==HalSMVariableType_HalSMNegativeSign){(*(HalSMNegativeSign*)var)=*(HalSMNegativeSign*)value;}
    else if(type==HalSMVariableType_HalSMGOTO){(*(HalSMGOTO*)var)=*(HalSMGOTO*)value;}
    else if(type==HalSMVariableType_HalSMGOTOSector){(*(HalSMGOTOSector*)var)=*(HalSMGOTOSector*)value;}
    else if(type==HalSMVariableType_HalSMInlineIf){(*(HalSMInlineIf*)var)=*(HalSMInlineIf*)value;}
    else if(type==HalSMVariableType_HalSMInlineElse){(*(HalSMInlineElse*)var)=*(HalSMInlineElse*)value;}
    else if(type==HalSMVariableType_HalSMBreak){(*(HalSMBreak*)var)=*(HalSMBreak*)value;}
    else if(type==HalSMVariableType_HalSMXOR){(*(HalSMXOR*)var)=*(HalSMXOR*)value;}
    else if(type==HalSMVariableType_HalSMBAND){(*(HalSMBAND*)var)=*(HalSMBAND*)value;}
    else if(type==HalSMVariableType_HalSMBOR){(*(HalSMBOR*)var)=*(HalSMBOR*)value;}
    else if(type==HalSMVariableType_HalSMBNOT){(*(HalSMBNOT*)var)=*(HalSMBNOT*)value;}
    else if(type==HalSMVariableType_fv){(*(HalSMFindValue*)var)=*(HalSMFindValue*)value;}
    else if(type==HalSMVariableType_HalSMOP){(*(HalSMOP*)var)=*(HalSMOP*)value;}
}

HalSMVariable* HalSMVariable_init_str(HalSMMemory* memory,HalStringFormatChar* str) {
    HalStringFormatChar** d=HalSMMemory_malloc(memory,sizeof(HalStringFormatChar*));
    *d=HalSMMemory_calloc(memory,string_len(str)+1,sizeof(HalStringFormatChar));
    stringncpy(*d,str,string_len(str));
    return HalSMVariable_init(memory,d,HalSMVariableType_str);
}

HalSMVariable* HalSMVariable_init_int(HalSMMemory* memory,HalSMInteger* n) {
    return HalSMVariable_init(memory,n,HalSMVariableType_int);
}

HalSMVariable* HalSMVariable_init_double(HalSMMemory* memory,HalSMDouble* d) {
    return HalSMVariable_init(memory,d,HalSMVariableType_double);
}

HalSMVariable* HalSMVariable_init_bool(HalSMMemory* memory,ui8 b) {
    ui8* c=HalSMMemory_malloc(memory,sizeof(ui8));
    *c=b;
    return HalSMVariable_init(memory,c,HalSMVariableType_HalSMBool);
}

HalSMVariable* HalSMVariable_init_unsigned_int(HalSMMemory* memory,ui64 n) {
    ui64* i=HalSMMemory_malloc(memory,sizeof(ui64));
    *i=n;
    return HalSMVariable_init(memory,i,HalSMVariableType_unsigned_int);
}

HalSMVariable* HalSMVariable_init_char(HalSMMemory* memory,HalStringFormatChar c) {
    HalStringFormatChar* out=HalSMMemory_malloc(memory,sizeof(HalStringFormatChar));
    *out=c;
    return HalSMVariable_init(memory,out,HalSMVariableType_char);
}

HalStringFormatChar* HalSMVariable_to_str(HalSMCompiler* hsmc,HalSMVariable* a,ui8 isToPrint)
{
    HalStringFormatChar* out;
    HalStringFormatChar* c;
    if (a->type==HalSMVariableType_int) {
        out=HalSMInteger_toString(hsmc->memory,(HalSMInteger*)a->value,0);
    } else if (a->type==HalSMVariableType_unsigned_int) {
        out=HalSMInteger_toString(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,*(ui64*)a->value),0);
    } else if (a->type==HalSMVariableType_double) {
        out=HalSMDouble_toString(hsmc->memory,(HalSMDouble*)a->value,0);
    } else if (a->type==HalSMVariableType_str) {
        if (isToPrint) {
            ui64 l=string_len(*(HalStringFormatChar**)a->value);
            c=HalSMMemory_malloc(hsmc->memory,(l+3)*sizeof(HalStringFormatChar));
            c[0]=ASCIIH('"');
            c[1]=ASCIIH('\0');
            string_cat(c,*(HalStringFormatChar**)a->value);
            c[l+1]=ASCIIH('"');
            c[l+2]=ASCIIH('\0');
            out=c;
        } else {
            ui64 l=string_len(*(HalStringFormatChar**)a->value);
            c=HalSMMemory_malloc(hsmc->memory,(l+1)*sizeof(HalStringFormatChar));
            string_cpy(c,*(HalStringFormatChar**)a->value);
            out=c;
        }
    } else if (a->type==HalSMVariableType_HalSMNull) {
        out=U8H("Null");
    } else if (a->type==HalSMVariableType_void) {
        out=U8H("(void)");
    } else if (a->type==HalSMVariableType_char) {
        c=HalSMMemory_malloc(hsmc->memory,2*sizeof(HalStringFormatChar));
        c[0]=*(HalStringFormatChar*)a->value;
        c[1]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMArray) {
        out=HalSMArray_to_print(hsmc,(HalSMArray*)a->value);
    } else if (a->type==HalSMVariableType_HalSMRunClassC) {
        c=HalSMMemory_malloc(hsmc->memory,(21+string_len(((HalSMRunClassC*)a->value)->name))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<Running Class C ("));
        string_cat(c,((HalSMRunClassC*)a->value)->name);
        string_cat(c,U8H(")>"));
        c[string_len(((HalSMRunClassC*)a->value)->name)+20]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMLocalFunction) {
        HalStringFormatChar* name=((HalSMLocalFunction*)a->value)->name;
        c=HalSMMemory_malloc(hsmc->memory,(19+string_len(name))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<LocalFunction ("));
        string_cat(c,name);
        string_cat(c,U8H(")>"));
        c[string_len(name)+18]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMFunctionC) {
        HalStringFormatChar* name=ConcatenateStrings(hsmc->memory,U8H("0x"),
        HalSMInteger_toString(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,(ui64)((HalSMFunctionC*)a->value)->func),1));
        c=HalSMMemory_malloc(hsmc->memory,35*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<Function C at ("));
        string_cat(c,name);
        string_cat(c,U8H(")>"));
        c[string_len(name)+18]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMCModule) {
        HalStringFormatChar* name=((HalSMCModule*)a->value)->name;
        c=HalSMMemory_malloc(hsmc->memory,(14+string_len(name))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<Module C ("));
        string_cat(c,name);
        string_cat(c,U8H(")>"));
        c[13+string_len(name)]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMModule) {
        HalStringFormatChar* name=((HalSMModule*)a->value)->name;
        c=HalSMMemory_malloc(hsmc->memory,(12+string_len(name))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<Module ("));
        string_cat(c,name);
        string_cat(c,U8H(")>"));
        c[13+string_len(name)]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMClassC) {
        HalStringFormatChar* name=((HalSMClassC*)a->value)->name;
        c=HalSMMemory_malloc(hsmc->memory,(13+string_len(name))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<Class C ("));
        string_cat(c,name);
        string_cat(c,U8H(")>"));
        c[12+string_len(name)]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMRunClassC) {
        HalStringFormatChar* name=((HalSMRunClassC*)a->value)->name;
        c=HalSMMemory_malloc(hsmc->memory,(16+string_len(name))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<RunClass C ("));
        string_cat(c,name);
        string_cat(c,U8H(")>"));
        c[15+string_len(name)]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMBool) {
        out=(*(ui8*)a->value)?U8H("true"):U8H("false");
    } else if (a->type==HalSMVariableType_HalSMRunClass) {
        HalStringFormatChar* name=((HalSMRunClass*)a->value)->name;
        c=HalSMMemory_malloc(hsmc->memory,(16+string_len(name))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<RunClass ("));
        string_cat(c,name);
        string_cat(c,U8H(")>"));
        c[13+string_len(name)]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMClass) {
        HalStringFormatChar* name=((HalSMClass*)a->value)->name;
        c=HalSMMemory_malloc(hsmc->memory,(13+string_len(name))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<Class ("));
        string_cat(c,name);
        string_cat(c,U8H(")>"));
        c[10+string_len(name)]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMVar) {
        HalStringFormatChar* name=*(HalStringFormatChar**)((HalSMVar*)a->value)->name->value;
        c=HalSMMemory_malloc(hsmc->memory,(13+string_len(name))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<HalSMVar ("));
        string_cat(c,name);
        string_cat(c,U8H(")>"));
        c[14+string_len(name)]=ASCIIH('\0');
        out=c;
    } else if (a->type==HalSMVariableType_HalSMDict) {
        out=Dict_to_print(hsmc,(Dict*)a->value);
    } else if (a->type==HalSMVariableType_custom) {
        out=((HalSMVariableCustom*)a->value)->type->toString(hsmc,a);
    } else if (a->type==HalSMVariableType_HalSMError) {
        HalStringFormatChar* error=((HalSMError*)a->value)->error;
        HalStringFormatChar* line=HalSMInteger_toString(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,((HalSMError*)a->value)->ind->line),0);
        c=HalSMMemory_malloc(hsmc->memory,(21+string_len(error)+string_len(line))*sizeof(HalStringFormatChar));
        string_cpy(c,U8H("<Error line="));
        string_cat(c,line);
        string_cat(c,U8H("; error="));
        string_cat(c,error);
        string_cat(c,U8H(">"));
        c[22+string_len(error)+string_len(line)]=ASCIIH('\0');
        out=c;
    } else {
        out=ConcatenateStrings(hsmc->memory,U8H("<"),ConcatenateStrings(hsmc->memory,HalSMVariableTypeName[a->type],U8H(">")));
    }
    return out;
}

ui8 HalSMVariable_Compare(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1)
{
    HalSMVariableType type=v0->type;
    if (type!=v1->type)return 0;
    void* var=v0->value;
    void* value=v1->value;
    if(type==HalSMVariableType_int){return HalSMInteger_isEqual((HalSMInteger*)var,(HalSMInteger*)value);}
    else if(type==HalSMVariableType_char){return HSFCharCompare(*(HalStringFormatChar*)var,*(HalStringFormatChar*)value);}
    else if(type==HalSMVariableType_double){return HalSMDouble_isEqual((HalSMDouble*)var,(HalSMDouble*)value);}
    else if(type==HalSMVariableType_void){return 1;}
    else if(type==HalSMVariableType_HalSMArray){return HalSMArray_compare(hsmc,(HalSMArray*)var,(HalSMArray*)value);}
    else if(type==HalSMVariableType_str){return HSFCompare(*(HalStringFormatChar**)var,*(HalStringFormatChar**)value);}
    else if(type==HalSMVariableType_HalSMFunctionC){return ((HalSMFunctionC*)var)->func==((HalSMFunctionC*)value)->func;}
    else if(type==HalSMVariableType_HalSMRunClassC){return HSFCompare(((HalSMRunClassC*)var)->name,((HalSMRunClassC*)value)->name)&&DictCompare(hsmc,((HalSMRunClassC*)var)->vrs,((HalSMRunClassC*)value)->vrs)&&DictCompare(hsmc,((HalSMRunClassC*)var)->funcs,((HalSMRunClassC*)value)->funcs);}
    else if(type==HalSMVariableType_HalSMError){return ((HalSMError*)var)->ind==((HalSMError*)value)->ind&&HSFCompare(((HalSMError*)var)->error,((HalSMError*)value)->error);}
    else if(type==HalSMVariableType_HalSMNull){return 1;}
    else if(type==HalSMVariableType_HalSMRunFunc){return 1;}
    else if(type==HalSMVariableType_HalSMRunFuncC){return 1;}
    else if(type==HalSMVariableType_HalSMLocalFunction){return HSFCompare(((HalSMLocalFunction*)var)->name,((HalSMLocalFunction*)value)->name)&&HalSMArray_compare(hsmc,((HalSMLocalFunction*)var)->args,((HalSMLocalFunction*)value)->args)&&HalSMArray_compare(hsmc,((HalSMLocalFunction*)var)->func,((HalSMLocalFunction*)value)->func)&&DictCompare(hsmc,((HalSMLocalFunction*)var)->vars,((HalSMLocalFunction*)value)->vars);}
    else if(type==HalSMVariableType_HalSMCModule){return DictCompare(hsmc,((HalSMCModule*)var)->lfuncs,((HalSMCModule*)value)->lfuncs)&&DictCompare(hsmc,((HalSMCModule*)var)->vrs,((HalSMCModule*)value)->vrs)&&DictCompare(hsmc,((HalSMCModule*)var)->classes,((HalSMCModule*)value)->classes)&&HSFCompare(((HalSMCModule*)var)->name,((HalSMCModule*)value)->name);}
    else if(type==HalSMVariableType_HalSMModule){return HSFCompare(((HalSMModule*)var)->name,((HalSMModule*)value)->name)&&((HalSMModule*)var)->hsmc==((HalSMModule*)value)->hsmc;}
    else if(type==HalSMVariableType_HalSMClassC){return HSFCompare(((HalSMClassC*)var)->name,((HalSMClassC*)value)->name)&&DictCompare(hsmc,((HalSMClassC*)var)->vrs,((HalSMClassC*)value)->vrs)&&DictCompare(hsmc,((HalSMClassC*)var)->funcs,((HalSMClassC*)value)->funcs);}
    else if(type==HalSMVariableType_HalSMCompiler){return v0->value==v1->value;}
    else if(type==HalSMVariableType_HalSMRunClass){return HSFCompare(((HalSMRunClass*)var)->name,((HalSMRunClass*)value)->name)&&DictCompare(hsmc,((HalSMRunClass*)var)->funcs,((HalSMRunClass*)value)->funcs)&&DictCompare(hsmc,((HalSMRunClass*)var)->vars,((HalSMRunClass*)value)->vars);}
    else if(type==HalSMVariableType_HalSMDoubleGet){return HSFCompare(((HalSMDoubleGet*)var)->st,((HalSMDoubleGet*)value)->st);}
    else if(type==HalSMVariableType_HalSMClass){return HSFCompare(((HalSMClass*)var)->name,((HalSMClass*)value)->name)&&DictCompare(hsmc,((HalSMClass*)var)->funcs,((HalSMClass*)value)->funcs)&&DictCompare(hsmc,((HalSMClass*)var)->vars,((HalSMClass*)value)->vars);}
    else if(type==HalSMVariableType_HalSMVar){return HSFCompare(*(HalStringFormatChar**)((HalSMVar*)var)->name->value,*(HalStringFormatChar**)((HalSMVar*)value)->name->value);}
    else if(type==HalSMVariableType_HalSMPlus){return 1;}
    else if(type==HalSMVariableType_HalSMMinus){return 1;}
    else if(type==HalSMVariableType_HalSMMult){return 1;}
    else if(type==HalSMVariableType_HalSMDivide){return 1;}
    else if(type==HalSMVariableType_HalSMEqual){return 1;}
    else if(type==HalSMVariableType_HalSMNotEqual){return 1;}
    else if(type==HalSMVariableType_HalSMMore){return 1;}
    else if(type==HalSMVariableType_HalSMLess){return 1;}
    else if(type==HalSMVariableType_HalSMBool){return *(ui8*)var==*(ui8*)value;}
    else if(type==HalSMVariableType_HalSMSetVar){return HSFCompare(*(HalStringFormatChar**)((HalSMSetVar*)var)->name->value,*(HalStringFormatChar**)((HalSMSetVar*)value)->name->value)&&HSFCompare(((HalSMSetVar*)var)->value,((HalSMSetVar*)value)->value);}
    else if(type==HalSMVariableType_HalSMReturn){return HSFCompare(((HalSMReturn*)var)->value,((HalSMReturn*)value)->value);}
    else if(type==HalSMVariableType_HalSMFunctionArray){return ((HalSMFunctionArray*)var)->type==((HalSMFunctionArray*)value)->type&&HalSMArray_compare(hsmc,((HalSMFunctionArray*)var)->args,((HalSMFunctionArray*)value)->args);}
    else if(type==HalSMVariableType_unsigned_int){return *(ui64*)var==*(ui64*)value;}
    else if(type==HalSMVariableType_custom){return ((HalSMVariableCustom*)var)->type->isEqual(hsmc,var,value);}
    else if(type==HalSMVariableType_HalSMMoreEqual){return 1;}
    else if(type==HalSMVariableType_HalSMLessEqual){return 1;}
    else if(type==HalSMVariableType_HalSMShift){return ((HalSMShift*)var)->c==((HalSMShift*)value)->c;}
    else if(type==HalSMVariableType_HalSMPow){return 1;}
    else if(type==HalSMVariableType_HalSMModulo){return 1;}
    else if(type==HalSMVariableType_HalSMAND){return 1;}
    else if(type==HalSMVariableType_HalSMOR){return 1;}
    else if(type==HalSMVariableType_HalSMNegate){return 1;}
    else if(type==HalSMVariableType_HalSMNegativeSign){return 1;}
    else if(type==HalSMVariableType_HalSMGOTO){return HSFCompare(((HalSMGOTO*)var)->sector,((HalSMGOTO*)value)->sector);}
    else if(type==HalSMVariableType_HalSMGOTOSector){return HSFCompare(((HalSMGOTOSector*)var)->sector,((HalSMGOTOSector*)value)->sector)&&((HalSMGOTOSector*)var)->index==((HalSMGOTOSector*)value)->index;}
    else if(type==HalSMVariableType_HalSMInlineIf){return 1;}
    else if(type==HalSMVariableType_HalSMInlineElse){return 1;}
    else if(type==HalSMVariableType_HalSMBreak){return 1;}
    else if(type==HalSMVariableType_HalSMXOR){return 1;}
    else if(type==HalSMVariableType_HalSMBAND){return 1;}
    else if(type==HalSMVariableType_HalSMBOR){return 1;}
    else if(type==HalSMVariableType_HalSMBNOT){return 1;}
    else if(type==HalSMVariableType_fv){return 0;}
    return 0;
}

void HalSMVariable_free(HalSMMemory* memory,HalSMVariable* arg)
{
    /*if (arg->type==HalSMVariableType_HalSMArray) {
        HalSMArray_free(memory,(HalSMArray*)arg->value);
    } else if(arg->type==HalSMVariableType_int) {
        HalSMInteger_Free(memory,(HalSMInteger*)arg->value);
    } else if(arg->type==HalSMVariableType_double) {
        HalSMDouble_Free(memory,(HalSMDouble*)arg->value);
    } else if(arg->type==HalSMVariableType_str) {
        //HalSMMemory_free(memory,*(HalStringFormatStr*)arg->value);
        HalSMMemory_free(memory,arg->value);
    } else if(arg->type==HalSMVariableType_HalSMClassC) {
        HalSMClassC* classc=(HalSMClassC*)arg->value;
        DictFreeE(memory,classc->funcs);
        DictFreeE(memory,classc->vrs);
        HalSMMemory_free(memory,classc->name);
        HalSMMemory_free(memory,classc);
    } else if(arg->type==HalSMVariableType_HalSMRunClassC) {
        HalSMRunClassC* classc=(HalSMRunClassC*)arg->value;
        DictFreeE(memory,classc->funcs);
        DictFreeE(memory,classc->vrs);
        HalSMMemory_free(memory,classc->name);
        HalSMMemory_free(memory,classc);
    } else if(arg->type==HalSMVariableType_HalSMCModule) {
        HalSMCModule* module=(HalSMCModule*)arg->value;
        DictFreeE(memory,module->classes);
        DictFreeE(memory,module->lfuncs);
        DictFreeE(memory,module->vrs);
        HalSMMemory_free(memory,module->name);
        HalSMMemory_free(memory,module);
    } else if(arg->type==HalSMVariableType_HalSMModule) {
        HalSMModule* module=(HalSMModule*)arg->value;
        HalSMMemory_free(memory,module->name);
        HalSMMemory_free(memory,module);
    } else if(arg->type==HalSMVariableType_HalSMLocalFunction) {
        HalSMLocalFunction* lf=(HalSMLocalFunction*)arg->value;
        DictFreeE(memory,lf->vars);
        HalSMArray_free(memory,lf->args);
        HalSMArray_free(memory,lf->func);
        HalSMVariable_free(memory,lf->module);
        HalSMMemory_free(memory,lf->name);
        HalSMMemory_free(memory,lf);
    } else if(arg->type==HalSMVariableType_HalSMVar) {
        HalSMMemory_free(memory,((HalSMVar*)arg->value)->name);
        HalSMMemory_free(memory,arg->value);
    } else if(arg->type==HalSMVariableType_HalSMSetArg) {
        HalSMSetArg* a=(HalSMSetArg*)arg->value;
        HalSMMemory_free(memory,a->name);
        HalSMMemory_free(memory,a->value);
        HalSMMemory_free(memory,a);
    } else if(arg->type==HalSMVariableType_HalSMError) {
        HalSMError* e=(HalSMError*)arg->value;
        HalSMMemory_free(memory,e->error);
        HalSMMemory_free(memory,e->ind->name);
        HalSMMemory_free(memory,e);
    } else if(arg->type==HalSMVariableType_HalSMRunFunc) {
        HalSMRunFunc* f=(HalSMRunFunc*)arg->value;
        HalSMArray_free(memory,f->args);
        HalSMArray_free(memory,f->pointer);
        HalSMLocalFunction* lf=f->func;
        DictFreeE(memory,lf->vars);
        HalSMArray_free(memory,lf->args);
        HalSMArray_free(memory,lf->func);
        HalSMVariable_free(memory,lf->module);
        HalSMMemory_free(memory,lf->name);
        HalSMMemory_free(memory,lf);
        HalSMMemory_free(memory,f);
    } else if(arg->type==HalSMVariableType_HalSMRunFuncC) {
        HalSMRunFuncC* f=(HalSMRunFuncC*)arg->value;
        HalSMArray_free(memory,f->args);
        HalSMArray_free(memory,f->pointer);
        HalSMMemory_free(memory,f);
    } else if(arg->type==HalSMVariableType_HalSMCompiler) {
        HalSMCompiler_free((HalSMCompiler*)arg->value);
    } else if(arg->type==HalSMVariableType_HalSMClass) {
        HalSMClass* c=(HalSMClass*)arg->value;
        DictFreeE(memory,c->funcs);
        DictFreeE(memory,c->vars);
        HalSMMemory_free(memory,c->name);
        HalSMMemory_free(memory,c);
    } else if(arg->type==HalSMVariableType_HalSMRunClass) {
        HalSMRunClass* c=(HalSMRunClass*)arg->value;
        DictFreeE(memory,c->funcs);
        DictFreeE(memory,c->vars);
        HalSMMemory_free(memory,c->name);
        HalSMMemory_free(memory,c);
    } else if(arg->type==HalSMVariableType_HalSMDict) {
        DictFreeE(memory,(Dict*)arg->value);
    } else if(arg->type==HalSMVariableType_HalSMReturn) {
        HalSMMemory_free(memory,((HalSMReturn*)arg->value)->value);
        HalSMMemory_free(memory,arg->value);
    } else if(arg->type==HalSMVariableType_HalSMFunctionArray) {
        HalSMFunctionArray* fa=(HalSMFunctionArray*)arg->value;
        HalSMArray_free(memory,fa->args);
        HalSMMemory_free(memory,fa);
    } else if(arg->type==HalSMVariableType_custom) {
        HalSMVariableCustom* c=(HalSMVariableCustom*)arg->value;
        DictFreeE(memory,c->funcs);
        DictFreeE(memory,c->vars);
        HalSMMemory_free(memory,c->value);
        HalSMMemory_free(memory,c->type);
        HalSMMemory_free(memory,c);
    } else {
        HalSMMemory_free(memory,arg->value);
    }*/
    HalSMMemory_free(memory,arg);
}

HalSMVariableCustom* HalSMVariableCustom_init(HalSMMemory* memory,HalSMVariableTypeCustom* type,void* value,Dict* vars,Dict* funcs)
{
    HalSMVariableCustom* out=HalSMMemory_malloc(memory,sizeof(HalSMVariableCustom));
    out->type=type;
    out->value=value;
    out->vars=vars;
    out->funcs=funcs;
    return out;
}

HalSMVariableTypeCustom* HalSMVariableTypeCustom_init(HalSMMemory* memory,HalSMVariableTypeCustom type)
{
    HalSMVariableTypeCustom* out=HalSMMemory_malloc(memory,sizeof(HalSMVariableTypeCustom));
    *out=type;
    return out;
}

HalSMSetVar* HalSMSetVar_init(HalSMMemory* memory,HalStringFormatChar* name,HalStringFormatChar* value,HalSMVariable* pointer)
{
    HalSMSetVar* out=HalSMMemory_malloc(memory,sizeof(HalSMSetVar));
    out->name=HalSMVariable_init_str(memory,HalSMCompiler_removeEndSpaces(memory,name));
    out->value=value;
    out->pointer=pointer;
    return out;
}

HalSMArray* HalSMArray_init(HalSMMemory* memory)
{
    HalSMArray* out=HalSMMemory_malloc(memory,sizeof(HalSMArray));
    out->size=0;
    out->arr=HalSMMemory_malloc(memory,0);
    return out;
}

HalSMArray* HalSMArray_split_str(HalSMMemory* memory,HalStringFormatChar* str,HalStringFormatChar* spl)
{
    HalSMArray* out=HalSMMemory_malloc(memory,sizeof(HalSMArray));
    out->size=0;
    out->arr=HalSMMemory_malloc(memory,0);
    ui64 slen=string_len(str);
    ui64 plen=string_len(spl);
    if (slen<plen) {
        return HalSMArray_init_with_elements(memory,(HalSMVariable*[]){HalSMVariable_init_str(memory,U8H(""))},1);
    }
    ui64 i=0;
    HalStringFormatChar* arr=NOTHALSMNULLPOINTER;
    HalStringFormatChar* temp=HalSMMemory_malloc(memory,sizeof(HalStringFormatChar));
    *temp=ASCIIH('\0');
    ui64 d=0;

    while (i<slen) {
        if (i<slen-plen+1) {
            arr=SubString(memory,str,i,i+plen);
            if (HSFCompare(arr,spl)) {
                //temp=HalSMMemory_realloc(memory,temp,(d+1)*sizeof(HalStringFormatChar));
                //temp[d]=ASCIIH('\0');
                HalSMArray_add(memory,out,HalSMVariable_init_str(memory,temp));
                i+=plen;
                d=0;
                temp=HalSMMemory_malloc(memory,sizeof(HalStringFormatChar));
                *temp=ASCIIH('\0');
            } else {
                d++;
                temp=HalSMMemory_realloc(memory,temp,(d+1)*sizeof(HalStringFormatChar));
                temp[d-1]=str[i];
                temp[d]=ASCIIH('\0');
                i++;
            }
        } else {
            d++;
            temp=HalSMMemory_realloc(memory,temp,(d+1)*sizeof(HalStringFormatChar));
            temp[d-1]=str[i];
            temp[d]=ASCIIH('\0');
            i++;
        }
    }

    if (string_len(temp)>0) {
        //temp=HalSMMemory_realloc(memory,temp,(d+1)*sizeof(HalStringFormatChar));
        //temp[d]=ASCIIH('\0');
        HalSMArray_add(memory,out,HalSMVariable_init_str(memory,temp));
    } else {
        HalSMArray_add(memory,out,HalSMVariable_init_str(memory,U8H("")));
    }

    HalSMMemory_free(memory,arr);
    HalSMMemory_free(memory,temp);
    return out;
}

i64 HalSMArray_index(HalSMCompiler* hsmc,HalSMArray* harr,HalSMVariable* value)
{
    for (ui64 i=0;i<harr->size;i++)
    {
        if (HalSMVariable_Compare(hsmc,harr->arr[i],value))return i;
    }
    return -1;
}

i64 HalSMArray_right_index(HalSMCompiler* hsmc,HalSMArray* harr,HalSMVariable* value)
{
    for (ui64 i=harr->size-1;;i++) {
        if (HalSMVariable_Compare(hsmc,harr->arr[i],value))return i;
        if (i==0){break;}
    }
    return -1;
}

void HalSMArray_add(HalSMMemory* memory,HalSMArray* harr,HalSMVariable* value)
{
    HalSMVariable** old=harr->arr;
    HalSMVariable** oldp=harr->arr;
    harr->arr=HalSMMemory_malloc(memory,(harr->size+1)*sizeof(HalSMVariable*));
    HalSMVariable** p=harr->arr;
    for (ui64 i=0;i<harr->size;i++){*p++=*oldp++;}
    harr->arr[harr->size++]=value;
    HalSMMemory_free(memory,old);
}

void HalSMArray_set(HalSMArray* harr,HalSMVariable* value,ui64 index)
{
    if (index<harr->size) {
        harr->arr[index]=value;
    }
}

void HalSMArray_remove(HalSMMemory* memory,HalSMArray* harr,ui64 index)
{
    if(index>=harr->size)return;
    HalSMVariable** arr=HalSMMemory_malloc(memory,(harr->size-1)*sizeof(HalSMVariable*));
    HalSMVariable** ap=arr;
    HalSMVariable** aa=harr->arr;
    /*for (ui64 i=0;i<harr->size;i++) {
        if(i!=index){*ap++=*aa++;}
    }*/
    ui64 i;
    for(i=0;i<index;i++) {
        *ap++=*aa++;
    }
    if(index<harr->size-1) {
        aa++;
    }
    for(i=index+1;i<harr->size;i++) {
        *ap++=*aa++;
    }
    harr->size=harr->size-1;
    HalSMMemory_free(memory,harr->arr);
    harr->arr=arr;
}

HalSMArray* HalSMArray_reverse(HalSMMemory* memory,HalSMArray* harr)
{
    HalSMArray* out=HalSMArray_init(memory);
    for (ui64 i=harr->size-1;i--;){HalSMArray_add(memory,out,harr->arr[i]);if (i==0) {break;}}
    return out;
}

void HalSMArray_appendArray(HalSMMemory* memory,HalSMArray* harr,HalSMArray* narr)
{
    harr->arr=HalSMMemory_realloc(memory,harr->arr,(harr->size+narr->size)*sizeof(HalSMVariable*));
    HalSMVariable** p=&harr->arr[harr->size];
    HalSMVariable** pn=&narr->arr[0];
    for (ui64 i=0;i<narr->size;i++){*p++=*pn++;}
    harr->size=harr->size+narr->size;
}

void HalSMArray_insert(HalSMMemory* memory,HalSMArray* harr,HalSMVariable* value,ui64 index)
{
    //Error
    if (index>harr->size) {return;}
    HalSMVariable** temp=HalSMMemory_malloc(memory,sizeof(HalSMVariable*)*(harr->size+1));
    ui64 i;
    for (i=0;i<index;i++) {
        temp[i]=harr->arr[i];
    }

    for (i=index;i<harr->size;i++) {
        temp[i+1]=harr->arr[i];
    }
    temp[index]=value;
    HalSMMemory_free(memory,harr->arr);
    harr->arr=temp;
    harr->size=harr->size+1;
}

HalSMVariable* HalSMArray_get(HalSMArray* harr,ui64 index)
{
    if (harr->size==0||index>=harr->size){HalSMVariable* temp=&null;return temp;}
    return harr->arr[index];
}

void AdditionStrings(HalSMMemory* memory,HalStringFormatChar** c,HalStringFormatChar* f,ui64 sizec,ui64 sizef)
{
    HalStringFormatChar* tmp=HalSMMemory_calloc(memory,sizec+1,sizeof(HalStringFormatChar));
    memory_cpy(tmp,*c,sizec*sizeof(HalStringFormatChar));
    *c=HalSMMemory_calloc(memory,sizec+sizef+1,sizeof(HalStringFormatChar));
    memory_cpy(*c,tmp,sizec*sizeof(HalStringFormatChar));
    memory_cpy(*c+string_len(*c),f,sizef*sizeof(HalStringFormatChar));
    HalSMMemory_free(memory,tmp);
}

HalStringFormatChar* HalSMArray_join_str(HalSMMemory* memory,HalSMArray* harr,HalStringFormatChar* join)
{
    if(harr->size==0) {
        return U8H("");
    } else if (harr->size==1) {
        return *(HalStringFormatChar**)harr->arr[0]->value;
    }
    ui64 size=0;
    ui64 lj=string_len(join);
    HalStringFormatChar* out=HalSMMemory_calloc(memory,1,sizeof(HalStringFormatChar));
    out[0]=ASCIIH('\0');
    ui64 i;
    size=0;
    HalSMVariable* o;
    for (i=0;i<harr->size;i++) {
        o=harr->arr[i];
        HalStringFormatChar* oc=*(HalStringFormatChar**)o->value;
        AdditionStrings(memory,&out,oc,size,string_len(oc));
        size+=string_len(oc);
        if (i!=harr->size-1) {
            AdditionStrings(memory,&out,join,size,lj);
            size+=lj;
        }
    }
    out[size]=ASCIIH('\0');
    return out;
}

HalStringFormatChar* HalSMArray_to_print(HalSMCompiler* hsmc,HalSMArray* harr)
{
    HalStringFormatChar* joinout=HalSMArray_join_str(hsmc->memory,HalSMCompiler_get_print_text(hsmc,harr,1),U8H(", "));
    ui64 l=string_len(joinout);
    HalStringFormatChar* outs=HalSMMemory_malloc(hsmc->memory,(3+l)*sizeof(HalStringFormatChar));
    string_cpy(outs,U8H("["));
    string_cat(outs,joinout);
    string_cat(outs,U8H("]"));
    outs[l+2]=ASCIIH('\0');
    return outs;
}

HalStringFormatChar* HalSMArray_chars_to_str(HalSMMemory* memory,HalSMArray* harr)
{
    HalStringFormatChar* out=HalSMMemory_malloc(memory,(harr->size+1)*sizeof(HalStringFormatChar));
    for (ui64 i=0;i<harr->size;i++) {out[i]=*(HalStringFormatChar*)harr->arr[i]->value;}
    out[harr->size]=ASCIIH('\0');
    return out;
}

HalSMArray* HalSMArray_slice(HalSMMemory* memory,HalSMArray* harr,ui64 s,ui64 e)
{
    if (s>=e){
        //Error
    }
    HalSMArray* out=HalSMMemory_malloc(memory,sizeof(HalSMArray));
    out->arr=HalSMMemory_malloc(memory,(MathMin(e,harr->size)-s)*sizeof(HalSMVariable*));
    out->size=MathMin(e,harr->size)-s;

    ui64 mm=MathMin(e,harr->size);

    for (ui64 i=s,d=0;i<mm;i++,d++) {out->arr[d]=harr->arr[i];}
    return out;
}

ui8 HalSMArray_compare(HalSMCompiler* hsmc,HalSMArray* harr,HalSMArray* barr)
{
    if (harr->size!=barr->size){return 0;}
    for (ui64 i=0;i<harr->size;i++) {
        if (HalSMVariable_Compare(hsmc,harr->arr[i],barr->arr[i])==0){return 0;}
    }
    return 1;
}

HalSMArray* HalSMArray_from_str(HalSMMemory* memory,HalStringFormatChar* str,ui64 size)
{
    HalSMArray* out=HalSMArray_init(memory);
    for (ui64 i=0;i<size;i++) {HalSMArray_add(memory,out,HalSMVariable_init_char(memory,str[i]));}
    return out;
}

HalSMArray* HalSMArray_copy(HalSMMemory* memory,HalSMArray* harr)
{
    HalSMArray* out=HalSMMemory_malloc(memory,sizeof(HalSMArray));
    out->size=harr->size;
    out->arr=HalSMMemory_calloc(memory,harr->size,sizeof(HalSMVariable*));
    memory_cpy(out->arr,harr->arr,harr->size*sizeof(HalSMVariable*));
    return out;
}

void HalSMArray_free(HalSMMemory* memory,HalSMArray* harr)
{
    for (ui64 i=0;i<harr->size;i++) {
        HalSMVariable_free(memory,harr->arr[i]);
    }
    HalSMMemory_free(memory,harr->arr);
    HalSMMemory_free(memory,harr);
}

i64 HalSMArray_find(HalSMCompiler* hsmc,HalSMArray* harr,HalSMVariable* value)
{
    HalSMVariable** p=harr->arr;
    for (ui64 i=0;i<harr->size;i++) {
        if (HalSMVariable_Compare(hsmc,*p++,value)){return i;}
    }
    return -1;
}

void HalSMArray_clear(HalSMMemory* memory,HalSMArray* harr)
{
    harr->arr=HalSMMemory_realloc(memory,harr->arr,0);
    harr->size=0;
}

HalSMArray* HalSMArray_init_with_elements(HalSMMemory* memory,HalSMVariable** arr,ui64 size)
{
    HalSMArray* out=HalSMMemory_malloc(memory,sizeof(HalSMArray));
    out->arr=HalSMMemory_malloc(memory,size*sizeof(HalSMVariable*));
    memory_cpy(out->arr,arr,size*sizeof(HalSMVariable*));
    out->size=size;
    return out;
}

HalSMFunctionC* HalSMFunctionC_init(HalSMCompiler* hsmc,HalSMFunctionCTypeDef func)
{
    HalSMFunctionC* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMFunctionC));
    out->func=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMFunctionCTypeDef));
    *out->func=func;
    out->type=0;
    out->data=NOTHALSMNULLPOINTER;
    out->hsmc=hsmc;
    return out;
}

HalSMFunctionC* HalSMFunctionC_initWithData(void* data,HalSMCompiler* hsmc,HalSMFunctionCTypeDefWithData func)
{
    HalSMFunctionC* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMFunctionC));
    out->func=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMFunctionCTypeDefWithData));
    *out->func=func;
    out->type=1;
    out->data=data;
    out->hsmc=hsmc;
    return out;
}

HalSMVariable* HalSMFunctionC_run(HalSMCompiler* hsmc,HalSMFunctionC* hfc,HalSMArray* args,Dict* vars) {
    if (hfc->type==1){return (*(HalSMFunctionCTypeDefWithData*)hfc->func)(hfc->data,hsmc,args,vars);}
    return (*(HalSMFunctionCTypeDef*)hfc->func)(hsmc,args,vars);
}

HalSMRunClassC* HalSMRunClassC_init(HalSMMemory* memory,HalStringFormatChar* name,Dict* vrs,Dict* funcs)
{
    HalSMRunClassC* runclassc=HalSMMemory_malloc(memory,sizeof(HalSMRunClassC));
    runclassc->name=name;
    runclassc->vrs=vrs;
    runclassc->funcs=funcs;
    return runclassc;
}

HalSMRunClassC* HalSMRunClassC__init__(HalSMCompiler* hsmc,HalSMRunClassC* runclassc,HalSMArray* args)
{
    i64 indexInit=DictElementIndexByKey(hsmc,runclassc->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__init__")));
    if (indexInit!=-1){
        HalSMFunctionC_run(hsmc,(HalSMFunctionC*)DictElementFindByIndex(runclassc->funcs,indexInit)->value->value,args,&NULLDict);
    }
    return runclassc;
}

HalSMClassC* HalSMClassC_init(HalSMMemory* memory,HalStringFormatChar* name)
{
    HalSMClassC* classc=HalSMMemory_malloc(memory,sizeof(HalSMClassC));
    classc->vrs=DictInit(memory);
    classc->funcs=DictInit(memory);
    classc->name=name;
    return classc;
}

HalSMRunClassC* HalSMClassC_run(HalSMCompiler* hsmc,HalSMClassC* classc,HalSMArray* args)
{
    HalSMRunClassC* o=HalSMRunClassC_init(hsmc->memory,classc->name,classc->vrs,classc->funcs);
    HalSMArray_insert(hsmc->memory,args,HalSMVariable_init(hsmc->memory,o,HalSMVariableType_HalSMRunClassC),0);
    return HalSMRunClassC__init__(hsmc,o,args);
}

HalSMClass* HalSMClass_init(HalSMMemory* memory,HalStringFormatChar* name,Dict* vrs)
{
    HalSMClass* out=HalSMMemory_malloc(memory,sizeof(HalSMClass));
    //out->vars=vrs;
    out->vars=DictInit(memory);
    out->funcs=DictInit(memory);
    out->name=name;
    return out;
}

HalSMRunClass* HalSMClass_run(HalSMClass* clas,HalSMCompiler* hsmc,HalSMArray* args)
{
    HalSMRunClass* out=HalSMRunClass_init(hsmc->memory,clas->name,clas->vars,clas->funcs);
    return HalSMRunClass__init__(out,hsmc,args);
}

HalSMRunClass* HalSMRunClass_init(HalSMMemory* memory,HalStringFormatChar* name,Dict* vrs,Dict* funcs)
{
    HalSMRunClass* out=HalSMMemory_malloc(memory,sizeof(HalSMRunClass));
    out->name=name;
    out->vars=DictCopy(memory,vrs);
    out->funcs=DictCopy(memory,funcs);
    return out;
}

HalSMRunClass* HalSMRunClass__init__(HalSMRunClass* runclass,HalSMCompiler* hsmc,HalSMArray* args)
{
    if (DictElementIndexByKey(hsmc,runclass->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__init__")))!=-1) {
        HalSMLocalFunction* func=(HalSMLocalFunction*)DictElementFindByKey(hsmc,runclass->funcs,HalSMVariable_init_str(hsmc->memory,U8H("__init__")))->value->value;
        HalSMArray_insert(hsmc->memory,args,HalSMVariable_init(hsmc->memory,runclass,HalSMVariableType_HalSMRunClass),0);
        HalSMLocalFunction_run(func,args,&NULLDict);
    }
    return runclass;
}

HalSMCModule* HalSMCModule_init(HalSMMemory* memory,HalStringFormatChar* name)
{
    HalSMCModule* out=HalSMMemory_malloc(memory,sizeof(HalSMCModule));
    out->lfuncs=DictInit(memory);
    out->vrs=DictInit(memory);
    out->classes=DictInit(memory);
    out->name=name;
    return out;
}

HalSMCModule* HalSMCModule_load(HalSMCompiler* hsmc,HalSMCModule_entry* entry)
{
    HalSMCModule* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMCModule));
    out->lfuncs=entry->lfuncs;
    out->vrs=entry->vrs;
    out->classes=entry->classes;
    out->name=entry->name;
    return out;
}

HalSMModule* HalSMModule_init(HalSMMemory* memory,HalStringFormatChar* name,HalSMCompiler* hsmcmodule)
{
    HalSMModule* o=HalSMMemory_malloc(memory,sizeof(HalSMModule));
    o->hsmc=hsmcmodule;
    o->name=name;
    return o;
}

HalSMRunFuncC* HalSMRunFuncC_init(HalSMMemory* memory,HalSMFunctionC* func,HalSMArray* args)
{
    HalSMRunFuncC* out=HalSMMemory_malloc(memory,sizeof(HalSMRunFuncC));
    out->func=func;
    out->args=args;
    out->pointer=HalSMArray_init(memory);
    return out;
}

HalSMRunFunc* HalSMRunFunc_init(HalSMMemory* memory,HalSMLocalFunction* func,HalSMArray* args)
{
    HalSMRunFunc* out=HalSMMemory_malloc(memory,sizeof(HalSMRunFunc));
    out->func=func;
    out->args=args;
    out->pointer=HalSMArray_init(memory);
    return out;
}

ui8 HalSMIsInt(HalSMMemory* memory,HalStringFormatChar* c)
{
    HalStringFormatChar b;
    ui64 l=string_len(c);
    for (ui64 i=0;i<l;i++)
    {
        b=c[i];
        if (HSFCharCompare(b,ASCIIH('-'))&&i==0){continue;}
        else if (HSFCharCompare(b,ASCIIH('0'))==0&&HSFCharCompare(b,ASCIIH('1'))==0&&HSFCharCompare(b,ASCIIH('2'))==0&&HSFCharCompare(b,ASCIIH('3'))==0&&HSFCharCompare(b,ASCIIH('4'))==0&&HSFCharCompare(b,ASCIIH('5'))==0&&HSFCharCompare(b,ASCIIH('6'))==0&&HSFCharCompare(b,ASCIIH('7'))==0&&HSFCharCompare(b,ASCIIH('8'))==0&&HSFCharCompare(b,ASCIIH('9'))==0){return 0;}
    }
    return 1;
}

ui8 HalSMIsDouble(HalSMMemory* memory,HalStringFormatChar* c)
{
    HalStringFormatChar b;
    i64 ct=0;
    ui64 l=string_len(c);

    for (ui64 i=0;i<l;i++)
    {
        b=c[i];
        if ((HSFCharCompare(b,ASCIIH('-'))&&i>0)||(HSFCharCompare(b,ASCIIH('0'))==0&&HSFCharCompare(b,ASCIIH('1'))==0&&HSFCharCompare(b,ASCIIH('2'))==0&&HSFCharCompare(b,ASCIIH('3'))==0&&HSFCharCompare(b,ASCIIH('4'))==0&&HSFCharCompare(b,ASCIIH('5'))==0&&HSFCharCompare(b,ASCIIH('6'))==0&&HSFCharCompare(b,ASCIIH('7'))==0&&HSFCharCompare(b,ASCIIH('8'))==0&&HSFCharCompare(b,ASCIIH('9'))==0&&HSFCharCompare(b,ASCIIH('.'))==0)){
        return 0;}
        else if(HSFCharCompare(b,ASCIIH('.')) && (i==0 || i==string_len(c)-1)){return 0;}
        else if(HSFCharCompare(b,ASCIIH('.')) && i>0 && i<string_len(c)){ct+=1;if(ct==2)return 0;}
    }
    if (ct==0){return 0;}
    return 1;
}

ui8 HalSMIsHex(HalSMMemory* memory,HalStringFormatChar* c)
{
    HalStringFormatChar b;
    ui64 l=string_len(c);
    if (l<3){return 0;}
    HalStringFormatChar* p=c;
    if (HSFCharCompare(*p,ASCIIH('-'))){p++;l--;}
    if (HSFCharCompare(p[0],ASCIIH('0'))==0||HSFCharCompare(p[1],ASCIIH('x'))==0){return 0;}
    p+=2;
    l-=2;

    for (ui64 i=0;i<l;i++,p++)
    {
        b=*p;
        if (HSFCharCompare(b,ASCIIH('0'))==0&&HSFCharCompare(b,ASCIIH('1'))==0&&HSFCharCompare(b,ASCIIH('2'))==0&&HSFCharCompare(b,ASCIIH('3'))==0&&HSFCharCompare(b,ASCIIH('4'))==0&&HSFCharCompare(b,ASCIIH('5'))==0&&HSFCharCompare(b,ASCIIH('6'))==0&&HSFCharCompare(b,ASCIIH('7'))==0&&HSFCharCompare(b,ASCIIH('8'))==0&&HSFCharCompare(b,ASCIIH('9'))==0&&HSFCharCompare(b,ASCIIH('a'))==0&&HSFCharCompare(b,ASCIIH('b'))==0&&HSFCharCompare(b,ASCIIH('c'))==0&&HSFCharCompare(b,ASCIIH('d'))==0&&HSFCharCompare(b,ASCIIH('e'))==0&&HSFCharCompare(b,ASCIIH('f'))==0&&HSFCharCompare(b,ASCIIH('A'))==0&&HSFCharCompare(b,ASCIIH('B'))==0&&HSFCharCompare(b,ASCIIH('C'))==0&&HSFCharCompare(b,ASCIIH('D'))==0&&HSFCharCompare(b,ASCIIH('E'))==0&&HSFCharCompare(b,ASCIIH('F'))==0){return 0;}
    }
    return 1;
}

ui8 HalSMIsBin(HalSMMemory* memory,HalStringFormatChar* c)
{
    HalStringFormatChar b;
    ui64 l=string_len(c);
    if (l<3){return 0;}
    HalStringFormatChar* p=c;
    if (HSFCharCompare(*p,ASCIIH('-'))){p++;l--;}
    if (HSFCharCompare(p[0],ASCIIH('0'))==0||HSFCharCompare(p[1],ASCIIH('b'))==0){return 0;}
    p+=2;
    l-=2;

    for (ui64 i=0;i<l;i++,p++)
    {
        b=*p;
        if (HSFCharCompare(b,ASCIIH('0'))==0&&HSFCharCompare(b,ASCIIH('1'))==0){return 0;}
    }
    return 1;
}

HalSMVariable* ParseHalSMVariableInt(HalSMCompiler* hsmc,HalStringFormatChar* c)
{
    return HalSMVariable_init(hsmc->memory,HalSMInteger_parse(hsmc->memory,c),HalSMVariableType_int);
}

HalSMVariable* ParseHalSMVariableDouble(HalSMCompiler* hsmc,HalStringFormatChar* c)
{
    return HalSMVariable_init(hsmc->memory,HalSMDouble_parse(hsmc->memory,c),HalSMVariableType_double);
}

HalSMVariable* HalSMLocalFunction_run(HalSMLocalFunction* lf,HalSMArray* args,Dict* vars)
{
    ui64 ia=0;
    HalSMVariable* v;
    Dict* vrs;
    HalSMCompiler* hsmc=lf->hsmc;
    HalSMErrorIndificator* ind=HalSMErrorIndificator_init(hsmc,lf->name,0,hsmc->curInd);
    hsmc->curInd=ind;
    if (vars->size>0) {
        vrs=DictInitWithElements(hsmc->memory,vars->elements,vars->size);
    } else {
        vrs=DictInit(hsmc->memory);
    }
    DictAppendDict(hsmc->memory,vrs,lf->vars);

    if (lf->module->type==HalSMVariableType_HalSMModule) {
        hsmc=((HalSMModule*)lf->module->value)->hsmc;
    }

    for (ui64 arg=0;arg<args->size&&hsmc->isClose==0;arg++)
    {
        v=args->arr[arg];
        if (v->type==HalSMVariableType_HalSMSetArg)
        {
            PutDictElementToDict(hsmc,vrs,DictElementInit(hsmc->memory,((HalSMSetArg*)v->value)->name,((HalSMSetArg*)v->value)->value));
            ia++;
            continue;
        }
        if (lf->args->size<=arg-ia) {
            HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Arguments more than parameters")));
            return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Arguments more than parameters")),HalSMVariableType_HalSMError);
        }
        if (lf->args->arr[arg-ia]->type==HalSMVariableType_HalSMVar) {
            PutDictElementToDict(hsmc,vrs,DictElementInit(hsmc->memory,((HalSMVar*)lf->args->arr[arg-ia]->value)->name,v));
        } else if (lf->args->arr[arg-ia]->type==HalSMVariableType_HalSMSetArg) {
            PutDictElementToDict(hsmc,vrs,DictElementInit(hsmc->memory,((HalSMSetArg*)lf->args->arr[arg-ia]->value)->name,v));
        } else {
            return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Unknown Type Argument")),HalSMVariableType_HalSMError);
        }
    }
    HalSMVariable* ad;
    HalSMSetArg* argi;
    HalSMVar* argv;
    ui64 a;
    for (a=0;a<lf->args->size;a++)
    {
        if (lf->args->arr[a]->type==HalSMVariableType_HalSMSetArg) {
            argi=(HalSMSetArg*)lf->args->arr[a]->value;
            ad=argi->name;
            if (DictElementIndexByKey(hsmc,vrs,ad)!=-1){continue;}
            /*HalSMArray temparrvar={1,(HalSMVariable*[]){argi->value}};
            HalSMVariable* tempvarp=HalSMVariable_init(hsmc->memory,&hnull,HalSMVariableType_HalSMNull);
            HalSMCompiler_getArgsSetVar(hsmc,&temparrvar,tempvarp,vrs);*/
            ui8 isOut=0;
            PutDictElementToDict(hsmc,vrs,DictElementInit(hsmc->memory,ad,HalSMCompiler_isGetU(hsmc,*(HalStringFormatChar**)argi->value->value,&isOut,0,vars)));
            if (isOut==0) {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Wrong set arg value")));
                return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Wrong set arg value")),HalSMVariableType_HalSMError);
            }
        } else if (lf->args->arr[a]->type==HalSMVariableType_HalSMVar) {
            argv=(HalSMVar*)lf->args->arr[a]->value;
            if (DictElementIndexByKey(hsmc,vrs,argv->name)!=-1) {
                continue;
            } else {
                HalSMError* err=HalSMError_init(hsmc,hsmc->curInd,U8H("Not enough args"));
                return HalSMVariable_init(hsmc->memory,err,HalSMVariableType_HalSMError);
            }
        }
    }

    /*if (lf->module->type!=HalSMVariableType_HalSMNull) {
        HalSMModule* module=(HalSMModule*)lf->module->value;
        for (a=0;a<module->hsmc->classes->size;a++) {
            PutDictElementToDict(hsmc,vrs,module->hsmc->classes->elements[a]);
        }

        for (a=0;a<module->hsmc->localFunctions->size;a++) {
            PutDictElementToDict(hsmc,vrs,module->hsmc->localFunctions->elements[a]);
        }

        for (a=0;a<module->hsmc->variables->size;a++) {
            PutDictElementToDict(hsmc,vrs,module->hsmc->variables->elements[a]);
        }
    }*/

    HalSMArray* arguss,*retargs;
    HalSMVariable* arg;
    HalSMVariable* err;
    HalSMVariable* ifr=&null;
    HalSMVariable* f;
    HalSMFunctionArray* resRunFunc=NOTHALSMNULLPOINTER;
    HalSMVariable* isGOTO=&null;
    HalSMGOTOSector* tempSector;
    ui8 isGet;
    for (ui64 fi=0;fi<lf->func->size;fi++)
    {
        f=lf->func->arr[fi];
        if (f->type==HalSMVariableType_HalSMGOTOSector) {
            if (isGOTO->type==HalSMVariableType_HalSMGOTO&&HSFCompare(((HalSMGOTOSector*)f->value)->sector,((HalSMGOTO*)isGOTO->value)->sector)) {
                isGOTO=&null;
            }
            tempSector=(HalSMGOTOSector*)f->value;
            tempSector->index=fi;
            tempSector->lf=lf;
            PutDictElementToDict(hsmc,vrs,DictElementInit(hsmc->memory,HalSMVariable_init_str(hsmc->memory,tempSector->sector),f));
        } else if (isGOTO->type!=HalSMVariableType_HalSMNull) {

        } else if (f->type==HalSMVariableType_HalSMRunFuncC) {
            arguss=((HalSMRunFuncC*)f->value)->args;
            HalSMCompiler_getArgsFunction(hsmc,arguss,((HalSMRunFuncC*)f->value)->pointer,vrs);
            err=HalSMFunctionC_run(hsmc,((HalSMRunFuncC*)f->value)->func,((HalSMRunFuncC*)f->value)->pointer,vrs);
            if(err->type==HalSMVariableType_HalSMError){return err;}
            else if(err->type==HalSMVariableType_HalSMGOTOSector) {
                tempSector=(HalSMGOTOSector*)err->value;
                if (tempSector->lf!=lf){return err;}
                fi=tempSector->index;
            } else if (err->type==HalSMVariableType_HalSMBreak){return err;}
            else if(err->type==HalSMVariableType_HalSMReturn) {
                if (((HalSMReturn*)err->value)->idLocalFunction==lf->id) {
                    retargs=HalSMCompiler_getArgs(hsmc,((HalSMReturn*)err->value)->value,vrs);
                    if (retargs->size==1) {
                        return retargs->arr[0];
                    }
                    return HalSMVariable_init(hsmc->memory,retargs,HalSMVariableType_HalSMArray);
                }
                return err;
            }
            if (ifr->type!=HalSMVariableType_HalSMNull) ifr=&null;
        } else if(f->type==HalSMVariableType_HalSMRunFunc) {
            arguss=((HalSMRunFunc*)f->value)->args;
            HalSMCompiler_getArgsFunction(hsmc,arguss,((HalSMRunFunc*)f->value)->pointer,vrs);
            err=HalSMLocalFunction_run(((HalSMRunFunc*)f->value)->func,((HalSMRunFunc*)f->value)->pointer,vrs);
            if(err->type==HalSMVariableType_HalSMError){return err;}
            else if(err->type==HalSMVariableType_HalSMGOTOSector) {
                tempSector=(HalSMGOTOSector*)err->value;
                if (tempSector->lf!=lf){return err;}
                fi=tempSector->index;
            } else if (err->type==HalSMVariableType_HalSMBreak){return err;}
            else if(err->type==HalSMVariableType_HalSMReturn) {
                if (((HalSMReturn*)err->value)->idLocalFunction==lf->id) {
                    retargs=HalSMCompiler_getArgs(hsmc,((HalSMReturn*)err->value)->value,vrs);
                    if (retargs->size==1) {
                        return retargs->arr[0];
                    }
                    return HalSMVariable_init(hsmc->memory,retargs,HalSMVariableType_HalSMArray);
                }
                return err;
            }
            if (ifr->type!=HalSMVariableType_HalSMNull) ifr=&null;
        } else if (f->type==HalSMVariableType_HalSMLocalFunction) {
            err=HalSMLocalFunction_run(((HalSMLocalFunction*)f->value),HalSMArray_init_with_elements(hsmc->memory,(HalSMVariable*[]){HalSMVariable_init(hsmc->memory,vrs,HalSMVariableType_HalSMDict)},1),vrs);
            if(err->type==HalSMVariableType_HalSMError){return err;}
            else if(err->type==HalSMVariableType_HalSMGOTOSector) {
                tempSector=(HalSMGOTOSector*)err->value;
                if (tempSector->lf!=lf){return err;}
                fi=tempSector->index;
            } else if (err->type==HalSMVariableType_HalSMBreak){return err;}
            else if(err->type==HalSMVariableType_HalSMReturn) {
                if (((HalSMReturn*)err->value)->idLocalFunction==lf->id) {
                    retargs=HalSMCompiler_getArgs(hsmc,((HalSMReturn*)err->value)->value,vrs);
                    if (retargs->size==1) {
                        return retargs->arr[0];
                    }
                    return HalSMVariable_init(hsmc->memory,retargs,HalSMVariableType_HalSMArray);
                }
                return err;
            }
            if (ifr->type!=HalSMVariableType_HalSMNull) ifr=&null;
        } else if (f->type==HalSMVariableType_HalSMSetVar) {
            //HalSMCompiler_getArgsSetVar(hsmc,((HalSMSetVar*)f->value)->value,((HalSMSetVar*)f->value)->pointer,vrs);
            ((HalSMSetVar*)f->value)->pointer=HalSMCompiler_getArgs(hsmc,((HalSMSetVar*)f->value)->value,vrs)->arr[0];
            arg=((HalSMSetVar*)f->value)->pointer;
            HalStringFormatChar* nameVar=*(HalStringFormatChar**)((HalSMSetVar*)f->value)->name->value;
            HalSMVariable* isGetV=HalSMCompiler_isGetU(hsmc,nameVar,&isGet,1,vrs);
            if (isGet) {
                isGetV->value=arg->value;
                isGetV->type=arg->type;
            }
            if (ifr->type!=HalSMVariableType_HalSMNull) ifr=&null;
        } else if (f->type==HalSMVariableType_HalSMFunctionArray) {
            resRunFunc=(HalSMFunctionArray*)f->value;
            if (resRunFunc->args->arr[0]->type==HalSMVariableType_HalSMVar) {
                ui8 isOut=0;
                err=HalSMCompiler_isGetU(hsmc,*(HalStringFormatChar**)((HalSMVar*)resRunFunc->args->arr[0]->value)->name->value,&isOut,0,vrs);
                if (isOut) {
                    if (err->type==HalSMVariableType_HalSMError){return err;}
                    else if(err->type==HalSMVariableType_HalSMGOTOSector) {
                        tempSector=(HalSMGOTOSector*)err->value;
                        if (tempSector->lf!=lf){return err;}
                        fi=tempSector->index;
                    } else if (err->type==HalSMVariableType_HalSMBreak){return err;}
                    else if(err->type==HalSMVariableType_HalSMReturn) {
                        if (((HalSMReturn*)err->value)->idLocalFunction==lf->id) {
                            retargs=HalSMCompiler_getArgs(hsmc,((HalSMReturn*)err->value)->value,vrs);
                            if (retargs->size==1) {
                                return retargs->arr[0];
                            }
                            return HalSMVariable_init(hsmc->memory,retargs,HalSMVariableType_HalSMArray);
                        }
                        return err;
                    }
                } else {
                    return HalSMVariable_init(hsmc->memory,HalSMError_init(hsmc,hsmc->curInd,U8H("Variable Not Found")),HalSMVariableType_HalSMError);
                }
            }
        } else if (f->type==HalSMVariableType_HalSMGOTO) {
            HalSMGOTO* gt=(HalSMGOTO*)f->value;
            err=HalSMVariable_init_str(hsmc->memory,gt->sector);
            HalSMVariable* ret;
            if (DictElementIndexByKey(hsmc,vrs,err)!=-1&&(ret=DictElementFindByKey(hsmc,vrs,err)->value)->type==HalSMVariableType_HalSMGOTOSector) {
                tempSector=(HalSMGOTOSector*)ret->value;
                if (tempSector->lf!=lf){return ret;}
                fi=tempSector->index;
            } else if (DictElementIndexByKey(hsmc,hsmc->GOTOSectors,err)!=-1) {
                //GOTO Function
                return DictElementFindByKey(hsmc,hsmc->GOTOSectors,err)->value;
            } else {
                isGOTO=f;
            }
        } else if (f->type==HalSMVariableType_HalSMBreak) {
            return f;
        } else if (f->type==HalSMVariableType_HalSMReturn) {
            if (((HalSMReturn*)f->value)->idLocalFunction==lf->id){
                retargs=HalSMCompiler_getArgs(hsmc,((HalSMReturn*)f->value)->value,vrs);
                if (retargs->size==1) {
                    return retargs->arr[0];
                }
                return HalSMVariable_init(hsmc->memory,retargs,HalSMVariableType_HalSMArray);
            }
            return f;
        }
        ind->line++;
        hsmc->curInd=ind;
    }
    END_LOCAL_FUNCTION:
    return &hvoid;
}

HalSMArray* HalSMLocalFunction_getArgs(HalSMCompiler* hsmc,HalStringFormatChar* l)
{
    HalStringFormatChar i;
    HalSMArray* n=HalSMArray_init(hsmc->memory);
    ui64 ll=string_len(l);

    ui64 isA=0,isF=0,isD=0;
    HalStringFormatChar isS=ASCIIH('n');
    HalSMArray* out=HalSMArray_init(hsmc->memory);
    HalStringFormatChar* isSetArg=NOTHALSMNULLPOINTER;

    for (ui64 ind=0;ind<ll;ind++) {
        i=l[ind];
        
        if (isA) {
            if (HSFCharCompare(i,ASCIIH('['))) {
                isA++;
            } else if (HSFCharCompare(i,ASCIIH(']'))) {
                isA--;
            }
        } else if (isF) {
            if (HSFCharCompare(i,ASCIIH('('))) {
                isF++;
            } else if (HSFCharCompare(i,ASCIIH(')'))) {
                isF--;
            }

        } else if (HSFCharCompare(isS,ASCIIH('n'))==0) {
            if (HSFCharCompare(i,isS)){isS=ASCIIH('n');}
        } else if (isD) {
            if (HSFCharCompare(i,ASCIIH('{'))) {
                isD++;
            } else if (HSFCharCompare(i,ASCIIH('}'))) {
                isD--;
            }
        } else if (HSFCharCompare(i,ASCIIH('\''))||HSFCharCompare(i,ASCIIH('"'))) {
            isS=i;
        } else if (HSFCharCompare(i,ASCIIH('['))) {
            isA++;
        } else if (HSFCharCompare(i,ASCIIH('{'))) {
            isD++;
        } else if (HSFCharCompare(i,ASCIIH('('))) {
            isF++;
        } else if (HSFCharCompare(i,ASCIIH('='))&&n->size>0) {
            if (HSFCharCompare(l[ind-1],ASCIIH('+'))==0&&HSFCharCompare(l[ind-1],ASCIIH('-'))==0&&HSFCharCompare(l[ind-1],ASCIIH('*'))==0&&HSFCharCompare(l[ind-1],ASCIIH('/'))==0) {
                isSetArg=HalSMArray_chars_to_str(hsmc->memory,n);
                HalSMArray_clear(hsmc->memory,n);
                continue;
            }
        } else if (HSFCharCompare(i,ASCIIH(','))) {
            if (n->size==0) {
                HalSMCompiler_ThrowError(hsmc,HalSMError_init(hsmc,hsmc->curInd,U8H("Wrong parameters in local function")));
                return out;
            }
            if (isSetArg==NOTHALSMNULLPOINTER) {
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMVar_init(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,n)),HalSMVariableType_HalSMVar));
            } else {
                HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMSetArg_initWithValue(hsmc->memory,HalSMVariable_init_str(hsmc->memory,isSetArg),HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,n))),HalSMVariableType_HalSMSetArg));
                isSetArg=NOTHALSMNULLPOINTER;
            }
            HalSMArray_clear(hsmc->memory,n);
            continue;
        } else if (HSFCharCompare(i, ASCIIH(' '))) {
            continue;
        }
        HalSMArray_add(hsmc->memory,n,HalSMVariable_init_char(hsmc->memory,i));
    }
    if (n->size>0) {
        if (isSetArg==NOTHALSMNULLPOINTER) {
            HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMVar_init(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,n)),HalSMVariableType_HalSMVar));
        } else {
            HalSMArray_add(hsmc->memory,out,HalSMVariable_init(hsmc->memory,HalSMSetArg_initWithValue(hsmc->memory,HalSMVariable_init_str(hsmc->memory,isSetArg),HalSMVariable_init_str(hsmc->memory,HalSMArray_chars_to_str(hsmc->memory,n))),HalSMVariableType_HalSMSetArg));
            isSetArg=NOTHALSMNULLPOINTER;
        }
        HalSMArray_clear(hsmc->memory,n);
    }
    return out;
}

HalSMLocalFunction* HalSMLocalFunction_init(HalSMCompiler* hsmc,HalStringFormatChar* name,HalStringFormatChar* args,HalSMVariable* module)
{
    HalSMLocalFunction* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMLocalFunction));
    out->hsmc=hsmc;
    out->name=name;
    out->id=hsmc->countLocalFunctions++;
    out->args=HalSMLocalFunction_getArgs(hsmc,args);
    out->func=HalSMArray_init(hsmc->memory);
    out->vars=DictInit(hsmc->memory);
    out->module=module;
    return out;
}

i64 StringIndexOf(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f) {
    ui64 fl=string_len(f);
    ui64 cl=string_len(c);
    if (fl==0||cl==0||cl<fl){return -1;}
    HalStringFormatChar* fa=HalSMMemory_malloc(memory,(fl+1)*sizeof(HalStringFormatChar));
    fa[fl]=ASCIIH('\0');
    ui64 d=cl-fl+1;
    for (ui64 i=0;i<d;i++) {
        memory_cpy(fa,&c[i],fl*sizeof(HalStringFormatChar));
        if (HSFCompare(fa,f)){return i;}
    }
    return -1;
}

i64 StringLastIndexOf(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f) {
    ui64 fl=string_len(f);
    ui64 cl=string_len(c);
    if (fl==0||cl==0||fl>=cl){return -1;}
    i64 out=-1;
    HalStringFormatChar* fa=HalSMMemory_malloc(memory,(fl+1)*sizeof(HalStringFormatChar));
    fa[fl]=ASCIIH('\0');
    ui64 d=cl-fl+1;
    for (ui64 i=0;i<d;i++) {
        memory_cpy(fa,&c[i],fl*sizeof(HalStringFormatChar));
        if (HSFCompare(fa,f)){out=i;}
    }
    return out;
}

ui8 StringEndsWith(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f)
{
    if (string_len(c)==0||string_len(f)==0) {
        return 0;
    } else if (string_len(c)==string_len(f)) {
        return HSFCompare(c,f);
    } else if (string_len(c)<string_len(f)) {
        return 0;
    }
    return HSFCompare(SubString(memory,c,string_len(c)-string_len(f),string_len(c)),f);
}

ui8 StringStartsWith(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f)
{
    if (string_len(c)==0||string_len(f)==0) {
        return 0;
    } else if (string_len(c)==string_len(f)) {
        return HSFCompare(c,f);
    } else if (string_len(c)<string_len(f)) {
        return 0;
    }
    return HSFCompare(SubString(memory,c,0,string_len(f)),f);
}

HalStringFormatChar* SubString(HalSMMemory* memory,HalStringFormatChar* c,int start,int end) {
    i64 cl=string_len(c);
    if (start>=cl){
        //PRINT ERROR
        return U8H("");
    }
    else if (cl==0||end>cl){return U8H("");}
    if (end<0){end=cl+end;}
    if (start<0){start=cl+start;}
    if (end<0){return U8H("");}
    if (start<0){return U8H("");}
    ui64 i,lo;
    lo=MathMin(end,cl)-start;
    HalStringFormatChar* out=HalSMMemory_malloc(memory,(lo+1)*sizeof(HalStringFormatChar));
    //memory_cpy(out,&c[start],lo*sizeof(HalStringFormatChar));
    HalStringFormatChar* cp=&c[start];
    HalStringFormatChar* outp=out;
    for (i=0;i<lo;i++) {
        *outp++=*cp++;
    }
    out[lo]=ASCIIH('\0');
    return out;
}

HalStringFormatChar* SubStringHalSMInteger(HalSMMemory* memory,HalStringFormatChar* c,HalSMInteger* start,HalSMInteger* end) {
    HalSMInteger* cl=string_len_HalSMInteger(memory,c);
    if (HalSMInteger_isMore(start,cl)||HalSMInteger_isEqual(start,cl)) {
        return U8H("");
    } else if (HalSMInteger_isEqual(cl,&HalSMIntegerZero)||HalSMInteger_isMore(end,cl)){return U8H("");}
    ui64 lo=HalSMInteger_ToUnsignedLongLongInteger(HalSMInteger_Sub(memory,HalSMInteger_isLess(end,cl)?end:cl,start));
    HalStringFormatChar* out=HalSMMemory_malloc(memory,(lo+1)*sizeof(HalStringFormatChar));
    memory_cpy(out,&c[HalSMInteger_ToUnsignedInteger(start)],lo*sizeof(HalStringFormatChar));
    out[lo]=ASCIIH('\0');
    return out;
}

HalStringFormatChar* ConcatenateStrings(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f) {
    if (string_len(c)==0&&string_len(f)==0){return U8H("");}
    HalStringFormatChar* out=HalSMMemory_malloc(memory,(string_len(c)+string_len(f)+1)*sizeof(HalStringFormatChar));
    string_cpy(out,c);
    string_cat(out,f);
    out[string_len(c)+string_len(f)]=ASCIIH('\0');
    return out;
}

HalStringFormatChar* StringReplace(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f,HalStringFormatChar* r) {
    ui64 lf=string_len(f);
    ui64 lr=string_len(r);
    ui64 cl=string_len(c);
    if(cl<lf||lf==0){return c;}
    else if(cl==0){return U8H("");}
    else if(c==f){return r;}
    HalStringFormatChar* out=HalSMMemory_malloc(memory,0);
    ui64 i=0;
    ui64 size=0;
    ui64 d=cl-lf+1;
    while(i<d){
        if(HSFCompare(SubString(memory,c,i,i+lf),f)){
            if(lr>0){
                out=HalSMMemory_realloc(memory,out,(size+lr)*sizeof(HalStringFormatChar));
                for (ui64 j=0;j<lr;j++)
                    out[size+j]=r[j];
                size+=lr;
            }
            i+=lf;
        } else {
            out=HalSMMemory_realloc(memory,out,(size+1)*sizeof(HalStringFormatChar));
            out[size]=c[i];
            size++;
            i++;
        }
    }
    out=HalSMMemory_realloc(memory,out,(size+1)*sizeof(HalStringFormatChar));
    out[size]=ASCIIH('\0');
    return out;
}

HalSMCalculateVars HalSMCalculateVars_init()
{
    HalSMCalculateVars out;
    out.version=U8H("0.0.1");
    out.addStr=HalSMCalculateVars_addStr;
    out.subStr=HalSMCalculateVars_subStr;
    out.mulStr=HalSMCalculateVars_mulStr;
    out.divStr=HalSMCalculateVars_divStr;
    out.addInt=HalSMCalculateVars_addInt;
    out.subInt=HalSMCalculateVars_subInt;
    out.mulInt=HalSMCalculateVars_mulInt;
    out.divInt=HalSMCalculateVars_divInt;
    out.addDouble=HalSMCalculateVars_addDouble;
    out.subDouble=HalSMCalculateVars_subDouble;
    out.mulDouble=HalSMCalculateVars_mulDouble;
    out.divDouble=HalSMCalculateVars_divDouble;
    return out;
}

HalStringFormatChar* HalSMCalculateVars_addStr(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1)
{
    return ConcatenateStrings(hsmc->memory,HalSMVariable_to_str(hsmc,v0,0),HalSMVariable_to_str(hsmc,v1,0));
}

HalStringFormatChar* HalSMCalculateVars_subStr(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1)
{
    HalStringFormatChar* v0s=HalSMVariable_to_str(hsmc,v0,0);
    HalStringFormatChar* v1s=HalSMVariable_to_str(hsmc,v1,0);
    if (string_len(v0s)<string_len(v1s)){return v0s;};
    i64 v=StringLastIndexOf(hsmc->memory,v0s,v1s);
    if (v==-1){return v0s;}
    return ConcatenateStrings(hsmc->memory,SubString(hsmc->memory,v0s,0,v),SubString(hsmc->memory,v0s,v+string_len(v1s),string_len(v0s)));
}

HalStringFormatChar* HalSMCalculateVars_mulStr(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1)
{
    if (v0->type==HalSMVariableType_int) {
        HalSMVariable* temp=v0;
        v0=v1;
        v1=temp;
    }
    if (v1->type!=HalSMVariableType_int||v0->type!=HalSMVariableType_str){return U8H("");}
    HalStringFormatChar* st=*(HalStringFormatChar**)v0->value;
    HalSMInteger* ch=(HalSMInteger*)v1->value;
    if (HalSMInteger_isEqual(ch,&HalSMIntegerZero)){return U8H("");}
    else if (HalSMInteger_isEqual(ch,&HalSMIntegerOne)){return st;}
    ui64 lst=string_len(st);
    ui64 l=HalSMInteger_ToUnsignedLongLongInteger(ch)*lst,i;
    ui64 size=lst*l;
    HalStringFormatChar* out=HalSMMemory_malloc(memory,(size+1)*sizeof(HalStringFormatChar));
    //out[size]=ASCIIH('\0');
    HalStringFormatChar* p=out;
    for (i=0;i<l;i++,p++) {
        *p=st[i%lst];
        //out[size]=ASCIIH('\0');
    }
    *p=ASCIIH('\0');
    return out;
}

HalStringFormatChar* HalSMCalculateVars_divStr(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1)
{
    if (v1->type==HalSMVariableType_str){return StringReplace(hsmc->memory,*(HalStringFormatChar**)v0->value,*(HalStringFormatChar**)v1->value,U8H(""));}
    HalStringFormatChar* s0=HalSMVariable_to_str(hsmc,v0,0);
    HalSMInteger* i1=(HalSMInteger*)v1->value;
    ui64 ls0=string_len(s0);
    if(ls0==0||HalSMInteger_isEqual(i1,&HalSMIntegerZero)){return U8H("");}
    else if(HalSMInteger_isEqual(i1,&HalSMIntegerOne)){return s0;}
    HalSMInteger remainder;
    HalSMInteger* louti=HalSMInteger_Div(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,string_len(s0)),i1,&remainder);
    ui64 lout=HalSMInteger_ToUnsignedLongLongInteger(louti);
    HalStringFormatChar* out=HalSMMemory_malloc(hsmc->memory,(lout+1)*sizeof(HalStringFormatChar));
    for (ui64 i=0;i<lout;i++) {
        out[i]=s0[i];
    }
    out[lout]=ASCIIH('\0');
    return out;
}

HalSMInteger* HalSMCalculateVars_addInt(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1)
{
    HalSMInteger* i0;
    if (v0->type==HalSMVariableType_str) {
        i0=HalSMInteger_parse(memory,*(HalStringFormatChar**)v0->value);
    } else {
        i0=(HalSMInteger*)v0->value;
    }

    HalSMInteger* i1;
    if (v1->type==HalSMVariableType_str) {
        i1=HalSMInteger_parse(memory,*(HalStringFormatChar**)v1->value);
    } else {
        i1=(HalSMInteger*)v1->value;
    }
    return HalSMInteger_Add(memory,i0,i1);
}

HalSMInteger* HalSMCalculateVars_subInt(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1)
{
    HalSMInteger* i0;
    if (v0->type==HalSMVariableType_str) {
        i0=HalSMInteger_parse(memory,*(HalStringFormatChar**)v0->value);
    } else {
        i0=(HalSMInteger*)v0->value;
    }

    HalSMInteger* i1;
    if (v1->type==HalSMVariableType_str) {
        i1=HalSMInteger_parse(memory,*(HalStringFormatChar**)v1->value);
    } else {
        i1=(HalSMInteger*)v1->value;
    }
    return HalSMInteger_Sub(memory,i0,i1);
}

HalSMInteger* HalSMCalculateVars_mulInt(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1)
{
    HalSMInteger* i0;
    if (v0->type==HalSMVariableType_str) {
        i0=HalSMInteger_parse(memory,*(HalStringFormatChar**)v0->value);
    } else {
        i0=(HalSMInteger*)v0->value;
    }

    HalSMInteger* i1;
    if (v1->type==HalSMVariableType_str) {
        i1=HalSMInteger_parse(memory,*(HalStringFormatChar**)v1->value);
    } else {
        i1=(HalSMInteger*)v1->value;
    }
    return HalSMInteger_Mul(memory,i0,i1);
}

HalSMInteger* HalSMCalculateVars_divInt(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1)
{
    HalSMInteger* i0;
    if (v0->type==HalSMVariableType_str) {
        i0=HalSMInteger_parse(memory,*(HalStringFormatChar**)v0->value);
    } else {
        i0=(HalSMInteger*)v0->value;
    }

    HalSMInteger* i1;
    if (v1->type==HalSMVariableType_str) {
        i1=HalSMInteger_parse(memory,*(HalStringFormatChar**)v1->value);
    } else {
        i1=(HalSMInteger*)v1->value;
    }
    HalSMInteger remainder;
    return HalSMInteger_Div(memory,i0,i1,&remainder);
}

HalSMDouble* HalSMCalculateVars_addDouble(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1)
{
    HalSMDouble* f0;
    if (v0->type==HalSMVariableType_str) {
        f0=HalSMDouble_parse(memory,*(HalStringFormatChar**)v0->value);
    } else if (v0->type==HalSMVariableType_int) {
        f0=HalSMDouble_FromHalSMInteger(memory,(HalSMInteger*)v0->value);
    } else {
        f0=(HalSMDouble*)v0->value;
    }

    HalSMDouble* f1;
    if (v1->type==HalSMVariableType_str) {
        f1=HalSMDouble_parse(memory,*(HalStringFormatChar**)v1->value);
    } else if (v1->type==HalSMVariableType_int) {
        f1=HalSMDouble_FromHalSMInteger(memory,(HalSMInteger*)v1->value);
    } else {
        f1=(HalSMDouble*)v1->value;
    }
    return HalSMDouble_Add(memory,f0,f1);
}

HalSMDouble* HalSMCalculateVars_subDouble(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1)
{
    HalSMDouble* f0;
    if (v0->type==HalSMVariableType_str) {
        f0=HalSMDouble_parse(memory,*(HalStringFormatChar**)v0->value);
    } else if (v0->type==HalSMVariableType_int) {
        f0=HalSMDouble_FromHalSMInteger(memory,(HalSMInteger*)v0->value);
    } else {
        f0=(HalSMDouble*)v0->value;
    }

    HalSMDouble* f1;
    if (v1->type==HalSMVariableType_str) {
        f1=HalSMDouble_parse(memory,*(HalStringFormatChar**)v1->value);
    } else if (v1->type==HalSMVariableType_int) {
        f1=HalSMDouble_FromHalSMInteger(memory,(HalSMInteger*)v1->value);
    } else {
        f1=(HalSMDouble*)v1->value;
    }
    return HalSMDouble_Sub(memory,f0,f1);
}
HalSMDouble* HalSMCalculateVars_mulDouble(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1)
{
    HalSMDouble* f0;
    if (v0->type==HalSMVariableType_str) {
        f0=HalSMDouble_parse(memory,*(HalStringFormatChar**)v0->value);
    } else if (v0->type==HalSMVariableType_int) {
        f0=HalSMDouble_FromHalSMInteger(memory,(HalSMInteger*)v0->value);
    } else {
        f0=(HalSMDouble*)v0->value;
    }

    HalSMDouble* f1;
    if (v1->type==HalSMVariableType_str) {
        f1=HalSMDouble_parse(memory,*(HalStringFormatChar**)v1->value);
    } else if (v1->type==HalSMVariableType_int) {
        f1=HalSMDouble_FromHalSMInteger(memory,(HalSMInteger*)v1->value);
    } else {
        f1=(HalSMDouble*)v1->value;
    }
    return HalSMDouble_Mul(memory,f0,f1);
}
HalSMDouble* HalSMCalculateVars_divDouble(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1)
{
    HalSMDouble* f0;
    if (v0->type==HalSMVariableType_str) {
        f0=HalSMDouble_parse(memory,*(HalStringFormatChar**)v0->value);
    } else if (v0->type==HalSMVariableType_int) {
        f0=HalSMDouble_FromHalSMInteger(memory,(HalSMInteger*)v0->value);
    } else {
        f0=(HalSMDouble*)v0->value;
    }

    HalSMDouble* f1;
    if (v1->type==HalSMVariableType_str) {
        f1=HalSMDouble_parse(memory,*(HalStringFormatChar**)v1->value);
    } else if (v1->type==HalSMVariableType_int) {
        f1=HalSMDouble_FromHalSMInteger(memory,(HalSMInteger*)v1->value);
    } else {
        f1=(HalSMDouble*)v1->value;
    }
    return HalSMDouble_Div(memory,f0,f1);
}

HalSMFunctionArray* HalSMFunctionArray_init(HalSMMemory* memory,HalSMArray* args,HalSMFunctionArrayType type)
{
    HalSMFunctionArray* out=HalSMMemory_malloc(memory,sizeof(HalSMFunctionArray));
    out->args=args;
    out->type=type;
    return out;
}

//GOTO

HalSMGOTO* HalSMGOTO_init(HalSMMemory* memory,HalStringFormatChar* sector)
{
    HalSMGOTO* out=HalSMMemory_malloc(memory,sizeof(HalSMGOTO));
    out->sector=sector;
    return out;
}

HalSMGOTOSector* HalSMGOTOSector_init(HalSMMemory* memory,HalStringFormatChar* sector,ui64 index,HalSMLocalFunction* lf)
{
    HalSMGOTOSector* out=HalSMMemory_malloc(memory,sizeof(HalSMGOTOSector));
    out->sector=sector;
    out->index=index;
    out->lf=lf;
    return out;
}

//String

ui64 string_len(HalStringFormatChar* s)
{
    ui64 out=0;
    while (HSFCharCompare(*s++,ASCIIH('\0'))==0) {
        out++;
    }
    return out;
}

HalSMInteger* string_len_HalSMInteger(HalSMMemory* memory,HalStringFormatChar* s)
{
    HalSMInteger* out=HalSMInteger_init(memory,0,HalSMMemory_malloc(memory,0),0);
    while (HSFCharCompare(*s,ASCIIH('\0'))==0) {
        HalSMInteger_AddSelf(memory,out,&HalSMIntegerOne);
        s++;
    }
    return out;
}

HalStringFormatChar* string_cat(HalStringFormatChar* s1,HalStringFormatChar* s2)
{
    if ((s1==NOTHALSMNULLPOINTER)&&(s2==NOTHALSMNULLPOINTER))return NOTHALSMNULLPOINTER;
    HalStringFormatChar* ptr=s1;
    while (HSFCharCompare(*ptr,ASCIIH('\0'))==0) ptr++;
    while (HSFCharCompare(*s2,ASCIIH('\0'))==0) *ptr++=*s2++;
    *ptr=ASCIIH('\0');
    return s1;
}

HalStringFormatChar* string_cpy(HalStringFormatChar* s1,HalStringFormatChar* s2)
{
    if ((s1==NOTHALSMNULLPOINTER)&&(s2==NOTHALSMNULLPOINTER))return NOTHALSMNULLPOINTER;
    
    HalStringFormatChar* ptr=s1;
    while (HSFCharCompare(*s2,ASCIIH('\0'))==0)
    {
        *s1=*s2;
        s1++;
        s2++;
    }
    *s1=ASCIIH('\0');
    return ptr;
}

HalStringFormatChar* stringncpy(HalStringFormatChar* s1,HalStringFormatChar* s2,ui64 n)
{
    if ((s1==NOTHALSMNULLPOINTER)&&(s2==NOTHALSMNULLPOINTER))return NOTHALSMNULLPOINTER;
    HalStringFormatChar* ptr=s1;

    while (HSFCharCompare(*s2,ASCIIH('\0'))==0&&n--)
    {
        *s1=*s2;
        s1++;
        s2++;
    }

    *s1=ASCIIH('\0');
    return ptr;
}

void* memory_cpy(void* dst,const void* src,ui64 n)
{
    char* bdst=(char*)dst;
    char* bsrc=(char*)src;
    if ((bdst!=NOTHALSMNULLPOINTER)&&(bsrc!=NOTHALSMNULLPOINTER)) {
        while (n)
        {
            *(bdst++)=*(bsrc++);
            --n;
        }
    }
    return dst;
}

//HalSM File Pointer

HalStringFormatChar* HFP_toString(HalSMCompiler* hsmc,HalSMVariable* v){return U8H("<HalSMFilePointer>");}
HalSMVariable* HFP_add(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HFP_sub(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HFP_mul(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HFP_div(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HFP_pow(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HFP_modulo(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HFP_increment(HalSMCompiler* hsmc,HalSMVariable* v){return v;}
HalSMVariable* HFP_decrement(HalSMCompiler* hsmc,HalSMVariable* v){return v;}
ui8 HFP_isEqual(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){{return a->value==b->value;}}
ui8 HFP_isMore(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return 0;}
ui8 HFP_isLess(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return 0;}
ui8 HFP_getBool(HalSMCompiler* hsmc,HalSMVariable* v){return 1;}
i64 HFP_indexVariableByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name) {
    if (HSFCompare(name,U8H("SEEK_SET"))) {
        return 0;
    } else if (HSFCompare(name,U8H("SEEK_CUR"))) {
        return 1;
    } else if (HSFCompare(name,U8H("SEEK_END"))) {
        return 2;
    }
    return -1;
}
i64 HFP_indexFunctionByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name) {
    if (HSFCompare(name,U8H("close"))) {
        return 0;
    } else if (HSFCompare(name,U8H("read"))) {
        return 1;
    } else if (HSFCompare(name,U8H("write"))) {
        return 2;
    } else if (HSFCompare(name,U8H("eof"))) {
        return 3;
    } else if (HSFCompare(name,U8H("seek"))) {
        return 4;
    } else if (HSFCompare(name,U8H("tell"))) {
        return 5;
    } else if (HSFCompare(name,U8H("rewind"))) {
        return 6;
    } else if (HSFCompare(name,U8H("flush"))) {
        return 7;
    } else if (HSFCompare(name,U8H("readUTF8"))) {
        return 8;
    } else if (HSFCompare(name,U8H("writeUTF8"))) {
        return 9;
    } else if (HSFCompare(name,U8H("readHSF"))) {
        return 10;
    } else if (HSFCompare(name,U8H("writeHSF"))) {
        return 11;
    } else if (HSFCompare(name,U8H("readUTF16"))) {
        return 12;
    } else if (HSFCompare(name,U8H("writeUTF16"))) {
        return 13;
    }
    return -1;
}
HalSMVariable* HFP_getVariableByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name) {
    if (HSFCompare(name,U8H("SEEK_SET"))) {
        return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,0),HalSMVariableType_int);
    } else if (HSFCompare(name,U8H("SEEK_CUR"))) {
        return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,1),HalSMVariableType_int);
    } else if (HSFCompare(name,U8H("SEEK_END"))) {
        return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,2),HalSMVariableType_int);
    }
    return &hvoid;
}
HalSMVariable* HFP_getFunctionByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name) {
    if (HSFCompare(name,U8H("close"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[0]->value;
    } else if (HSFCompare(name,U8H("read"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[1]->value;
    } else if (HSFCompare(name,U8H("write"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[2]->value;
    } else if (HSFCompare(name,U8H("eof"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[3]->value;
    } else if (HSFCompare(name,U8H("seek"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[4]->value;
    } else if (HSFCompare(name,U8H("tell"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[5]->value;
    } else if (HSFCompare(name,U8H("rewind"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[6]->value;
    } else if (HSFCompare(name,U8H("flush"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[7]->value;
    } else if (HSFCompare(name,U8H("readUTF8"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[8]->value;
    } else if (HSFCompare(name,U8H("writeUTF8"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[9]->value;
    } else if (HSFCompare(name,U8H("readHSF"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[10]->value;
    } else if (HSFCompare(name,U8H("writeHSF"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[11]->value;
    } else if (HSFCompare(name,U8H("readUTF16"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[12]->value;
    } else if (HSFCompare(name,U8H("writeUTF16"))) {
        return ((HalSMVariableCustom*)v->value)->type->funcs->elements[13]->value;
    }
    return &hvoid;
}

HalSMVariable* HFP_getItem(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HFP_shiftRight(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HFP_shiftLeft(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HFP_convert(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HFP_xor(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HFP_and(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HFP_or(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HFP_bnot(HalSMCompiler* hsmc,HalSMVariable* a){return &hvoid;}

HalSMVariable* HFP_close(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        hsmc->filesystemlibrary->closeFile(((HalSMVariableCustom*)args->arr[0]->value)->value);
    }
    return &hvoid;
}

HalSMVariable* HFP_read(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==1) {
        HalSMInteger* out=HalSMInteger_init(hsmc->memory,0,HalSMMemory_malloc(hsmc->memory,1),1);
        hsmc->filesystemlibrary->read(out->value,1,1,((HalSMVariableCustom*)args->arr[0]->value)->value);
        return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_int);
    } else if (args->size==2&&args->arr[1]->type==HalSMVariableType_int) {
        HalSMByteArray* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMByteArray));
        ui64 size=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value);
        out->bytes=HalSMMemory_malloc(hsmc->memory,size);
        out->size=size;
        hsmc->filesystemlibrary->read(out->bytes,1,size,((HalSMVariableCustom*)args->arr[0]->value)->value);
        return HalSMVariable_init(hsmc->memory,HalSMVariableCustom_init(hsmc->memory,(HalSMVariableTypeCustom*)hsmc->sys_custom_variables->elements[1]->value->value,out,&NULLDict,&NULLDict),HalSMVariableType_custom);
    } else if (args->size==3&&args->arr[1]->type==HalSMVariableType_int&&args->arr[2]->type==HalSMVariableType_int) {
        HalSMByteArray* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMByteArray));
        ui64 count,size;
        size=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value);
        count=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[2]->value);
        out->bytes=HalSMMemory_malloc(hsmc->memory,count*size);
        out->size=size;
        hsmc->filesystemlibrary->read(out->bytes,size,count,((HalSMVariableCustom*)args->arr[0]->value)->value);
        return HalSMVariable_init(hsmc->memory,HalSMVariableCustom_init(hsmc->memory,(HalSMVariableTypeCustom*)hsmc->sys_custom_variables->elements[1]->value->value,out,&NULLDict,&NULLDict),HalSMVariableType_custom);
    }
    return &hvoid;
}

HalSMVariable* HFP_write(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==2) {
        if (args->arr[1]->type==HalSMVariableType_int) {
            HalSMInteger* out=(HalSMInteger*)args->arr[1]->value;
            return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,hsmc->filesystemlibrary->write(out->value,1,out->size,((HalSMVariableCustom*)args->arr[0]->value)->value)),HalSMVariableType_int);
        } else if (args->arr[1]->type==HalSMVariableType_custom&&HSFCompare(((HalSMVariableCustom*)args->arr[1]->value)->type->name,U8H("ByteArray"))) {
            HalSMByteArray* out=(HalSMByteArray*)args->arr[1]->value;
            return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,hsmc->filesystemlibrary->write(out->bytes,1,out->size,((HalSMVariableCustom*)args->arr[0]->value)->value)),HalSMVariableType_int);
        }
    }
    return &hvoid;
}

HalSMVariable* HFP_eof(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==1) {
        int e=hsmc->filesystemlibrary->eof(((HalSMVariableCustom*)args->arr[0]->value)->value);
        HalSMInteger* out;
        if (e<0) {
            out=HalSMInteger_FromSignedInteger(hsmc->memory,(signed int)e);
        } else {
            out=HalSMInteger_FromUnsignedInteger(hsmc->memory,(ui32)e);
        }
        return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_int);
    }
    return &hvoid;
}

HalSMVariable* HFP_seek(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==2&&args->arr[1]->type==HalSMVariableType_int) {
        hsmc->filesystemlibrary->seek(((HalSMVariableCustom*)args->arr[0]->value)->value,HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value),0);
    } else if (args->size==3&&args->arr[1]->type==HalSMVariableType_int&&args->arr[2]->type==HalSMVariableType_int) {
        hsmc->filesystemlibrary->seek(((HalSMVariableCustom*)args->arr[0]->value)->value,HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value),HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[2]->value));
    }
    return &hvoid;
}

HalSMVariable* HFP_tell(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==1) {
        return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,hsmc->filesystemlibrary->tell(((HalSMVariableCustom*)args->arr[0]->value)->value)),HalSMVariableType_int);
    }
    return &hvoid;
}

HalSMVariable* HFP_rewind(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==1) {
        hsmc->filesystemlibrary->rewind(((HalSMVariableCustom*)args->arr[0]->value)->value);
    }
    return &hvoid;
}

HalSMVariable* HFP_flush(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==1) {
        hsmc->filesystemlibrary->flush(((HalSMVariableCustom*)args->arr[0]->value)->value);
    }
    return &hvoid;
}

HalSMVariable* HFP_readUTF8(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if (args->size==2&&args->arr[1]->type==HalSMVariableType_int) {
        ui64 l=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value),i;
        ui8 bytes[4];
        void* p=((HalSMVariableCustom*)args->arr[0]->value)->value;
        HalStringFormatChar* out=HalSMMemory_malloc(hsmc->memory,0);
        for (i=0;i<l;i++) {
            hsmc->filesystemlibrary->read(bytes,1,1,p);
            if (hsmc->filesystemlibrary->eof(p)) {break;}
            out=HalSMMemory_realloc(hsmc->memory,out,i*sizeof(HalStringFormatChar)+sizeof(HalStringFormatChar));
            if (utf8_is_single_byte(bytes)) {
                if (bytes[0]==0) {break;}
                out[i]=U8CH(bytes[0]);
            } else if (utf8_is_double_byte(bytes)) {
                hsmc->filesystemlibrary->read(&bytes[1],1,1,p);
                out[i]=U8CH((bytes[0]<<8)|bytes[1]);
            } else if (utf8_is_triple_byte(bytes)) {
                hsmc->filesystemlibrary->read(&bytes[1],2,1,p);
                out[i]=U8CH(bytes[2]+(bytes[1]<<8)+(bytes[0]<<16));
            } else if (utf8_is_quadruple_byte(bytes)) {
                hsmc->filesystemlibrary->read(&bytes[1],3,1,p);
                out[i]=U8CH(bytes[3]+(bytes[2]<<8)+(bytes[1]<<16)+(bytes[0]<<24));
            }
        }
        out=HalSMMemory_realloc(hsmc->memory,out,i*sizeof(HalStringFormatChar)+sizeof(HalStringFormatChar));
        out[i]=ASCIIH('\0');
        return HalSMVariable_init_str(hsmc->memory,out);
    }
    return &hvoid;
}

HalSMVariable* HFP_writeUTF8(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[1]->type==HalSMVariableType_str) {
        char* str=HU8(*(HalStringFormatChar**)args->arr[1]->value);
        void* p=((HalSMVariableCustom*)args->arr[0]->value)->value;
        ui64 l=0;
        char* pstr;
        for (pstr=str;;pstr++) {
            if (*pstr=='\0'){l=pstr-str;break;}
        }
        return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,hsmc->filesystemlibrary->write(str,1,l,p)),HalSMVariableType_int);
    }
    return &hvoid;
}

HalSMVariable* HFP_readHSF(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[1]->type==HalSMVariableType_int) {
        ui64 l=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value),i;
        ui8 bytes[4];
        void* p=((HalSMVariableCustom*)args->arr[0]->value)->value;
        HalStringFormatChar* out=HalSMMemory_malloc(hsmc->memory,0);
        for (i=0;i<l;i++) {
            hsmc->filesystemlibrary->read(bytes,4,1,p);
            if (hsmc->filesystemlibrary->eof(p)){break;}
            if (*(ui32*)bytes==0){break;}
            out=HalSMMemory_realloc(hsmc->memory,out,i*sizeof(HalStringFormatChar)+sizeof(HalStringFormatChar));
            ui32 hi=*(ui32*)bytes;
            out[i]=*(HalStringFormatChar*)&hi;
        }
        out=HalSMMemory_realloc(hsmc->memory,out,i*sizeof(HalStringFormatChar)+sizeof(HalStringFormatChar));
        out[i]=ASCIIH('\0');
        return HalSMVariable_init_str(hsmc->memory,out);
    }
    return &hvoid;
}

HalSMVariable* HFP_writeHSF(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[1]->type==HalSMVariableType_str) {
        HalStringFormatChar* str=*(HalStringFormatChar**)args->arr[1]->value;
        ui64 l=string_len(str);
        void* p=((HalSMVariableCustom*)args->arr[0]->value)->value;
        return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,hsmc->filesystemlibrary->write((ui8*)str,4,l,p)),HalSMVariableType_int);
    }
    return &hvoid;
}


HalSMVariable* HFP_readUTF16(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[1]->type==HalSMVariableType_int) {
        ui64 l=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[1]->value),i;
        ui16 v=0;
        void* p=((HalSMVariableCustom*)args->arr[0]->value)->value;
        short* out=HalSMMemory_malloc(hsmc->memory,0);
        hsmc->filesystemlibrary->read(&v,2,1,p);
        if (v!=0xfeff){return &hvoid;}
        for (i=0;i<l;i++) {
            hsmc->filesystemlibrary->read(&v,2,1,p);
            if (hsmc->filesystemlibrary->eof(p)){break;}
            if (v==0){break;}
            out=HalSMMemory_realloc(hsmc->memory,out,i*sizeof(short)+sizeof(short));
            out[i]=v;
        }
        out=HalSMMemory_realloc(hsmc->memory,out,i*sizeof(short)+sizeof(short));
        out[i]=0x00;
        return HalSMVariable_init_str(hsmc->memory,U16H(out));
    }
    return &hvoid;
}

HalSMVariable* HFP_writeUTF16(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    if (args->size==2&&args->arr[1]->type==HalSMVariableType_str) {
        short* str=HU16(*(HalStringFormatChar**)args->arr[1]->value);
        void* p=((HalSMVariableCustom*)args->arr[0]->value)->value;
        ui64 l=0;
        short* pstr;
        for (pstr=str;;pstr++) {
            if (*pstr==0){l=pstr-str;break;}
        }
        short utf16=0xfeff;
        ui64 out=hsmc->filesystemlibrary->write(&utf16,1,2,p)+hsmc->filesystemlibrary->write(str,2,l,p);
        return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedLongLongInteger(hsmc->memory,out),HalSMVariableType_int);
    }
    return &hvoid;
}

//HalSM Byte Array

HalStringFormatChar* HBA_toString(HalSMCompiler* hsmc,HalSMVariable* v) {
    HalSMByteArray* ba=(HalSMByteArray*)((HalSMVariableCustom*)v->value)->value;
    HalStringFormatChar* join=U8H(", ");
    if(ba->size==0) {
        return U8H("");
    } else if (ba->size==1) {
        return HalSMInteger_toString(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,ba->bytes[0]),0);
    }
    ui64 size;
    ui64 lj=string_len(join);
    HalStringFormatChar* out=HalSMMemory_calloc(hsmc->memory,2,sizeof(HalStringFormatChar));
    out[0]=ASCIIH('[');
    out[1]=ASCIIH('\0');
    ui64 i;
    size=1;
    for (i=0;i<ba->size;i++) {
        HalStringFormatChar* oc=HalSMInteger_toString(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,ba->bytes[i]),0);
        AdditionStrings(hsmc->memory,&out,oc,size,string_len(oc));
        size+=string_len(oc);
        if (i!=ba->size-1) {
            AdditionStrings(hsmc->memory,&out,join,size,lj);
            size+=lj;
        }
    }
    AdditionStrings(hsmc->memory,&out,U8H("]"),size,1);
    out[size+1]=ASCIIH('\0');
    return out;
}
HalSMVariable* HBA_add(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HBA_sub(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HBA_mul(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HBA_div(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HBA_pow(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HBA_modulo(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return a;}
HalSMVariable* HBA_increment(HalSMCompiler* hsmc,HalSMVariable* v){return v;}
HalSMVariable* HBA_decrement(HalSMCompiler* hsmc,HalSMVariable* v){return v;}
ui8 HBA_isEqual(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){{return a->value==b->value;}}
ui8 HBA_isMore(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return 0;}
ui8 HBA_isLess(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return 0;}
ui8 HBA_getBool(HalSMCompiler* hsmc,HalSMVariable* v){return 1;}
i64 HBA_indexVariableByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name) {
    return -1;
}
i64 HBA_indexFunctionByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name) {
    return -1;
}
HalSMVariable* HBA_getVariableByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name) {
    return &hvoid;
}
HalSMVariable* HBA_getFunctionByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name) {
    return &hvoid;
}
HalSMVariable* HBA_getItem(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b) {
    HalSMByteArray* ba=(HalSMByteArray*)((HalSMVariableCustom*)a->value)->value;
    if (b->type==HalSMVariableType_int&&((HalSMInteger*)b->value)->size<9) {
        ui64 ind=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)b->value);
        if (ind<ba->size) {
            return HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,ba->bytes[ind]),HalSMVariableType_int);
        }
    }
    return &hvoid;
}
HalSMVariable* HBA_shiftRight(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HBA_shiftLeft(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HBA_convert(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b) {
    if (b->type==HalSMVariableType_HalSMArray) {
        HalSMByteArray* ba=(HalSMByteArray*)((HalSMVariableCustom*)a->value)->value;
        HalSMArray* out=HalSMMemory_malloc(hsmc->memory,sizeof(HalSMArray));
        out->size=ba->size;
        out->arr=HalSMMemory_malloc(hsmc->memory,ba->size*sizeof(HalSMVariable));
        ui64 i;
        for (i=0;i<ba->size;i++) {
            out->arr[i]=HalSMVariable_init(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,ba->bytes[i]),HalSMVariableType_int);
        }
        return HalSMVariable_init(hsmc->memory,out,HalSMVariableType_HalSMArray);
    }
    return &hvoid;
}

HalSMVariable* HBA_xor(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HBA_and(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HBA_or(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b){return &hvoid;}
HalSMVariable* HBA_bnot(HalSMCompiler* hsmc,HalSMVariable* a){return &hvoid;}

/*
* HalSM Instructions
*/

