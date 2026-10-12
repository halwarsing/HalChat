#include <HalSM.h>
//HalSMInteger

HalSMInteger* HalSMInteger_init(HalSMMemory* memory,unsigned char negative,unsigned char* value,unsigned long long size)
{
    HalSMInteger* out=HalSMMemory_malloc(memory,sizeof(HalSMInteger));
    out->negative=negative;
    out->value=value;
    out->size=size;
    return out;
}

HalSMInteger* HalSMInteger_copy(HalSMMemory* memory,HalSMInteger* a)
{
    HalSMInteger* out=HalSMMemory_malloc(memory,sizeof(HalSMInteger));
    out->negative=a->negative;
    out->value=HalSMMemory_malloc(memory,a->size);
    memory_cpy(out->value,a->value,a->size);
    out->size=a->size;
    return out;
}

HalSMInteger* HalSMInteger_FromUnsignedInteger(HalSMMemory* memory,unsigned int value)
{
    unsigned char* v=HalSMMemory_malloc(memory,4);
    v[3]=(value>>24)&0xFF;
    v[2]=(value>>16)&0xFF;
    v[1]=(value>>8)&0xFF;
    v[0]=value&0xFF;

    return HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v,4));
}

HalSMInteger* HalSMInteger_FromSignedInteger(HalSMMemory* memory,signed int value)
{
    unsigned char* v=HalSMMemory_malloc(memory,4);
    unsigned char negate=value<0?1:0;
    if (negate) {
        value=-value;
    }
    v[3]=(value>>24)&0xFF;
    v[2]=(value>>16)&0xFF;
    v[1]=(value>>8)&0xFF;
    v[0]=value&0xFF;
    return HalSMInteger_clear(memory,HalSMInteger_init(memory,negate,v,4));
}

HalSMInteger* HalSMInteger_FromUnsignedLongLongInteger(HalSMMemory* memory,unsigned long long int value)
{
    unsigned char* v=HalSMMemory_malloc(memory,8);
    v[7]=(value>>56)&0xFF;
    v[6]=(value>>48)&0xFF;
    v[5]=(value>>40)&0xFF;
    v[4]=(value>>32)&0xFF;
    v[3]=(value>>24)&0xFF;
    v[2]=(value>>16)&0xFF;
    v[1]=(value>>8)&0xFF;
    v[0]=value&0xFF;
    return HalSMInteger_clear(memory,HalSMInteger_init(memory,0,v,8));
}

unsigned int HalSMInteger_ToUnsignedInteger(HalSMInteger* value)
{
    unsigned char out[4]={0,0,0,0};
    unsigned long long int l=value->size>4?4:value->size;
    for (unsigned long long int i=0;i<l;i++) {
        out[i]=value->value[i];
    }
    return *(unsigned int*)out;
}

unsigned long long int HalSMInteger_ToUnsignedLongLongInteger(HalSMInteger* value)
{
    unsigned char out[8]={0,0,0,0};
    unsigned long long int l=value->size>8?8:value->size;
    for (unsigned long long int i=0;i<l;i++) {
        out[i]=value->value[i];
    }
    return *(unsigned long long int*)out;
}

HalSMInteger* HalSMInteger_parse(HalSMMemory* memory,HalStringFormatChar* str)
{
    HalSMInteger* out=HalSMInteger_init(memory,0,HalSMMemory_malloc(memory,0),0);
    unsigned char neg = HSFCharCompare(str[0],ASCIIH('-'));
    long long int i = neg ? 1 : 0;
    HalSMInteger t={0,(unsigned char[]){0},1};
    while (str[i].symbol>='0'&&str[i].symbol<='9')
    {
        HalSMInteger_MulSelf(memory,out,&HalSMIntegerTen);
        t.value[0]=str[i].symbol-'0';
        HalSMInteger_AddSelf(memory,out,&t);  
        i++;
    }
    if ( neg ) {
        out->negative=1;
    }
    return HalSMInteger_clear(memory,out);
}

HalSMInteger* HalSMInteger_parseHex(HalSMMemory* memory,HalStringFormatChar* str,unsigned char isZeroX)
{
    unsigned long long int l=string_len(str);
    if (l<3){return &HalSMIntegerZero;}
    unsigned char neg=HSFCharCompare(str[0],ASCIIH('-'));
    unsigned long long int i,ind;
    unsigned char v=0;
    HalStringFormatChar* p=str;
    if (neg){p++;l--;}
    if (isZeroX&&(HSFCharCompare(p[0],ASCIIH('0'))==0||HSFCharCompare(p[1],ASCIIH('x'))==0)){return &HalSMIntegerZero;}
    if (isZeroX){p+=2;l-=2;}
    HalSMInteger* out=HalSMInteger_init(memory,0,HalSMMemory_malloc(memory,(l+1)/2),(l+1)/2);
    i=0;
    ind=out->size-1;
    if (l%2==1) {
        if (p->symbol>=65&&p->symbol<=70) {
            v=p->symbol-65+10;
        } else if (p->symbol>='0'&&p->symbol<='9') {
            v=p->symbol-'0';
        }
        out->value[ind-i/2]=v;
        p++;
        i=2;
        l++;
        ind-=1;
    }
    for (;i<l;i++,p++) {
        if (p->symbol>=65&&p->symbol<=70) {
            v=p->symbol-65+10;
        } else if (p->symbol>='0'&&p->symbol<='9') {
            v=p->symbol-'0';
        }
        if (i%2==1) {
            out->value[ind-i/2]|=v;
        } else {
            out->value[ind-i/2]=v<<4;
        }
    }
    out->negative=neg;
    return HalSMInteger_clear(memory,out);
}

