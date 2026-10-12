
#include <HalSM.h>
//HalSMDouble

HalSMDouble* HalSMDouble_init(HalSMMemory* memory,unsigned char negative,HalSMInteger* a,HalSMInteger* b)
{
    
    HalSMDouble* out=HalSMMemory_malloc(memory,sizeof(HalSMDouble));
    out->negative=negative;
    out->a=a;
    out->b=b;
    return out;
}

HalSMDouble* HalSMDouble_copy(HalSMMemory* memory,HalSMDouble* a)
{
    HalSMDouble* out=HalSMMemory_malloc(memory,sizeof(HalSMDouble));
    out->negative=a->negative;

    out->a=HalSMInteger_copy(memory,a->a);

    out->b=HalSMInteger_copy(memory,a->b);
    return out;
}

HalSMDouble* HalSMDouble_FromUnsignedInteger(HalSMMemory* memory,unsigned int value)
{
    unsigned char* v=HalSMMemory_malloc(memory,4);
    v[0]=(value>>24)&0xFF;
    v[1]=(value>>16)&0xFF;
    v[2]=(value>>8)&0xFF;
    v[3]=value&0xFF;
    
    unsigned char* v1=HalSMMemory_malloc(memory,1);
    v1[0]=1;
    return HalSMDouble_init(memory,0,HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v,4)),HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v1,1)));
}

HalSMDouble* HalSMDouble_FromSignedInteger(HalSMMemory* memory,signed int value)
{
    unsigned char* v=HalSMMemory_malloc(memory,4);
    unsigned char negate=value<0?1:0;
    if (negate) {
        value=-value;
    }
    v[0]=(value>>24)&0xFF;
    v[1]=(value>>16)&0xFF;
    v[2]=(value>>8)&0xFF;
    v[3]=value&0xFF;
    
    unsigned char* v1=HalSMMemory_malloc(memory,1);
    v1[0]=1;
    return HalSMDouble_init(memory,negate,HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v,4)),HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v1,1)));
}

HalSMDouble* HalSMDouble_FromFloat(HalSMMemory* memory,float value)
{
    unsigned char negate=value<0?1:0;
    value=negate?-value:value;

    //precision
    unsigned int c=1000000;
    float b=(value-((unsigned int)value))*c;
    unsigned int d=((unsigned int)value)*c+(unsigned int)(b+0.5);

    while (d%10==0) {
        d/=10;
        c/=10;
    }

    unsigned char* v=HalSMMemory_malloc(memory,4);
    v[0]=(d>>24)&0xFF;
    v[1]=(d>>16)&0xFF;
    v[2]=(d>>8)&0xFF;
    v[3]=d&0xFF;

    unsigned char* v1=HalSMMemory_malloc(memory,4);
    v1[0]=(c>>24)&0xFF;
    v1[1]=(c>>16)&0xFF;
    v1[2]=(c>>8)&0xFF;
    v1[3]=c&0xFF;
    return HalSMDouble_clear(memory,HalSMDouble_init(memory,negate,HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v,4)),HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v1,4))));
}

HalSMDouble* HalSMDouble_FromDouble(HalSMMemory* memory,double value)
{
    unsigned char negate=value<0?1:0;
    value=negate?-value:value;

    unsigned int c=100000000;
    double b=(value-((unsigned int)value))*c;
    unsigned int d=((unsigned int)value)*c+(unsigned int)(b+0.5);

    while (d%10==0) {
        d/=10;
        c/=10;
    }

    unsigned char* v=HalSMMemory_malloc(memory,4);
    v[0]=(d>>24)&0xFF;
    v[1]=(d>>16)&0xFF;
    v[2]=(d>>8)&0xFF;
    v[3]=d&0xFF;

    unsigned char* v1=HalSMMemory_malloc(memory,4);
    v1[0]=(c>>24)&0xFF;
    v1[1]=(c>>16)&0xFF;
    v1[2]=(c>>8)&0xFF;
    v1[3]=c&0xFF;
    return HalSMDouble_clear(memory,HalSMDouble_init(memory,negate,HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v,4)),HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v1,4))));
}

HalSMDouble* HalSMDouble_FromHalSMInteger(HalSMMemory* memory,HalSMInteger* value)
{
    unsigned char* v1=HalSMMemory_malloc(memory,1);
    v1[0]=1;
    value=HalSMInteger_copy(memory,value);
    unsigned char isNeg=value->negative;
    value->negative=0;
    return HalSMDouble_init(memory,isNeg,value,HalSMInteger_init(memory,0,v1,1));

}

