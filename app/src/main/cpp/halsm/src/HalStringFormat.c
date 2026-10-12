#include <HalStringFormat.h>

//HALSTRINGFORMATCHARACTERS

//ASCII
//NUL,SOH,STX,ETX,EOT,ENQ,ACK,BEL,BS,TAB,LF,VT,FF,CR,SO,SI,DLE,DC1,DC2,DC3,DC4,NAK,SYN,ETB,CAN
//EM,SUB,ESC,FS,GS,RS,US,SPACE,!,",#,$,%,&,',(,),*,+,,,-,.,/,0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,?,@
//ABCDEFGHIJKLMNOPQRSTUVWXYZ,[,\,],^,_,`,abcdefghijklmnopqrstuvwxyz,{,|,},~,DEL

//Russian
//АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя

//Test
//"ℑ", "ᛍ", "雷", "⌬", "≪", "Υ", "ᛡ", "ƒ", "е", "≿", "⊧", "Ԋ", "п", "ϑ", "⍎", "A", "℀", "∕", "雲", "⊵", "ℛ", "Х", "ψ", "ҏ", "ɍ", "Ӗ", "ⅅ", "p", "ѧ", "Ц", "⊠", "ɠ", "Ҩ", "⌔", "≞", "∛", "ү", "Ӓ", "⌈", "秋", "Ѕ", "⍓", "ꝱ", "ꝋ", "Б", "≲", "ß", "⋔", "ᚸ", "⊆", "⊎", "ᚠ", "ꝉ", "ӟ", "∞", "ᚼ", "⌍", "⌁", "ℶ", "⋒", "Ӊ", "⊓", "Ѫ", "g", "湖", "Ӝ", "Ҵ", "Ұ", "⍁", "≂", "Χ", "幻", "⊢", "Ѧ", "Ⅎ", "≕", "Z", "⌗", "月", "≈", "∍", "ұ", "ԋ", "ю", "З", "∵", "қ", "ӽ", "f", "⌽", "∝", "Y", "ᛊ", "−", "⌥", "⋈", "π", "⌧", "ᛎ", "≽", "ѥ", "ɦ", "ꞷ", "Ѡ", "≋", "⋧", "⌂", "⋀", "⊻", "廉", "Ч", "ц", "≟", "Њ", "⋚", "⍌", "ℊ", "J", "鼎", "⋕", "ⅈ", "ℒ", "j", "ℋ", "ҙ", "Ӫ", "ҷ", "⍚", "C", "Ƥ", "ӄ", "ꝳ", "⊏", "⅏", "℈", "і", "⅌", "υ", "ᛑ", "孝", "Ѣ", "≚", "⍀", "Ψ", "O", "ȿ", "⊉", "ρ", "⅁", "з", "Ε", "ᛋ", "ꝥ", "⍑", "≨", "℄", "ᚬ", "ӝ", "ҩ", "F", "日"

//HALSTRINGFORMATCHARACTERS

void*(*hsf_malloc_func)(unsigned long long int x);

void* hsf_malloc(unsigned long long int x){return hsf_malloc_func(x);}

void HalStringFormat_init(void*(*hmf)(unsigned long long int)){hsf_malloc_func=hmf;}

unsigned long long int HalStringFormat_length(HalStringFormatChar* str)
{
    unsigned int* intchars=(unsigned int*)str;
    unsigned long i=0;
    while (*intchars++!=0) {
        i++;
    }
    return i;
}

int HalUTF8_indexByUTF8Char(unsigned int chr)
{
    unsigned long long int i;
    int out=-1;
    for (i=0;i<UTF8HalStringFormatSize;i++) {
        if (UTF8HalStringFormatCharacters[i].utf8c==chr) {
            out=i;
            break;
        }
    }
    return out;
}

int HalUTF8_indexByHalStringFormatChar(HalStringFormatChar chr)
{
    unsigned long long int i;
    unsigned long long int n=*(unsigned int*)&(chr);
    int out=-1;
    for (i=0;i<UTF8HalStringFormatSize;i++) {
        if (*(unsigned int*)&UTF8HalStringFormatCharacters[i].hsfc==n) {
            out=i;
            break;
        }
    }
    return out;
}

unsigned long long int UTF8String_length(char* str,unsigned long long int size)
{
    unsigned char* ustr=(unsigned char*)str;
    unsigned long long int out=0;
    for (unsigned long long int i=0;i<size;i++) {
        if ((ustr[i] & 0xC0) != 0x80) {
            out++;
        }
    }
    return out;
}

HalStringFormatChar U8CH(unsigned int chr)
{
    int index=HalUTF8_indexByUTF8Char(chr);
    return (index==-1)?(HalStringFormatChar){0,0,0}:UTF8HalStringFormatCharacters[index].hsfc;
}