HalSMInteger* HalSMInteger_parseBin(HalSMMemory* memory,HalStringFormatChar* str)
{
    unsigned long long int ind;
    unsigned long long int l=string_len(str);
    if (l<3){return &HalSMIntegerZero;}
    unsigned char neg=HSFCharCompare(str[0],ASCIIH('-'));
    unsigned long long int i;
    unsigned char v=0;
    HalStringFormatChar* p=str;
    if (neg){p++;l--;}
    if (HSFCharCompare(p[0],ASCIIH('0'))==0||HSFCharCompare(p[1],ASCIIH('b'))==0){return &HalSMIntegerZero;}
    p+=2;
    l-=2;
    HalSMInteger* out=HalSMInteger_init(memory,0,HalSMMemory_calloc(memory,(l+7)/8,sizeof(unsigned char)),(l+7)/8);
    i=0;
    ind=out->size-1;
    if (l%8>0) {
        for (;i<l%8;i++,p++) {
            v=p->symbol-'0';
            out->value[ind]|=v<<((l%8)-1-i);
        }
        //l++;
        l=l-i;
        i=0;
        ind-=1;
    }
    for (;i<l;i++,p++) {
        v=p->symbol-'0';
        out->value[ind-i/8]|=v<<(7-(i%8));
    }
    out->negative=neg;
    return HalSMInteger_clear(memory,out);
}

void HalSMInteger_AddSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    if (a->negative&&b->negative==0) {
        a->negative=0;
        HalSMInteger_SubSelf(memory,a,b);
        if (a->negative) {
            a->negative=0;
        } else {
            a->negative=1;
        }
        HalSMInteger_clear(memory,a);
        return;
    } else if (b->negative&&a->negative==0) {
        b->negative=0;
        HalSMInteger_SubSelf(memory,a,b);
        b->negative=1;
        HalSMInteger_clear(memory,a);
        return;
    }
    unsigned long long int i,j;
    unsigned short tempr;
    unsigned char tempn=0,tempb;
    unsigned char* temps;
    unsigned long long int cm=a->size;
    if (a->size<b->size) {
        a->value=HalSMMemory_realloc(memory,a->value,b->size);
        for(i=a->size;i<b->size;i++) {
            a->value[i]=0;
        }
        a->size=b->size;
        cm=b->size;
    }
    /*for (i=a->size-1,j=b->size,c=0;c<cm;i--,c++) {
        if (j==0) {
            tempb=0;
        } else {
            j--;
            tempb=b->value[j];
        }
        tempr=a->value[i]+tempb+tempn;
        temps=(unsigned char*)&tempr;
        a->value[i]=temps[0];
        tempn=temps[1];
    }*/

    for(i=0,j=0;i<cm;i++) {
        if(j==b->size) {
            tempb=0;
        } else {
            tempb=b->value[j];
            j++;
        }
        tempr=a->value[i]+tempb+tempn;
        temps=(unsigned char*)&tempr;
        a->value[i]=temps[0];
        tempn=temps[1];
    }

    if (tempn!=0) {
        a->value=HalSMMemory_realloc(memory,a->value,a->size+1);
        /*for (i=a->size-1;;i--) {
            a->value[i+1]=a->value[i];
            if (i==0) {break;}
        }*/
        a->value[a->size]=tempn;
        a->size=a->size+1;
    }
    HalSMInteger_clear(memory,a);
}

HalSMInteger* HalSMInteger_Add(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    unsigned char* temp=HalSMMemory_malloc(memory,a->size);
    memory_cpy(temp,a->value,a->size);
    HalSMInteger* out=HalSMInteger_init(memory,a->negative,temp,a->size);
    HalSMInteger_AddSelf(memory,out,b);
    return out;
}