HalSMDouble* HalSMDouble_parse(HalSMMemory* memory,HalStringFormatChar* str)
{
    HalSMInteger* rez = HalSMInteger_FromUnsignedInteger(memory,0), *fact = HalSMInteger_FromUnsignedInteger(memory,1);
    //HalSMDouble HalSMDoubleTen={0,&HalSMIntegerTen,&HalSMIntegerOne};
    //HalSMInteger HalSMDoubledf={0,(unsigned char[]){0,0,0,1},4};
    //HalSMInteger HalSMDoubleds={0,(unsigned char[]){0,0,0,1},4};
    //HalSMDouble HalSMDoubled={0,&HalSMDoubledf,&HalSMDoubleds};
    HalSMInteger num={0,(unsigned char[]){0},1};
    ui8 isn=0;
    if (HSFCharCompare(*str,ASCIIH('-'))){
        str++;
        //fact->negative=1;
        isn=1;
    }
    ui64 l=HalStringFormat_length(str);
    //OPTIMIZE HSF
    for (ui64 point_seen = 0,i=0;i<l; i++,str++){
        if (HSFCharCompare(*str,ASCIIH('.'))||HSFCharCompare(*str,ASCIIH(','))){
            point_seen = 1; 
            continue;
        }
        long long int d = str->symbol - '0';
        if (d >= 0 && d <= 9){
            if (point_seen){HalSMInteger_MulSelf(memory,fact,&HalSMIntegerTen);}
            HalSMInteger_MulSelf(memory,rez,&HalSMIntegerTen);
            num.value[0]=d;
            HalSMInteger_AddSelf(memory,rez,&num);
            //HalSMDoubled.a->value[3]=d;
            //HalSMDouble_AddSelf(memory,rez,&HalSMDoubled);
        } else {break;}
    }
    
    return HalSMDouble_clear(memory,HalSMDouble_init(memory,isn,rez,fact));
}

unsigned char HalSMDouble_isMore(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    HalSMDouble* r=HalSMDouble_Sub(memory,a,b);
    if (r->negative==1||HalSMDouble_isEqual(r,&HalSMDoubleZero)){return 0;}
    return 1;
}

unsigned char HalSMDouble_isLess(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    HalSMDouble* r=HalSMDouble_Sub(memory,a,b);
    if (r->negative==0||HalSMDouble_isEqual(r,&HalSMDoubleZero)){HalSMDouble_Free(memory,r);return 0;}
    return 1;
}

unsigned char HalSMDouble_isEqual(HalSMDouble* a,HalSMDouble* b)
{
    /*HalSMInteger* av0=HalSMInteger_clear(memory,a->a);
    HalSMInteger* av1=HalSMInteger_clear(memory,a->b);
    HalSMInteger* bv0=HalSMInteger_clear(memory,b->a);
    HalSMInteger* bv1=HalSMInteger_clear(memory,b->b);*/
    //if (a->negative!=b->negative||av0->size!=bv0->size||av1->size!=bv1->size) {return 0;}
    if (a->negative!=b->negative){return 0;}
    /*unsigned long long i;
    for (i=0;i<av0->size;i++) {
        if (av0->value[i]!=bv0->value[i]) {return 0;}
    }
    for (i=0;i<av1->size;i++) {
        if (av1->value[i]!=bv1->value[i]) {return 0;}
    }
    return 1;*/
    if (HalSMInteger_isEqual(a->a,b->a)==0||HalSMInteger_isEqual(a->b,b->b)==0){return 0;}
    return 1;
}

void HalSMDouble_AddSelf(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    if (a->negative&&b->negative==0) {
        a->negative=0;
        HalSMDouble_SubSelf(memory,a,b);
        if (a->a->size==0||a->b->size==0||(a->a->size==1&&a->a->value[0]==0)||(a->b->size==1&&a->b->value[0]==0)) {
            a->negative=0;
            return;
        }
        a->negative=a->negative?0:1;
        return;
    } else if (b->negative&&a->negative==0) {
        b->negative=0;
        HalSMDouble_SubSelf(memory,a,b);
        b->negative=1;
        return;
    }

    HalSMInteger_MulSelf(memory,a->a,b->b);
    HalSMInteger_AddSelf(memory,a->a,HalSMInteger_Mul(memory,b->a,a->b));
    HalSMInteger_MulSelf(memory,a->b,b->b);
    //HalSMDouble_clear(memory,a);
}