HalStringFormatChar ASCIIH(char chr)
{
    return UTF8HalStringFormatCharacters[chr].hsfc;
    //return (HalStringFormatChar){0,chr,0};
}

HalStringFormatChar* UTF8String_toHalStringFormat(HalStringFormatChar* out,char* str,unsigned long long int size)
{
    unsigned char* ustr=(unsigned char*)str;
    unsigned long long int ind=0;
    unsigned long long int i=0;
    while (i<size) {
        if (utf8_is_single_byte((&str[i]))) {
            out[ind++]=U8CH(ustr[i]);
            i++;
        } else if (utf8_is_double_byte((&str[i]))) {
            out[ind++]=U8CH((ustr[i]<<8)|ustr[i+1]);
            i+=2;
        } else if (utf8_is_triple_byte((&str[i]))) {
            out[ind++]=U8CH(ustr[i+2]+(ustr[i+1]<<8)+(ustr[i]<<16));
            i+=3;
        } else if (utf8_is_quadruple_byte((&str[i]))) {
            out[ind++]=U8CH(ustr[i+3]+(ustr[i+2]<<8)+(ustr[i+1]<<16)+(ustr[i]<<24));
            i+=4;
        } else {
            i++;
        }
    }
    return out;
}

char* HalStringFormat_toUTF8(char* out,HalStringFormatChar* str,unsigned long long int size)
{
    unsigned int utf8c;
    int ind;
    char* outp=out;
    for (unsigned long long int i=0;i<size;i++) {
        ind=HalUTF8_indexByHalStringFormatChar(str[i]);
        if (ind==-1) {
            continue;
        }
        utf8c=UTF8HalStringFormatCharacters[ind].utf8c;
        if (utf8c<=0xFF) {
            *(outp++)=(char)utf8c;
        } else if (utf8c<=0xFFFF) {
            *(outp++)=(char)((utf8c>>8)&0xFF);
            *(outp++)=(char)(utf8c&0xFF);
        } else if (utf8c<=0xFFFFFF) {
            *(outp++)=(char)((utf8c>>16)&0xFF);
            *(outp++)=(char)((utf8c>>8)&0xFF);
            *(outp++)=(char)(utf8c&0xFF);
        } else {
            *(outp++)=(char)((utf8c>>24)&0xFF);
            *(outp++)=(char)((utf8c>>16)&0xFF);
            *(outp++)=(char)((utf8c>>8)&0xFF);
            *(outp++)=(char)(utf8c&0xFF);
        }
    }
    *(outp++)='\0';
    return out;
}

unsigned long long int HalStringFormat_UTF8GetLength(HalStringFormatChar* str,unsigned long long int size)
{
    unsigned int utf8c;
    unsigned long long int ind;
    unsigned long long int out=0;
    
    for (unsigned long long int i=0;i<size;i++) {
        ind=HalUTF8_indexByHalStringFormatChar(str[i]);
        if (ind==-1) {
            continue;
        }
        utf8c=UTF8HalStringFormatCharacters[ind].utf8c;
        if (utf8c<=0xFF) {
            out++;
        } else if (utf8c<=0xFFFF) {
            out+=2;
        } else if (utf8c<=0xFFFFFF) {
            out+=3;
        } else {
            out+=4;
        }
    }
    return out;
}

HalStringFormatChar* U8H(char* str)
{
    unsigned long long int string_size=0;
    char* pstr;
    for (pstr=str;;pstr++) {
        if (*pstr=='\0'){string_size=pstr-str;break;}
    }
    string_size++;
    unsigned long long int size=UTF8String_length(str,string_size);
    HalStringFormatChar* out=(HalStringFormatChar*)hsf_malloc(size*sizeof(HalStringFormatChar));
    UTF8String_toHalStringFormat(out,str,string_size);
    return out;
}

char* HU8(HalStringFormatChar* str)
{
    unsigned long long int l=HalStringFormat_length(str);
    unsigned long long int size=HalStringFormat_UTF8GetLength(str,l)+1;
    char* out=hsf_malloc(size);
    HalStringFormat_toUTF8(out,str,l);
    return out;
}

unsigned char HSFCharCompare(HalStringFormatChar a,HalStringFormatChar b)
{
    return *(unsigned int*)&a==*(unsigned int*)&b;
}

unsigned char HSFCompare(HalStringFormatChar* a,HalStringFormatChar* b)
{
    unsigned long long int i,l;
    l=HalStringFormat_length(a);
    if (l!=HalStringFormat_length(b)){return 0;}
    for (i=0;i<l;i++) {
        if (HSFCharCompare(*a++,*b++)==0){return 0;}
    }
    return 1;
}