void HalSMInteger_SubSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    // 0. Если a == b, то результат ноль
    if (HalSMInteger_isEqual(a, b)) {
        HalSMMemory_free(memory,a->value);
        a->value = HalSMMemory_calloc(memory,1, sizeof(unsigned char));  // число "0"
        a->size = 1;
        a->negative = 0;
        return;
    }
    // 1. Обработка знаков 
    //    (+a) - (-b) = a + b
    //    (-a) - (+b) = -(a + b)
    //    Если оба отрицательные: (-a) - (-b) = (-(a - b)).
    //    Тут нужны ветви, подобные вашему коду. Для упрощения я пропущу нюансы.

    // Допустим, мы дошли до «оба неотрицательные, a >= b».
    // => Вычитаем b из a напрямую

    // Смотрим, если a < b (по модулю), результат будет отрицательный
    if (HalSMInteger_isLess(a, b)) {
        // Результат = b - a, со знаком «-»
        HalSMInteger* out = HalSMInteger_Sub(memory, b, a); 
        // Теперь out содержит b-a; нужно перенести out->value в a
        HalSMMemory_free(memory,a->value);
        a->value = out->value;
        a->size  = out->size;
        a->negative = 1; 
        return;
    }

    // 2. Вычитаем поразрядно
    unsigned char borrow = 0;
    for (unsigned long long i = 0; i < a->size; i++) {
        unsigned short av = a->value[i];
        unsigned short bv = (i < b->size) ? b->value[i] : 0;
        unsigned short diff = av - bv - borrow;
        if (diff > av) {
            // был переход через 0
            borrow = 1;
            diff = (unsigned short)(diff + 256); // base=256
        } else {
            borrow = 0;
        }
        a->value[i] = (unsigned char)diff;
    }
    // Если в конце borrow остался 1, значит |b| > |a|, но мы этот случай уже учли логикой a<b выше.

    // 3. Удаляем ведущие нули
    HalSMInteger_clear(memory, a);
}

HalSMInteger* HalSMInteger_Sub(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    unsigned char* temp=HalSMMemory_malloc(memory,a->size);
    memory_cpy(temp,a->value,a->size);
    HalSMInteger* out=HalSMInteger_init(memory,a->negative,temp,a->size);
    HalSMInteger_SubSelf(memory,out,b);
    return out;
}

void HalSMInteger_MulSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{   
    if (a->negative==1&&b->negative==1) {a->negative=0;}
    else if (b->negative==1) {a->negative=1;}
    unsigned int* product=HalSMMemory_calloc(memory,a->size+b->size,sizeof(unsigned int));
    int base;
    base=256;
    for (unsigned long long i = 0; i < a->size; i++) {
        unsigned long long carry = 0ULL;
        for (unsigned long long j = 0; j < b->size || carry; j++) {
            unsigned long long cur = product[i + j] + (unsigned long long)a->value[i] * (j < b->size ? b->value[j] : 0ULL) + carry;
            product[i + j] = (unsigned int)(cur % base);
            carry = cur / base;
        }
    }
    a->size+=b->size;
    a->value=HalSMMemory_realloc(memory,a->value,a->size);
    for (unsigned long long int i=0;i<a->size;i++) {
        a->value[i]=product[i];
    }
    HalSMMemory_free(memory,product);
    HalSMInteger_clear(memory,a);
}

HalSMInteger* HalSMInteger_Mul(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    HalSMInteger* out=HalSMInteger_copy(memory,a);
    HalSMInteger_MulSelf(memory,out,b);
    return out;
}

signed char HalSMInteger_compare(HalSMInteger* a,HalSMInteger* b) {
    if(HalSMInteger_isMore(a,b))return 1;
    if(HalSMInteger_isLess(a,b))return -1;
    return 0;
}

void HalSMInteger_DivSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b,HalSMInteger* remainder)
{
    unsigned char isBNegative=b->negative;
    unsigned char isANegative=a->negative;
    b->negative=0;
    a->negative=0;
    long long int i;
    HalSMInteger* R=HalSMInteger_init(memory,0,HalSMMemory_calloc(memory,1,1),1);
    HalSMInteger* Q=HalSMInteger_init(memory,0,HalSMMemory_calloc(memory,1,1),1);

    unsigned long long z=a->size*8;
    for (i=0;i<z;i++) {
        // "Сдвигаем" R влево на 1 бит
        HalSMInteger_shiftLeftSelf(memory, R, 1);

        // Считываем очередной "старший" бит из a, учитывая little-endian хранение
        // В LE старший байт = a->value[a->size - 1], 
        // а бит (7 - (i%8)) внутри него
        unsigned long long byteIndex = (a->size - 1) - (i / 8);
        unsigned char bitIndex = 7 - (i % 8);
        unsigned char curBit = HalSMInteger_getBit(a, byteIndex, bitIndex);

        HalSMInteger_setBitSelf(memory, R, 0, 0, curBit);

        // Если R >= b, то вычитаем b и ставим бит в Q
        if (!HalSMInteger_isLess(R, b)) {
            HalSMInteger_SubSelf(memory, R, b);
            HalSMInteger_setBitSelf(memory, Q, byteIndex, bitIndex, 1);
        }
        //if (i == 0) break;
    }

    if (remainder!=NOTHALSMNULLPOINTER){
        remainder->negative=R->negative;
        remainder->size=R->size;
        remainder->value=R->value;
        HalSMInteger_clear(memory,remainder);
    }
    a->size=Q->size;
    HalSMMemory_free(memory,a->value);
    a->value=Q->value;
    if ((isANegative==0&&isBNegative)||(isBNegative==0&&isANegative)) {
        a->negative=1;
    }
    HalSMInteger_clear(memory,a);
}