HalSMDouble* HalSMDouble_Add(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    HalSMDouble* out=HalSMDouble_copy(memory,a);
    HalSMDouble_AddSelf(memory,out,b);
    return out;
}

void HalSMDouble_SubSelf(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    if (b->negative&&a->negative==0) {
        b->negative=0;
        HalSMDouble_AddSelf(memory,a,b);
        b->negative=1;
        return;
    } else if (a->negative&&b->negative==0) {
        a->negative=0;
        HalSMDouble_AddSelf(memory,a,b);
        a->negative=1;
        return;
    }

    HalSMInteger_MulSelf(memory,a->a,b->b);
    HalSMInteger* bb=HalSMInteger_Mul(memory,b->a,a->b);
    HalSMInteger_SubSelf(memory,a->a,bb);
    HalSMInteger_Free(memory,bb);
    HalSMInteger_MulSelf(memory,a->b,b->b);
    if (a->negative&&b->negative) {
        a->negative=a->a->negative?0:1;
    } else {
        a->negative=a->a->negative;
    }
    a->a->negative=0;
    //HalSMDouble_clear(memory,a);
}

HalSMDouble* HalSMDouble_Sub(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    HalSMDouble* out=HalSMDouble_copy(memory,a);
    HalSMDouble_SubSelf(memory,out,b);
    return out;
}

void HalSMDouble_MulSelf(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    HalSMInteger_MulSelf(memory,a->a,b->a);
    HalSMInteger_MulSelf(memory,a->b,b->b);
    if (a->negative&&b->negative)a->negative=0;
    else if (b->negative)a->negative=1;
    //HalSMDouble_clear(memory,a);
}

HalSMDouble* HalSMDouble_Mul(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    HalSMDouble* out=HalSMDouble_copy(memory,a);
    HalSMDouble_MulSelf(memory,out,b);
    return out;
}

void HalSMDouble_DivSelf(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    HalSMInteger_MulSelf(memory,a->a,b->b);
    HalSMInteger_MulSelf(memory,a->b,b->a);
    if (a->negative&&b->negative)a->negative=0;
    else if (b->negative)a->negative=1;
    //HalSMDouble_clear(memory,a);
}

HalSMDouble* HalSMDouble_Div(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b)
{
    HalSMDouble* out=HalSMDouble_copy(memory,a);
    HalSMDouble_DivSelf(memory,out,b);
    return out;
}

HalStringFormatChar* HalSMDouble_toString(HalSMMemory* memory,HalSMDouble* a,unsigned char isHex)
{
    HalSMInteger* remainder=HalSMMemory_malloc(memory,sizeof(HalSMInteger));
    HalSMInteger* quotient=HalSMInteger_Div(memory,a->a,a->b,remainder);
    HalStringFormatChar* f=HalSMInteger_toString(memory,quotient,0);
    HalSMInteger* res=HalSMInteger_FromUnsignedInteger(memory,0);
    unsigned long long int i=0;
    unsigned long long int cn=0;
    unsigned char isNotStart=1;
    while (HalSMInteger_isMore(remainder,&HalSMIntegerZero)) {
        if (HalSMInteger_isMore(remainder,a->b)){
            HalSMInteger_MulSelf(memory,res,&HalSMIntegerTen);
            HalSMInteger_AddSelf(memory,res,HalSMInteger_Div(memory,remainder,a->b,remainder));
            isNotStart=0;
        } else if (HalSMInteger_isEqual(remainder,a->b)){
            HalSMInteger_MulSelf(memory,res,&HalSMIntegerTen);
            HalSMInteger_AddSelf(memory,res,&HalSMIntegerOne);
            break;
        } else if (isNotStart) {
            cn++;
        } else {
            HalSMInteger_MulSelf(memory,res,&HalSMIntegerTen);
        }
        HalSMInteger_MulSelf(memory,remainder,&HalSMIntegerTen);
        if (i==100)break;
        i++;
    }
    HalStringFormatChar* out;
    if (cn>1) {
        HalStringFormatChar* t=HalSMInteger_toString(memory,res,0);
        out=HalSMMemory_malloc(memory,(cn+string_len(t)+1)*sizeof(HalStringFormatChar));
        memory_cpy(&out[cn-1],t,(string_len(t)+1)*sizeof(HalStringFormatChar));
        while (cn-->1) {
            out[cn-1]=ASCIIH('0');
        }
    } else {
        out=HalSMInteger_toString(memory,res,0);
    }
    out=ConcatenateStrings(memory,f,ConcatenateStrings(memory,U8H("."),out));
    return (a->negative)?ConcatenateStrings(memory,U8H("-"),out):out;
}