char* HSF2ASCII(HalStringFormatChar* str)
{
    unsigned long long i,l;
    l=HalStringFormat_length(str);
    char* out=hsf_malloc(l+1);
    HalStringFormatChar chr;
    int ind;
    for (i=0;i<l;i++) {
        chr=*str++;
        if (chr.alphabet>0) {
            *out++=' ';
        } else {
            ind=HalUTF8_indexByHalStringFormatChar(str[i]);
            if (ind>-1){*out++=(char)UTF8HalStringFormatCharacters[ind].utf8c;}
        }
    }
    out[l]='\0';
    return out;
}

//UTF16

int HalUTF16_indexByUTF16Char(unsigned int chr)
{
    unsigned long long int i;
    int out=-1;
    for (i=0;i<UTF16HalStringFormatSize;i++) {
        if (UTF16HalStringFormatCharacters[i].utf16c==chr) {
            out=i;
            break;
        }
    }
    return out;
}

int HalUTF16_indexByHalStringFormatChar(HalStringFormatChar chr)
{
    unsigned long long int i;
    unsigned long long int n=*(unsigned int*)&(chr);
    int out=-1;
    for (i=0;i<UTF16HalStringFormatSize;i++) {
        if (*(unsigned int*)&UTF16HalStringFormatCharacters[i].hsfc==n) {
            out=i;
            break;
        }
    }
    return out;
}

unsigned long long int UTF16String_length(short* str,unsigned long long int size)
{
    short* ustr=(short*)str;
    unsigned long long int ind=0;
    unsigned long long int i=0;
    unsigned long long int out=0;
    while (i<size) {
        if (U16_IS_SINGLE((str[i]))) {
            out++;
            i++;
        } else {
            out++;
            i+=2;
        }
    }
    return out;
}

HalStringFormatChar U16CH(unsigned int chr)
{
    int index=HalUTF16_indexByUTF16Char(chr);
    return (index==-1)?(HalStringFormatChar){0,0,0}:UTF16HalStringFormatCharacters[index].hsfc;
}

HalStringFormatChar* UTF16String_toHalStringFormat(HalStringFormatChar* out,short* str,unsigned long long int size)
{
    unsigned long long int ind=0;
    unsigned long long int i=0;
    while (i<size) {
        if (U16_IS_SINGLE((str[i]))) {
            out[ind++]=U16CH(str[i]);
            i++;
        } else {
            out[ind++]=U16CH((str[i]<<16)|str[i]);
            i+=2;
        }
    }
    return out;
}

short* HalStringFormat_toUTF16(short* out,HalStringFormatChar* str,unsigned long long int size)
{
    unsigned int utf16c;
    int ind;
    short* outp=out;
    for (unsigned long long int i=0;i<size;i++) {
        ind=HalUTF8_indexByHalStringFormatChar(str[i]);
        if (ind==-1) {
            continue;
        }
        utf16c=UTF16HalStringFormatCharacters[ind].utf16c;
        if (utf16c<=0xFFFF) {
            *(outp++)=(short)utf16c;
        } else {
            *(outp++)=(short)((utf16c>>16)&0xFFFF);
            *(outp++)=(short)(utf16c&0xFFFF);
        }
    }
    *(outp++)='\0';
    return out;
}

unsigned long long int HalStringFormat_UTF16GetLength(HalStringFormatChar* str,unsigned long long int size)
{
    unsigned int utf16c;
    unsigned long long int ind;
    unsigned long long int out=0;
    
    for (unsigned long long int i=0;i<size;i++) {
        ind=HalUTF16_indexByHalStringFormatChar(str[i]);
        if (ind==-1) {
            continue;
        }
        utf16c=UTF16HalStringFormatCharacters[ind].utf16c;
        if (utf16c<=0xFFFF) {
            out++;
        } else {
            out+=2;
        }
    }
    return out;
}

HalStringFormatChar* U16H(short* str)
{
    unsigned long long int string_size=0;
    short* pstr;
    for (pstr=str;;pstr++) {
        if (*pstr=='\0'){string_size=pstr-str;break;}
    }
    string_size++;
    unsigned long long int size=UTF16String_length(str,string_size);
    HalStringFormatChar* out=(HalStringFormatChar*)hsf_malloc(size*sizeof(HalStringFormatChar));
    UTF16String_toHalStringFormat(out,str,string_size);
    return out;
}

short* HU16(HalStringFormatChar* str)
{
    unsigned long long int l=HalStringFormat_length(str);
    unsigned long long int size=HalStringFormat_UTF16GetLength(str,l)+sizeof(short);
    short* out=(short*)hsf_malloc(size);
    HalStringFormat_toUTF16(out,str,l);
    return out;
}

//UTF16