HalSMInteger* HalSMInteger_Div(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b,HalSMInteger* remainder)
{
    HalSMInteger* out=HalSMInteger_copy(memory,a);
    HalSMInteger_DivSelf(memory,out,b,remainder);
    return out;
}

HalSMInteger* HalSMInteger_negate(HalSMInteger* a)
{
    a->negative=a->negative==1?0:1;
    return a;
}

HalSMInteger* HalSMInteger_absolute(HalSMInteger* a)
{
    a->negative=0;
    return a;
}

unsigned char HalSMInteger_isMore(HalSMInteger* a,HalSMInteger* b)
{
    if (a->size>b->size||(a->negative==0&&b->negative==1)){return 1;}
    else if (a->size<b->size||(a->size==1&&b->size==1&&a->value[0]==b->value[0])||(a->negative==1&&b->negative==0)) {return 0;}
    unsigned long long i;
    if (a->negative==1) {
        for (i=a->size-1;;i--) {
            if (a->value[i]>b->value[i]) {return 0;}
            else if (a->value[i]<b->value[i]) {return 1;}
            if(i==0)break;
        }
    } else {
        for (i=a->size-1;;i--) {
            if (a->value[i]<b->value[i]) {return 0;}
            else if (a->value[i]>b->value[i]) {return 1;}
            if(i==0)break;
        }
    }
    return 0;
}

unsigned char HalSMInteger_isLess(HalSMInteger* a,HalSMInteger* b)
{
    if (a->size<b->size||(a->negative==1&&b->negative==0)){return 1;}
    else if (a->size>b->size||(a->size==1&&b->size==1&&a->value[0]==b->value[0])||(a->negative==0&&b->negative==1)) {return 0;}
    unsigned long long i;
    if (a->negative==1) {
        for (i=a->size-1;;i--) {
            if (a->value[i]<b->value[i]) {return 0;}
            else if (a->value[i]>b->value[i]) {return 1;}
            if(i==0)break;
        }
    } else {
        for (i=a->size-1;;i--) {
            if (a->value[i]>b->value[i]) {return 0;}
            else if (a->value[i]<b->value[i]) {return 1;}
            if(i==0)break;
        }
    }
    return 0;
}

unsigned char HalSMInteger_isEqual(HalSMInteger* a,HalSMInteger* b)
{
    if (a->size!=b->size||a->negative!=b->negative) {return 0;}
    unsigned long long i;
    for (i=0;i<a->size;i++) {
        if (a->value[i]!=b->value[i]) {return 0;}
    }
    return 1;
}

HalSMInteger* HalSMInteger_getValueWithoutNull(HalSMMemory* memory,HalSMInteger* a)
{
    if (a->size==0) {return a;}
    else if (a->size==1) {return a;}
    
    unsigned long long int i,cn;
    cn=0;
    for (i=0;i<a->size;i++) {
        if (a->value[i]==0)cn++;
        else break;
    }
    if (cn==0)return a;
    if (a->size-cn==0) {
        a->size=1;
    } else {
        a->size=a->size-cn;
    }
    memory_cpy(a->value,&a->value[cn],a->size);
    a->value=HalSMMemory_realloc(memory,a->value,a->size);
    return a;
}

char* HalSMInteger_Byte2Bits(HalSMMemory* memory,unsigned char byte)
{
    char* out=HalSMMemory_malloc(memory,9);
    out[0]=(byte>>7&1)+'0';
    out[1]=((byte>>6)&1)+'0';
    out[2]=((byte>>5)&1)+'0';
    out[3]=((byte>>4)&1)+'0';
    out[4]=((byte>>3)&1)+'0';
    out[5]=((byte>>2)&1)+'0';
    out[6]=((byte>>1)&1)+'0';
    out[7]=(byte&1)+'0';
    out[8]='\0';
    return out;
}

HalStringFormatChar* HalSMInteger_Bytes2Bits(HalSMMemory* memory,unsigned char* bytes,unsigned long long size)
{
    if (size==0) {return U8H("");}
    else if (size==1) {return U8H(HalSMInteger_Byte2Bits(memory,bytes[0]));}
    HalStringFormatChar* out=HalSMMemory_malloc(memory,(size*8+1)*sizeof(HalStringFormatChar));
    string_cpy(out,U8H(HalSMInteger_Byte2Bits(memory,bytes[0])));
    for (unsigned long long i=1;i<size;i++) {
        string_cat(out,U8H(HalSMInteger_Byte2Bits(memory,bytes[i])));
    }
    out[size*8].alphabet=0;
    out[size*8].symbol=0;
    out[size*8].parameters=0;
    return out;
}