HalStringFormatChar* HalSMDouble_toStringRound(HalSMMemory* memory,HalSMDouble* a,unsigned long long int round)
{
    HalSMInteger* remainder=HalSMMemory_malloc(memory,sizeof(HalSMInteger));
    HalSMInteger* quotient=HalSMInteger_Div(memory,a->a,a->b,remainder);
    HalStringFormatChar* f=HalSMInteger_toString(memory,quotient,0);
    HalSMInteger* res=HalSMInteger_FromUnsignedInteger(memory,0);
    unsigned long long int i=0;
    unsigned long long int cn=0;
    unsigned char isNotStart=1;
    while (HalSMInteger_isMore(remainder,&HalSMIntegerZero)) {
        if (HalSMInteger_isMore(remainder,a->b)){
            HalSMInteger_MulSelf(memory,res,&HalSMIntegerTen);
            HalSMInteger_AddSelf(memory,res,HalSMInteger_Div(memory,remainder,a->b,remainder));
            isNotStart=0;
        } else if (HalSMInteger_isEqual(remainder,a->b)){
            HalSMInteger_MulSelf(memory,res,&HalSMIntegerTen);
            HalSMInteger_AddSelf(memory,res,&HalSMIntegerOne);
            break;
        } else if (isNotStart) {
            cn++;
        } else {
            HalSMInteger_MulSelf(memory,res,&HalSMIntegerTen);
        }
        HalSMInteger_MulSelf(memory,remainder,&HalSMIntegerTen);
        if (i==round)break;
        i++;
    }
    HalStringFormatChar* out;
    if (cn>1) {
        HalStringFormatChar* t=HalSMInteger_toString(memory,res,0);
        out=HalSMMemory_malloc(memory,(cn+string_len(t)+1)*sizeof(HalStringFormatChar));
        memory_cpy(&out[cn-1],t,(string_len(t)+1)*sizeof(HalStringFormatChar));
        while (cn-->1) {
            out[cn-1]=ASCIIH('0');
        }
    } else {
        out=HalSMInteger_toString(memory,res,0);
    }
    out=ConcatenateStrings(memory,f,ConcatenateStrings(memory,U8H("."),out));
    return (a->negative)?ConcatenateStrings(memory,U8H("-"),out):out;
}

double HalSMDouble_ToDouble(HalSMMemory* memory,HalSMDouble* a)
{
    int i;
    HalSMInteger* ai=HalSMInteger_copy(memory,a->a);
    HalSMInteger remainder;
    HalSMInteger_DivSelf(memory,ai,a->b,&remainder);
    double out=HalSMInteger_ToUnsignedInteger(ai);
    for (i=10;i<=10000000;i*=10) {
        ai=HalSMInteger_copy(memory,&remainder);
        if (HalSMInteger_isEqual(ai,&HalSMIntegerZero)){break;}
        HalSMInteger_MulSelf(memory,ai,&HalSMIntegerTen);
        HalSMInteger_DivSelf(memory,ai,a->b,&remainder);
        out+=((double)HalSMInteger_ToUnsignedInteger(ai))/i;
    }
    if (a->negative) {
        out=-out;
    }
    return out;
}

float HalSMDouble_ToFloat(HalSMMemory* memory,HalSMDouble* a)
{
    int i;
    HalSMInteger* ai=HalSMInteger_copy(memory,a->a);
    HalSMInteger remainder;
    HalSMInteger_DivSelf(memory,ai,a->b,&remainder);
    float out=HalSMInteger_ToUnsignedInteger(ai);
    for (i=10;i<=10000000;i*=10) {
        ai=HalSMInteger_copy(memory,&remainder);
        if (HalSMInteger_isEqual(ai,&HalSMIntegerZero)){break;}
        HalSMInteger_MulSelf(memory,ai,&HalSMIntegerTen);
        HalSMInteger_DivSelf(memory,ai,a->b,&remainder);
        out+=((float)HalSMInteger_ToUnsignedInteger(ai))/i;
    }
    if (a->negative) {
        out=-out;
    }
    return out;
}

HalSMInteger* stepsr(HalSMMemory* memory,HalSMInteger* k,HalSMInteger* a,HalSMInteger* x)
{
    HalSMInteger* temp=HalSMInteger_Sub(memory,k,&HalSMIntegerOne);
    return HalSMInteger_Div(memory,HalSMInteger_Add(memory,HalSMInteger_Mul(memory,temp,x),HalSMInteger_Pow(memory,HalSMInteger_Div(memory,a,x,NOTHALSMNULLPOINTER),temp)),k,NOTHALSMNULLPOINTER);
}