HalStringFormatChar* SumStringValues(HalSMMemory* memory,HalStringFormatChar* s1,HalStringFormatChar* s2)
{
    char* result=HalSMMemory_malloc(memory,0);
    unsigned long long resultl=0;
    HalStringFormatChar* str1=s1,*str2=s2;
    if (string_len(str1)>string_len(str2)) {
        HalStringFormatChar* temp=str1;
        str1=str2;
        str2=temp;
    }

    while (string_len(str1)<string_len(str2)) {
        str1=ConcatenateStrings(memory,U8H("0"),str1);
    }
    unsigned long long len=string_len(str1);
    unsigned char carry=0;
    while (len)
    {
        len--;
        long long int d1=str1[len].symbol-'0';
        long long int d2=str2[len].symbol-'0';

        long long int sum=d1+d2+carry;
        carry=sum>9;
        if (carry)
        {
            sum-=10;
        }
        result=HalSMMemory_realloc(memory,result,++resultl);
        result[resultl-1]='0'+sum;
    }

    if (carry)
    {
        result=HalSMMemory_realloc(memory,result,++resultl);
        result[resultl-1]='1';
    }

    result=HalSMMemory_realloc(memory,result,resultl+1);
    result[resultl]='\0';

    unsigned long long i;
    char* out=HalSMMemory_malloc(memory,resultl+1);
    for (i=0;i<resultl;i++) {
        out[resultl-1-i]=result[i];
    }
    out[resultl]='\0';
    return U8H(out);
}

HalStringFormatChar* ShiftLeftString(HalSMMemory* memory,HalStringFormatChar* s,unsigned long long N)
{
    HalStringFormatChar* result=s;
    while (N>0) {
        result=SumStringValues(memory,result,result);
        N--;
    }
    return result;
}

HalStringFormatChar* byteToString(HalSMMemory* memory,unsigned char byte) {
    char* out;
    if (byte>99) {
        out=HalSMMemory_calloc(memory,4,sizeof(char));
        out[0]='0'+(byte/100);
        out[1]='0'+((byte%100)/10);
        out[2]='0'+(byte%10);
        return U8H(out);
    } else if (byte>9) {
        out=HalSMMemory_calloc(memory,3,sizeof(char));
        out[0]='0'+(byte/10);
        out[1]='0'+(byte%10);
        return U8H(out);
    }
    out=HalSMMemory_calloc(memory,2,sizeof(char));
    out[0]='0'+byte;
    return U8H(out);
}

unsigned char hi_div10(unsigned char* hex,unsigned long long size) {
    unsigned char rem=0;
    unsigned char n;
    unsigned long long i;
    for (i=size-1;;i--) {
        n=rem*256+hex[i];
        hex[i]=n/10;
        rem=n%10;
        if (i==0)break;
    }
    return rem;
}

unsigned char hi_isNotNull(unsigned char* hex,unsigned long long size) {
    unsigned long long i;
    for (i=0;i<size;i++) {
        if (hex[i]>0)return 1;
    }
    return 0;
}

HalStringFormatChar* HalSMInteger_Byte2HexOne(HalStringFormatChar* out,unsigned char x)
{
    unsigned char a=(x>>4)&0xF;
    //unsigned char b=x&0xF;
    out[0]=U8CH(a>=10?'a'+a-10:'0'+a);
    //out[1]=U8CH(b>=10?'a'+b-10:'0'+b);
    return out;
}

HalStringFormatChar* HalSMInteger_toString(HalSMMemory* memory,HalSMInteger* a,unsigned char isHex)
{
    if(a->size==0){return U8H("0");}
    if (isHex==1) {
        HalStringFormatChar* out=HalSMMemory_malloc(memory,(2*a->size+1)*sizeof(HalStringFormatChar));
        unsigned long long i;
        for (i=0;i<a->size;i++) {
            HalSMInteger_Byte2Hex(&out[i*2],a->value[a->size-i-1]);
        }
        out[2*a->size]=ASCIIH('\0');
        return out;
    } else if (isHex==2) {
        HalStringFormatChar* out=HalSMMemory_calloc(memory,2*a->size+1,sizeof(HalStringFormatChar));
        unsigned long long i,l=0;
        unsigned char x=a->value[a->size-1];
        if(x<0x10) {
            out[0]=U8CH(x>=10?'a'+x-10:'0'+x);
            l=1;
        }
        for (i=l;i<a->size;i++) {
            HalSMInteger_Byte2Hex(&out[i*2-l],a->value[a->size-i-1]);
        }
        out[2*a->size]=ASCIIH('\0');
        return out;
    }

    HalStringFormatChar* hex=HalSMInteger_toString(memory,a,1);
    unsigned long long int i=0,l,d,decl=1;
    char c;
    unsigned char* dec=HalSMMemory_calloc(memory,1,sizeof(HalStringFormatChar));
    l=string_len(hex);
    if (l>0&&HSFCharCompare(hex[0],ASCIIH('0'))) {
        i=1;
    }
    
    //Make check ascii
    while (i<l) {
        c=UTF8HalStringFormatCharacters[HalUTF8_indexByHalStringFormatChar(hex[i])].utf8c;
        long long int carry=(c<='9'?c-'0':c-'a'+10);
        for (d=0;d<decl;d++) {
            long long int val=dec[d]*16+carry;
            dec[d]=val%10;
            carry=val/10;
        }

        while (carry>0) {
            dec=HalSMMemory_realloc(memory,dec,(++decl)*sizeof(HalStringFormatChar));
            dec[decl-1]=carry%10;
            carry/=10;
        }
        i++;
    }

    HalStringFormatChar* out;
    HalStringFormatChar* outp;
    if (a->negative&&(decl>1||dec[0]>0)) {
        out=HalSMMemory_malloc(memory,(decl+2)*sizeof(HalStringFormatChar));
        out[0]=ASCIIH('-');
        out[decl+1]=ASCIIH('\0');
        outp=&out[decl];
    } else {
        out=HalSMMemory_malloc(memory,(decl+1)*sizeof(HalStringFormatChar));
        out[decl]=ASCIIH('\0');
        outp=&out[decl-1];
    }
    for (i=0;i<decl;i++,outp--) {
        *outp=ASCIIH('0'+dec[i]);
    }
    return out;
}