HalSMInteger* iPow(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* e)
{
    HalSMInteger* r=HalSMInteger_copy(memory,&HalSMIntegerOne);
    if (HalSMInteger_isEqual(e,&HalSMIntegerZero)){return r;}
    while (HalSMInteger_isEqual(e,&HalSMIntegerZero)==0) {
        if (HalSMInteger_getBit(e,0,0)==1){HalSMInteger_MulSelf(memory,r,a);}
        HalSMInteger_shiftRightSelf(memory,e,1);
        HalSMInteger_clear(memory,e);
        HalSMInteger_MulSelf(memory,a,a);
    }
    return r;
}

HalSMInteger* HalSMInteger_SquareRoot(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* k)
{
    if (HalSMInteger_isLess(a,&HalSMIntegerOne)) {
        return a;
    }
    
    /*HalSMInteger* x0,*x1;
    x0=HalSMInteger_Div(memory,a,&HalSMIntegerTwo,NOTHALSMNULLPOINTER);
    x1=HalSMInteger_Add(memory,x0,HalSMInteger_Div(memory,a,x0,NOTHALSMNULLPOINTER));
    HalSMInteger_DivSelf(memory,x1,&HalSMIntegerTwo,NOTHALSMNULLPOINTER);
    while (HalSMInteger_isLess(x1,x0)) {
        x0=x1;
        x1=HalSMInteger_Add(memory,x0,HalSMInteger_Div(memory,a,x0,NOTHALSMNULLPOINTER));
        HalSMInteger_DivSelf(memory,x1,&HalSMIntegerTwo,NOTHALSMNULLPOINTER);
    }
    return x0;*/
    HalSMInteger* u,*s,*t,*d;
    u=HalSMInteger_copy(memory,a);
    s=HalSMInteger_Add(memory,u,&HalSMIntegerOne);
    while (HalSMInteger_isLess(u,s)) {
        s=u;
        d=HalSMInteger_Sub(memory,k,&HalSMIntegerOne);
        t=HalSMInteger_Add(memory,HalSMInteger_Mul(memory,d,s),HalSMInteger_Div(memory,a,HalSMInteger_Pow(memory,s,d),NOTHALSMNULLPOINTER));
        u=HalSMInteger_Div(memory,t,k,NOTHALSMNULLPOINTER);
    }
    return s;
}

HalSMDouble* HalSMDouble_toDesyatki(HalSMMemory* memory,HalSMDouble* a)
{
    while (HalSMDouble_isMore(memory,a,&HalSMDoubleOne)) {
        HalSMDouble_DivSelf(memory,a,&HalSMDoubleTen);
    }
    return a;
}

/*HalSMDouble* stepdsr(HalSMMemory* memory,HalSMDouble* x,HalSMDouble* a,HalSMDouble* v,HalSMInteger* z,HalSMDouble* c)
{
    HalSMDouble* p,*d,*m,*e,*l;
    p=HalSMDouble_Pow(memory,x,z);
    HalSMDouble_CropSelf(memory,p,10);
    d=HalSMDouble_Div(memory,a,p);
    HalSMDouble_CropSelf(memory,d,10);
    HalSMDouble_Free(memory,p);
    m=HalSMDouble_Mul(memory,v,x);
    HalSMDouble_CropSelf(memory,m,10);
    e=HalSMDouble_Add(memory,m,d);
    HalSMDouble_CropSelf(memory,e,10);
    HalSMDouble_Free(memory,d);
    HalSMDouble_Free(memory,m);
    l=HalSMDouble_Mul(memory,c,e);
    HalSMDouble_CropSelf(memory,l,10);
    HalSMDouble_Free(memory,e);
    return l;
}*/