HalStringFormatChar* HalSMInteger_Byte2Hex(HalStringFormatChar* out,unsigned char x)
{
    unsigned char a=(x>>4)&0xF;
    unsigned char b=x&0xF;
    out[0]=U8CH(a>=10?'a'+a-10:'0'+a);
    out[1]=U8CH(b>=10?'a'+b-10:'0'+b);
    return out;
}

HalStringFormatChar* HalSMInteger_toStringBytes(HalSMMemory* memory,HalSMInteger* a)
{
    unsigned long long i,curInd;
    HalStringFormatChar* out=HalSMMemory_malloc(memory,3*a->size+1);
    out[3*a->size]=ASCIIH('\0');
    HalStringFormatChar* temp=HalSMMemory_calloc(memory,2,sizeof(char));
    for (i=0;i<a->size;i++)
    {
        curInd=i*3;
        out[curInd]=ASCIIH('\\');
        temp=HalSMInteger_Byte2Hex(temp,a->value[i]);
        if (HSFCharCompare(temp[1],ASCIIH('\0'))) {
            out[curInd+1]=ASCIIH('0');
            out[curInd+2]=temp[0];
        } else {
            out[curInd+1]=temp[0];
            out[curInd+2]=temp[1];
        }
        temp[0]=ASCIIH('\0');
        temp[1]=ASCIIH('\0');
    }
    return out;
}

unsigned char HalSMInteger_getBit(HalSMInteger* a,long long int byte,unsigned char bit) {
    if (byte>=a->size){return 0;}
    return (a->value[byte]>>bit)&1;
}

void HalSMInteger_setBitSelf(HalSMMemory* memory,HalSMInteger* a,long long int byte,unsigned char bit,unsigned char value) {
    if (byte>=a->size){
        a->value=HalSMMemory_realloc(memory,a->value,byte+1);
        unsigned long long int i,j;
        /*for (i=a->size-1,j=byte;;i--,j--) {
            a->value[j]=a->value[i];
            if (i==0){break;}
        }
        j=byte+1-a->size;
        for (i=0;i<j;i++) {
            a->value[i]=0;
        }*/
        j=byte+1;
        for(i=a->size;i<j;i++) {
            a->value[i]=0;
        }
        a->size=byte+1;
    }
    //CHANGE BIT
    if (value==0) {
        a->value[byte]&=~(1<<bit);
        return;
    }
    a->value[byte]|=(1<<bit);
}

HalSMInteger* HalSMInteger_setBit(HalSMMemory* memory,HalSMInteger* a,long long int byte,unsigned char bit,unsigned char value)
{
    HalSMInteger* out=HalSMInteger_copy(memory,a);
    HalSMInteger_setBitSelf(memory,out,byte,bit,value);
    return out;
}

unsigned char reverseBits(unsigned char n)
{
    return (65280>>n)&0xff;
}

void HalSMInteger_shiftRightSelf(HalSMMemory* memory,HalSMInteger* a,long long int bit) {
    if (bit == 0) return;

    unsigned int byteShift = bit / 8; 
    unsigned int bitShift  = bit % 8;

    // 1. Сдвиг на целые байты (удаляем byteShift младших байт)
    if (byteShift > 0) {
        if (byteShift >= a->size) {
            // Число становится 0
            HalSMMemory_free(memory,a->value);
            a->value = HalSMMemory_calloc(memory,1, 1);
            a->size  = 1;
            a->value[0] = 0;
            a->negative  = 0;
            return;
        }
        // Убираем первые byteShift байт
        for (unsigned long long i = 0; i < a->size - byteShift; i++) {
            a->value[i] = a->value[i + byteShift];
        }
        a->size -= byteShift;
        a->value = HalSMMemory_realloc(memory,a->value, a->size);
    }

    if (bitShift == 0) {
        HalSMInteger_clear(memory, a);
        return;
    }

    // 2. Сдвиг на bitShift (1..7) вправо
    unsigned int carry = 0;  
    for (unsigned long long i = a->size - 1;; i--) {
        unsigned int cur = (unsigned int)a->value[i] + (carry << 8);
        unsigned int newVal = cur >> bitShift;
        carry = cur & ((1 << bitShift) - 1);
        a->value[i] = (unsigned char)newVal;
        if(i==0)break;
    }
    HalSMInteger_clear(memory, a);
}