void HalSMDouble_SquareRootSelf(HalSMMemory* memory,HalSMDouble* x,HalSMInteger* n,unsigned long long int crop)
{
    //HalSquareRootFindAlgorithm
    //root=HSRFA(isqrt(x),x-isqrt(x))
    //HALWARSING ROOT DIVIDER = 3
    //1. Find integer square root of x = iroot
    //2. a=x - iroot^2
    //3. b=a/30
    //4. c=subisqrt
    //5. while precision
    //6.  c=c+b
    //7.  a=x-(iroot+c)^2
    //8.  b=a/3
    //9. return iroot+c
    /*HalSMInteger remainder;
    HalSMInteger* iroot=HalSMInteger_SquareRoot(memory,HalSMInteger_Div(memory,x->a,x->b,&remainder));
    HalSMDouble* a=HalSMDouble_Sub(memory,x,HalSMDouble_FromHalSMInteger(memory,HalSMInteger_Mul(memory,iroot,iroot)));
    HalSMDouble* b=HalSMDouble_Div(memory,a,HalSMDouble_parse(memory,U8H("3")));
    HalSMDouble* c=HalSMDouble_FromUnsignedInteger(memory,0);
    HalSMDouble* root=HalSMDouble_FromHalSMInteger(memory,iroot);
    HalSMDouble* temp;
    for (int i=0;i<30;i++) {
        HalSMDouble_AddSelf(memory,c,b);
        HalSMDouble_Free(memory,b);
        temp=HalSMDouble_Add(memory,root,HalSMDouble_toDesyatki(memory,HalSMDouble_copy(memory,c)));
        a=HalSMDouble_Sub(memory,x,HalSMDouble_Mul(memory,temp,temp));
        HalSMDouble_Free(memory,temp);
        b=HalSMDouble_Div(memory,a,&HalSMDoubleThree);
        HalSMDouble_Free(memory,a);
    }
    *x=*HalSMDouble_Add(memory,root,HalSMDouble_toDesyatki(memory,HalSMDouble_copy(memory,c)));*/
    /*HalSMInteger* na,*nb,*qa,*qb,*da;
    da=HalSMInteger_Div(memory,a->a,a->b,NOTHALSMNULLPOINTER);
    HalSMInteger* i=HalSMInteger_copy(memory,q);
    na=HalSMInteger_copy(memory,da);
    while (HalSMInteger_isMore(i,&HalSMIntegerOne)) {
        na=HalSMInteger_SquareRoot(memory,na);
        HalSMInteger_SubSelf(memory,i,&HalSMIntegerOne);
    }
    nb=HalSMInteger_Add(memory,na,&HalSMIntegerOne);
    qa=HalSMInteger_copy(memory,na);
    qb=HalSMInteger_copy(memory,nb);
    i=HalSMInteger_copy(memory,q);
    while (HalSMInteger_isMore(i,&HalSMIntegerOne)) {
        HalSMInteger_MulSelf(memory,qa,qa);
        HalSMInteger_MulSelf(memory,qb,qb);
        HalSMInteger_SubSelf(memory,i,&HalSMIntegerOne);
    }
        printf("G: %s\n",HU8(HalSMInteger_toString(memory,da,0)));
        printf("G: %s\n",HU8(HalSMInteger_toString(memory,qa,0)));
        printf("G: %s\n",HU8(HalSMInteger_toString(memory,qb,0)));
    if (HalSMInteger_isLess(HalSMInteger_Sub(memory,da,qa),HalSMInteger_absolute(HalSMInteger_Sub(memory,qb,da)))) {
        //A
        HalSMDouble_SubSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,qa));
        HalSMDouble_DivSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,HalSMInteger_Mul(memory,na,&HalSMIntegerTwo)));
        HalSMDouble_AddSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,na));
    } else {
        //B
        HalSMDouble_SubSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,qb));
        HalSMDouble_DivSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,HalSMInteger_Mul(memory,nb,&HalSMIntegerTwo)));
        HalSMDouble_SubSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,nb));
        a->negative=0;
    }*/

    /*HalSMInteger* na,*nb,*qa,*qb,*da;
    da=HalSMInteger_Div(memory,a->a,a->b,NOTHALSMNULLPOINTER);
    na=HalSMInteger_SquareRoot(memory,da);
    nb=HalSMInteger_Add(memory,na,&HalSMIntegerOne);
    qa=HalSMInteger_Mul(memory,na,na);
    qb=HalSMInteger_Mul(memory,nb,nb);
    if (HalSMInteger_isLess(HalSMInteger_Sub(memory,da,qa),HalSMInteger_absolute(HalSMInteger_Sub(memory,qb,da)))) {
        //A
        HalSMDouble_SubSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,qa));
        HalSMDouble_DivSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,HalSMInteger_Mul(memory,na,&HalSMIntegerTwo)));
        HalSMDouble_AddSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,na));
    } else {
        //B
        HalSMDouble_SubSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,qb));
        HalSMDouble_DivSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,HalSMInteger_Mul(memory,nb,&HalSMIntegerTwo)));
        HalSMDouble_SubSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,nb));
        a->negative=0;
    }*/

    /*HalSMInteger* na,*nb,*qa,*qb,*da;
    unsigned char isNegative=0;
    if (a->negative){isNegative=1;a->negative=0;}
    da=HalSMInteger_Div(memory,a->a,a->b,NOTHALSMNULLPOINTER);
    na=HalSMInteger_copy(memory,da);
    na=HalSMInteger_SquareRoot(memory,na,q);
    nb=HalSMInteger_Add(memory,na,&HalSMIntegerOne);
    qa=HalSMInteger_Pow(memory,na,q);
    qb=HalSMInteger_Pow(memory,nb,q);

    if (HalSMInteger_isLess(HalSMInteger_Sub(memory,da,qa),HalSMInteger_absolute(HalSMInteger_Sub(memory,qb,da)))) {
        //A
        HalSMDouble_SubSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,qa));
        HalSMDouble_DivSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,HalSMInteger_Mul(memory,q,HalSMInteger_Pow(memory,na,HalSMInteger_Sub(memory,q,&HalSMIntegerOne)))));
        HalSMDouble_AddSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,na));
    } else {
        //B
        HalSMDouble_SubSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,qb));
        HalSMDouble_DivSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,HalSMInteger_Mul(memory,q,HalSMInteger_Pow(memory,nb,HalSMInteger_Sub(memory,q,&HalSMIntegerOne)))));
        HalSMDouble_AddSelf(memory,a,HalSMDouble_FromHalSMInteger(memory,nb));
        a->negative=0;
    }
    HalSMInteger rem;
    HalSMInteger_Div(memory,q,&HalSMIntegerTwo,&rem);
    if (isNegative&&HalSMInteger_isEqual(&rem,&HalSMIntegerOne)) {
        a->negative=1;
    }*/
    /*unsigned long long int i;
    HalSMDouble* x=HalSMDouble_copy(memory,&HalSMDoubleOne),*g,*v,*c;
    HalSMInteger* z;
    g=HalSMDouble_FromHalSMInteger(memory,n);
    v=HalSMDouble_Sub(memory,g,&HalSMDoubleOne);
    c=HalSMDouble_Div(memory,&HalSMDoubleOne,g);
    z=HalSMInteger_Sub(memory,n,&HalSMIntegerOne);
    for (i=0;i<200;i++) {
        x=stepdsr(memory,x,a,v,z,c);
    }
    a->a=x->a;
    a->b=x->b;*/
    HalSMDouble* high,*low,*mid,*p;
    high=HalSMDouble_copy(memory,&HalSMDoubleOne);
    while (HalSMDouble_isMore(memory,HalSMDouble_Pow(memory,high,n),x)==0) {
        HalSMInteger_MulSelf(memory,high->a,&HalSMIntegerTwo);
    }
    low=HalSMDouble_Div(memory,high,&HalSMDoubleTwo);
    while (HalSMDouble_isLess(memory,low,high)) {
        mid=HalSMDouble_Div(memory,HalSMDouble_Add(memory,low,high),&HalSMDoubleTwo);
        HalSMDouble_CropSelf(memory,mid,crop);
        p=HalSMDouble_Pow(memory,mid,n);
        HalSMDouble_CropSelf(memory,p,crop);
        if (HalSMDouble_isLess(memory,low,mid)&&HalSMDouble_isLess(memory,p,x)) {
            low=mid;
        } else if (HalSMDouble_isMore(memory,high,mid)&&HalSMDouble_isMore(memory,p,x)) {
            high=mid;
        } else {
            *x=*mid;
            return;
        }
    }
    *x=*mid;
    HalSMDouble_AddSelf(memory,x,&HalSMDoubleOne);
}