HalSMInteger* HalSMInteger_shiftRight(HalSMMemory* memory,HalSMInteger* a,long long int bit) {
    unsigned char* temp=HalSMMemory_malloc(memory,a->size);
    memory_cpy(temp,a->value,a->size);
    HalSMInteger* out=HalSMInteger_init(memory,a->negative,temp,a->size);
    HalSMInteger_shiftRightSelf(memory,out,bit);
    return out;
}

void HalSMInteger_shiftLeftSelf(HalSMMemory* memory,HalSMInteger* a,long long int bit) {
    if (bit == 0) return;

    // 1. Сдвиг на целые байты
    unsigned int byteShift = bit / 8;   // сколько байт
    unsigned int bitShift  = bit % 8;   // остаток по битам

    // Если нужно сдвинуть массив на byteShift байтов (в little-endian — «старшие байты» добавляются в конец)
    if (byteShift > 0) {
        // Увеличим buffer на byteShift
        ui64 oldSize = a->size;
        a->value = HalSMMemory_realloc(memory,a->value, a->size + byteShift);
        // Сдвинем существующие байты вверх на byteShift
        // Копируем с конца (i: oldSize-1..0) в (i+byteShift)
        for (unsigned long long i = oldSize - 1;; i--) {
            a->value[i + byteShift] = a->value[i];
            if(i==0)break;
        }
        // Заполним новые младшие байты нулями
        for (unsigned long long i = 0; i < byteShift; i++) {
            a->value[i] = 0;
        }
        a->size += byteShift;
    }

    if (bitShift == 0) {
        // если не нужно дополнительно сдвигать битами — всё
        HalSMInteger_clear(memory, a);
        return;
    }

    // 2. Сдвиг на оставшиеся биты (1..7)
    unsigned short carry = 0;
    for (unsigned long long i = 0; i < a->size; i++) {
        unsigned short val = (a->value[i] << bitShift) | carry;
        a->value[i] = (unsigned char)(val & 0xFF);
        carry = (val >> 8) & 0xFF;
    }
    if (carry) {
        // Расширяем массив на 1 байт
        a->value = HalSMMemory_realloc(memory,a->value, a->size + 1);
        a->value[a->size] = (unsigned char)carry;
        a->size++;
    }
    HalSMInteger_clear(memory, a);
}

HalSMInteger* HalSMInteger_shiftLeft(HalSMMemory* memory,HalSMInteger* a,long long int bit) {
    unsigned char* temp=HalSMMemory_malloc(memory,a->size);
    memory_cpy(temp,a->value,a->size);
    HalSMInteger* out=HalSMInteger_init(memory,a->negative,temp,a->size);
    HalSMInteger_shiftLeftSelf(memory,out,bit);
    return out;
}

HalSMInteger* HalSMInteger_clear(HalSMMemory* memory,HalSMInteger* a)
{
    // Гарантируем, что оставляем хотя бы 1 байт (число "0")
    while (a->size > 1 && a->value[a->size - 1] == 0) {
        a->size--;
    }
    // Сжимаем буфер под новый размер
    a->value = HalSMMemory_realloc(memory,a->value, a->size * sizeof(unsigned char));
    return a;
}

void HalSMInteger_PowSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    HalSMInteger* base=HalSMInteger_copy(memory,a);
    a->value=HalSMMemory_malloc(memory,1);
    a->value[0]=1;
    a->size=1;
    HalSMInteger* exp=HalSMInteger_copy(memory,b);
    unsigned char isNegative=a->negative;
    while (hi_isNotNull(exp->value,exp->size)) {
        if (HalSMInteger_getBit(exp,0,0)){HalSMInteger_MulSelf(memory,a,base);}
        HalSMInteger_MulSelf(memory,base,base);
        HalSMInteger_shiftRightSelf(memory,exp,1);
    }
    HalSMInteger remainder;
    if (isNegative) {
        HalSMInteger_Div(memory,b,&HalSMIntegerTwo,&remainder);
        a->negative=1;
        if (HalSMInteger_isEqual(&remainder,&HalSMIntegerZero)) {
            a->negative=0;
        }
    }
}

HalSMInteger* HalSMInteger_Pow(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    unsigned char* temp=HalSMMemory_malloc(memory,a->size);
    memory_cpy(temp,a->value,a->size);
    HalSMInteger* out=HalSMInteger_init(memory,a->negative,temp,a->size);
    HalSMInteger_PowSelf(memory,out,b);
    return out;
}

void HalSMInteger_Free(HalSMMemory* memory,HalSMInteger* a)
{
    HalSMMemory_free(memory,a->value);
    HalSMMemory_free(memory,a);
}

unsigned char HalSMInteger_ByteCTZ(unsigned char byte) {
    if (((byte>>7)&1)==1) {
        return 0;
    } else if (((byte>>6)&1)==1) {
        return 1;
    } else if (((byte>>5)&1)==1) {
        return 2;
    } else if (((byte>>4)&1)==1) {
        return 3;
    } else if (((byte>>3)&1)==1) {
        return 4;
    } else if (((byte>>2)&1)==1) {
        return 5;
    } else if (((byte>>1)&1)==1) {
        return 6;
    } else if (((byte>>0)&1)==1) {
        return 7;
    }
    return 8;
}