HalSMDouble* HalSMDouble_SquareRoot(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* q,unsigned long long int crop)
{
    HalSMDouble* out=HalSMDouble_copy(memory,a);
    HalSMDouble_SquareRootSelf(memory,out,q,crop);
    return out;
}

void HalSMDouble_AbsSelf(HalSMMemory* memory,HalSMDouble* a)
{
    a->negative=0;
}

HalSMDouble* HalSMDouble_Abs(HalSMMemory* memory,HalSMDouble* a)
{
    HalSMDouble* out=HalSMDouble_copy(memory,a);
    HalSMDouble_AbsSelf(memory,out);
    return out;
}

void HalSMDouble_Free(HalSMMemory* memory,HalSMDouble* a)
{
    HalSMMemory_free(memory,a->a->value);
    HalSMMemory_free(memory,a->a);
    HalSMMemory_free(memory,a->b->value);
    HalSMMemory_free(memory,a->b);
    HalSMMemory_free(memory,a);
}

void HalSMDouble_PowSelf(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* b)
{
    unsigned char isNegative=0;
    if (b->negative==1) {
        isNegative=1;
        b->negative=0;
    }
    HalSMInteger_PowSelf(memory,a->a,b);
    HalSMInteger_PowSelf(memory,a->b,b);
    
    HalSMInteger remainder;
    if (a->negative) {
        HalSMInteger_Div(memory,b,&HalSMIntegerTwo,&remainder);
        if (HalSMInteger_isEqual(&remainder,&HalSMIntegerZero)) {
            a->negative=0;
        }
    }
    if (isNegative) {
        b->negative=1;
        HalSMDouble* temp=HalSMDouble_Div(memory,&HalSMDoubleOne,a);
        a->a=temp->a;
        a->negative=temp->negative;
        a->b=temp->b;
    }
}

HalSMDouble* HalSMDouble_Pow(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* b)
{
    HalSMDouble* out=HalSMDouble_copy(memory,a);
    HalSMDouble_PowSelf(memory,out,b);
    return out;
}

void HalSMDouble_RoundSelf(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* b)
{
    
}

HalSMDouble* HalSMDouble_Round(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* b)
{
    HalSMDouble* out=HalSMDouble_copy(memory,a);
    HalSMDouble_RoundSelf(memory,out,b);
    return out;
}

void HalSMDouble_CropSelf(HalSMMemory* memory,HalSMDouble* a,unsigned long long int crop)
{
    HalStringFormatChar* s=HalSMDouble_toString(memory,a,0);
    unsigned long long int index=StringIndexOf(memory,s,U8H(".")),l,m;
    l=string_len(s);
    m=l-index-1;
    if (m>crop) {
        s[index+crop+1]=ASCIIH('\0');
        HalSMDouble* out=HalSMDouble_parse(memory,s);
        a->a=out->a;
        a->b=out->b;
    }
    HalSMMemory_free(memory,s);
}

HalSMInteger* HalSMDouble_gcdSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    if (HalSMInteger_isEqual(a,&HalSMIntegerZero)) {
        return b;
    }
    if (HalSMInteger_isEqual(b,&HalSMIntegerZero)) {
        return a;
    }

    HalSMInteger* t;
    unsigned long long int k;
    for (k=0;HalSMInteger_isEqual(HalSMInteger_BitAND(memory,HalSMInteger_BitOR(memory,a,b),&HalSMIntegerOne),&HalSMIntegerZero);k++) {
        HalSMInteger_shiftRightSelf(memory,a,1);
        HalSMInteger_shiftRightSelf(memory,b,1);
    }

    while (HalSMInteger_isEqual(HalSMInteger_BitAND(memory,a,&HalSMIntegerOne),&HalSMIntegerZero)) {
        HalSMInteger_shiftRightSelf(memory,a,1);
    }

    do {
        while (HalSMInteger_isEqual(HalSMInteger_BitAND(memory,b,&HalSMIntegerOne),&HalSMIntegerZero)) {
            HalSMInteger_shiftRightSelf(memory,b,1);
        }

        if (HalSMInteger_isMore(a,b)) {
            t=a;
            a=b;
            b=t;
        }
        HalSMInteger_SubSelf(memory,b,a);
    } while (HalSMInteger_isEqual(b,&HalSMIntegerZero)==0);
    HalSMInteger_shiftLeftSelf(memory,a,k);
    return a;
}

HalSMInteger* HalSMDouble_gcd(HalSMMemory* memory,HalSMInteger* u,HalSMInteger* v)
{
    return HalSMDouble_gcdSelf(memory,HalSMInteger_copy(memory,u),HalSMInteger_copy(memory,v));
}

void HalSMDouble_clearSelf(HalSMMemory* memory,HalSMDouble* a)
{
    //return;
    HalSMInteger* gcd=HalSMDouble_gcd(memory,a->a,a->b);
    if (HalSMInteger_isEqual(gcd,&HalSMIntegerZero)){return;}
    HalSMInteger_DivSelf(memory,a->a,gcd,NOTHALSMNULLPOINTER);
    HalSMInteger_DivSelf(memory,a->b,gcd,NOTHALSMNULLPOINTER);
}

HalSMDouble* HalSMDouble_clear(HalSMMemory* memory,HalSMDouble* a)
{
    //return a;
    HalSMDouble* out=HalSMDouble_copy(memory,a);
    HalSMDouble_clearSelf(memory,out);
    return out;
}

//HalSMDouble