long long int HalSMInteger_ctz(HalSMInteger* a)
{
    if (a->size==0){return 0;}
    long long int out=0,i;
    for (i=a->size-1;;i--) {
        if (a->value[i]==0) {
            out+=8;
        } else {
            out+=HalSMInteger_ByteCTZ(a->value[i]);
            break;
        }
        if (i==0) {
            break;
        }
    }
    return out;
}

//Need change

void HalSMInteger_BitORSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    if (b->size==0&&a->size==0) {return;}
    unsigned long long int i,cm,j;
    unsigned char tempb;
    cm=a->size;
    if (a->size<b->size) {
        a->value=HalSMMemory_realloc(memory,a->value,b->size);
        for (i=a->size;i<b->size;i++) {
            a->value[i]=0;
        }
        a->size=b->size;
        cm=b->size;
    }
    for (i=0,j=0;i<cm;i++) {
        if (j==b->size) {
            tempb=0;
        } else {
            tempb=b->value[j];
            j++;
        }
        a->value[i]=a->value[i]|tempb;
    }
}

HalSMInteger* HalSMInteger_BitOR(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    HalSMInteger* out=HalSMInteger_copy(memory,a);
    HalSMInteger_BitORSelf(memory,out,b);
    return out;
}

void HalSMInteger_BitXORSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    if (b->size==0&&a->size==0) {return;}
    unsigned long long int i,cm,j;
    unsigned char tempb;
    cm=a->size;
    if (a->size<b->size) {
        a->value=HalSMMemory_realloc(memory,a->value,b->size);
        for (i=a->size;i<b->size;i++) {
            a->value[i]=0;
        }
        a->size=b->size;
        cm=b->size;
    }
    for (i=0,j=0;i<cm;i++) {
        if (j==b->size) {
            tempb=0;
        } else {
            tempb=b->value[j];
            j++;
        }
        a->value[i]=a->value[i]^tempb;
    }
}

HalSMInteger* HalSMInteger_BitXOR(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    HalSMInteger* out=HalSMInteger_copy(memory,a);
    HalSMInteger_BitXORSelf(memory,out,b);
    return out;
}

void HalSMInteger_BitANDSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    if (b->size==0&&a->size==0) {return;}
    unsigned long long int i,cm,j;
    unsigned char tempb;
    cm=a->size;
    if (a->size<b->size) {
        a->value=HalSMMemory_realloc(memory,a->value,b->size);
        for (i=a->size;i<b->size;i++) {
            a->value[i]=0;
        }
        a->size=b->size;
        cm=b->size;
    }
    for (i=0,j=0;i<cm;i++) {
        if (j==b->size) {
            tempb=0;
        } else {
            tempb=b->value[j];
            j++;
        }
        a->value[i]=a->value[i]&tempb;
    }
}

HalSMInteger* HalSMInteger_BitAND(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b)
{
    HalSMInteger* out=HalSMInteger_copy(memory,a);
    HalSMInteger_BitANDSelf(memory,out,b);
    return out;
}

void HalSMInteger_BitNOTSelf(HalSMMemory* memory,HalSMInteger* a)
{
    if (a->size==0) {return;}
    unsigned long long int i;
    for (i=0;i<a->size;i++) {
        a->value[i]=~a->value[i];
    }
}

HalSMInteger* HalSMInteger_BitNOT(HalSMMemory* memory,HalSMInteger* a)
{
    HalSMInteger* out=HalSMInteger_copy(memory,a);
    HalSMInteger_BitNOTSelf(memory,out);
    return out;
}

HalSMInteger* HalSMInteger_FromHalSMDouble(HalSMMemory* memory,HalSMDouble* a)
{
    HalSMInteger remainder;
    HalSMInteger* out=HalSMInteger_Div(memory,a->a,a->b,&remainder);
    if (HalSMInteger_isLess(&remainder,HalSMInteger_Div(memory,a->b,&HalSMIntegerTwo,NOTHALSMNULLPOINTER))==0) {
        HalSMInteger_AddSelf(memory,out,&HalSMIntegerOne);
    }
    return out;
}

HalSMInteger* HalSMInteger_FloorHalSMDouble(HalSMMemory* memory,HalSMDouble* a)
{
    HalSMInteger* out=HalSMInteger_Div(memory,a->a,a->b,NOTHALSMNULLPOINTER);
    out->negative=a->negative;
    return out;
}

HalSMInteger* HalSMInteger_CeilHalSMDouble(HalSMMemory* memory,HalSMDouble* a)
{
    HalSMInteger remainder;
    HalSMInteger* out=HalSMInteger_Div(memory,a->a,a->b,&remainder);
    if (HalSMInteger_isMore(&remainder,&HalSMIntegerZero)) {
        HalSMInteger_AddSelf(memory,out,&HalSMIntegerOne);
    }
    out->negative=a->negative;
    return out;
}

//HalSMInteger