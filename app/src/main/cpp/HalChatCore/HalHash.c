#include <HalHash.h>

// Bin2Hash считает на своей длинной арифметике (основание 2^32) и на обычных
// 64-битных числах, когда значение в них помещается. Значения те же, что у
// HalHash.js на BigInt:
//   половины десятичной записи = x / 10^k и x % 10^k, где k = digits - digits/2
//   склейка BigInt(a.toString()+b.toString()) = a * 10^digits(b) + b
//   d[b] += t[i]^2 + t[i]*g для всех i  ==  d[b] = t[b] + S, где S = сумма по i
// Вся память, выделенная внутри функций, освобождается до выхода.
// Без стандартной библиотеки и глобальных переменных: память только через функции HalHash.

// ---------- длинные числа ----------

typedef struct HHBig {
    unsigned int* d;
    unsigned int n;
} HHBig;

static const unsigned long long HHPow10[20]={
    1ULL,10ULL,100ULL,1000ULL,10000ULL,100000ULL,1000000ULL,10000000ULL,100000000ULL,
    1000000000ULL,10000000000ULL,100000000000ULL,1000000000000ULL,10000000000000ULL,
    100000000000000ULL,1000000000000000ULL,10000000000000000ULL,100000000000000000ULL,
    1000000000000000000ULL,10000000000000000000ULL
};

static const char HHHex[]="0123456789abcdef";

#define HH_U64_MAX 18446744073709551615ULL

static HHBig hhAlloc(HalHash* hh,unsigned int n) {
    HHBig r;
    r.n=n;
    r.d=n?hh->calloc(n,sizeof(unsigned int)):0;
    return r;
}

static void hhFree(HalHash* hh,HHBig* a) {
    if(a->d)hh->free(a->d);
    a->d=0;
    a->n=0;
}

static void hhNorm(HHBig* a) {
    while(a->n>0&&a->d[a->n-1]==0)a->n--;
}

static HHBig hhFromU64(HalHash* hh,unsigned long long v) {
    HHBig r=hhAlloc(hh,2);
    r.d[0]=(unsigned int)v;
    r.d[1]=(unsigned int)(v>>32);
    hhNorm(&r);
    return r;
}

// Байты как число big-endian, как BigInt("0x"+Bin2Hex(bytes)) в HalHash.js.
static HHBig hhFromBytesBE(HalHash* hh,const unsigned char* b,unsigned long long len) {
    unsigned long long k;
    HHBig r=hhAlloc(hh,(unsigned int)((len+3)/4));
    for(k=0;k<len;k++) {
        r.d[k/4]|=((unsigned int)b[len-1-k])<<(8*(k%4));
    }
    hhNorm(&r);
    return r;
}

static unsigned char hhFitsU64(HHBig a,unsigned long long* v) {
    if(a.n>2)return 0;
    *v=a.n==0?0:(a.n==1?a.d[0]:(a.d[0]|((unsigned long long)a.d[1]<<32)));
    return 1;
}

static HHBig hhAdd(HalHash* hh,HHBig a,HHBig b) {
    unsigned int n=(a.n>b.n?a.n:b.n)+1,i;
    unsigned long long s,carry=0;
    HHBig r=hhAlloc(hh,n);
    for(i=0;i<n;i++) {
        s=carry;
        if(i<a.n)s+=a.d[i];
        if(i<b.n)s+=b.d[i];
        r.d[i]=(unsigned int)s;
        carry=s>>32;
    }
    hhNorm(&r);
    return r;
}

static HHBig hhMul(HalHash* hh,HHBig a,HHBig b) {
    unsigned int i,j;
    unsigned long long t,carry;
    HHBig r;
    if(a.n==0||b.n==0)return hhAlloc(hh,0);
    r=hhAlloc(hh,a.n+b.n);
    for(i=0;i<a.n;i++) {
        carry=0;
        for(j=0;j<b.n;j++) {
            t=(unsigned long long)a.d[i]*b.d[j]+r.d[i+j]+carry;
            r.d[i+j]=(unsigned int)t;
            carry=t>>32;
        }
        r.d[i+b.n]=(unsigned int)carry;
    }
    hhNorm(&r);
    return r;
}

static unsigned int hhU64ToDec(unsigned long long v,char* out) {
    char tmp[21];
    unsigned int n=0,i;
    do {
        tmp[n++]=(char)('0'+v%10);
        v/=10;
    } while(v);
    for(i=0;i<n;i++)out[i]=tmp[n-1-i];
    return n;
}

static unsigned int hhU64ToHex(unsigned long long v,char* out) {
    char tmp[17];
    unsigned int n=0,i;
    do {
        tmp[n++]=HHHex[v&15];
        v>>=4;
    } while(v);
    for(i=0;i<n;i++)out[i]=tmp[n-1-i];
    return n;
}

// Десятичная запись без ведущих нулей ("0" для нуля). Память освобождает вызывающий.
static char* hhToDec(HalHash* hh,HHBig a,unsigned int* outLen) {
    unsigned int n=a.n,i,chunks=0,len=0,k,z;
    unsigned long long cur,rem;
    unsigned int* t=n?hh->malloc(sizeof(unsigned int)*n):0;
    unsigned int* parts=hh->malloc(sizeof(unsigned int)*(n*2+2));
    char* s=hh->malloc(n*10+12);
    for(i=0;i<n;i++)t[i]=a.d[i];
    while(n>0) {
        rem=0;
        for(i=n;i-->0;) {
            cur=(rem<<32)|t[i];
            t[i]=(unsigned int)(cur/1000000000ULL);
            rem=cur%1000000000ULL;
        }
        parts[chunks++]=(unsigned int)rem;
        while(n>0&&t[n-1]==0)n--;
    }
    if(chunks==0) {
        s[len++]='0';
    } else {
        len+=hhU64ToDec(parts[chunks-1],s);
        for(k=chunks-1;k-->0;) {
            for(z=9;z-->0;) {
                s[len+z]=(char)('0'+parts[k]%10);
                parts[k]/=10;
            }
            len+=9;
        }
    }
    s[len]=0;
    if(t)hh->free(t);
    hh->free(parts);
    *outLen=len;
    return s;
}

// Разбор десятичной строки; пустая строка = 0, ведущие нули допустимы.
static HHBig hhFromDec(HalHash* hh,const char* s,unsigned int len) {
    unsigned int used=0,pos=0,k,i;
    unsigned long long chunk,t,carry;
    HHBig r;
    if(len==0)return hhAlloc(hh,0);
    r=hhAlloc(hh,len/9+2);
    while(pos<len) {
        k=(pos==0&&len%9)?len%9:9;
        chunk=0;
        for(i=0;i<k;i++)chunk=chunk*10+(unsigned long long)(s[pos+i]-'0');
        pos+=k;
        carry=chunk;
        for(i=0;i<used;i++) {
            t=(unsigned long long)r.d[i]*HHPow10[k]+carry;
            r.d[i]=(unsigned int)t;
            carry=t>>32;
        }
        while(carry) {
            r.d[used++]=(unsigned int)carry;
            carry>>=32;
        }
    }
    r.n=used;
    hhNorm(&r);
    return r;
}

static unsigned long long hhHexLen(HHBig a) {
    unsigned int top,k=0;
    if(a.n==0)return 1;
    top=a.d[a.n-1];
    while(top) {
        k++;
        top>>=4;
    }
    return k+8ULL*(a.n-1);
}

static unsigned int hhToHex(HHBig a,char* out) {
    unsigned int len,i,z;
    if(a.n==0) {
        out[0]='0';
        return 1;
    }
    len=hhU64ToHex(a.d[a.n-1],out);
    for(i=a.n-1;i-->0;) {
        for(z=0;z<8;z++)out[len+z]=HHHex[(a.d[i]>>(28-4*z))&15];
        len+=8;
    }
    return len;
}

// ---------- число: маленькое в unsigned long long или большое HHBig ----------

typedef struct HalHashNum {
    HHBig big;
    unsigned long long small;
    unsigned char isSmall;
} HalHashNum;

static unsigned int HalHash_decDigits(unsigned long long v) {
    unsigned int n=1;
    while(v>=10) {
        v/=10;
        n++;
    }
    return n;
}

static HalHashNum numSmall(unsigned long long v) {
    HalHashNum r;
    r.big.d=0;
    r.big.n=0;
    r.small=v;
    r.isSmall=1;
    return r;
}

// Забирает b себе; если помещается в 64 бита - освобождает и хранит как маленькое.
static HalHashNum numFromBig(HalHash* hh,HHBig b) {
    HalHashNum r;
    unsigned long long v;
    if(hhFitsU64(b,&v)) {
        hhFree(hh,&b);
        return numSmall(v);
    }
    r.big=b;
    r.small=0;
    r.isSmall=0;
    return r;
}

static void numFree(HalHash* hh,HalHashNum* a) {
    if(!a->isSmall)hhFree(hh,&a->big);
}

static HHBig numBig(HalHash* hh,HalHashNum* a,unsigned char* isTemp) {
    if(a->isSmall) {
        *isTemp=1;
        return hhFromU64(hh,a->small);
    }
    *isTemp=0;
    return a->big;
}

static char* numToDec(HalHash* hh,HalHashNum* a,unsigned int* len) {
    char* s;
    if(!a->isSmall)return hhToDec(hh,a->big,len);
    s=hh->malloc(21);
    *len=hhU64ToDec(a->small,s);
    s[*len]=0;
    return s;
}

static unsigned long long numHexLen(HalHashNum* a) {
    unsigned long long v,len=1;
    if(!a->isSmall)return hhHexLen(a->big);
    v=a->small;
    while(v>=16) {
        v>>=4;
        len++;
    }
    return len;
}

static unsigned int numToHex(HalHashNum* a,char* out) {
    if(!a->isSmall)return hhToHex(a->big,out);
    return hhU64ToHex(a->small,out);
}

static void numHalves(HalHash* hh,HalHashNum* x,HalHashNum* hi,HalHashNum* lo) {
    unsigned int digits,len;
    unsigned long long p;
    char* s;
    if(x->isSmall) {
        digits=HalHash_decDigits(x->small);
        p=HHPow10[digits-digits/2];
        *hi=numSmall(x->small/p);
        *lo=numSmall(x->small%p);
        return;
    }
    s=hhToDec(hh,x->big,&len);
    *hi=numFromBig(hh,hhFromDec(hh,s,len/2));
    *lo=numFromBig(hh,hhFromDec(hh,s+len/2,len-len/2));
    hh->free(s);
}

static HalHashNum numAdd(HalHash* hh,HalHashNum a,HalHashNum b) {
    unsigned char ta,tb;
    HHBig x,y,r;
    if(a.isSmall&&b.isSmall&&a.small<=HH_U64_MAX-b.small)return numSmall(a.small+b.small);
    x=numBig(hh,&a,&ta);
    y=numBig(hh,&b,&tb);
    r=hhAdd(hh,x,y);
    if(ta)hhFree(hh,&x);
    if(tb)hhFree(hh,&y);
    return numFromBig(hh,r);
}

static HalHashNum numDoublePlus(HalHash* hh,HalHashNum a,HalHashNum g) {
    unsigned char ta,tg;
    HHBig x,y,two,m,r;
    if(a.isSmall&&g.isSmall&&a.small<=(HH_U64_MAX-g.small)/2)return numSmall(a.small*2+g.small);
    x=numBig(hh,&a,&ta);
    y=numBig(hh,&g,&tg);
    two=hhFromU64(hh,2);
    m=hhMul(hh,x,two);
    r=hhAdd(hh,m,y);
    hhFree(hh,&two);
    hhFree(hh,&m);
    if(ta)hhFree(hh,&x);
    if(tg)hhFree(hh,&y);
    return numFromBig(hh,r);
}

static HalHashNum numConcat(HalHash* hh,HalHashNum a,HalHashNum b) {
    unsigned int da,db,i;
    char* sa,*sb,*buf;
    HHBig r;
    if(a.isSmall&&b.isSmall) {
        da=HalHash_decDigits(a.small);
        db=HalHash_decDigits(b.small);
        if(da+db<=19)return numSmall(a.small*HHPow10[db]+b.small);
    }
    sa=numToDec(hh,&a,&da);
    sb=numToDec(hh,&b,&db);
    buf=hh->malloc(da+db+1);
    for(i=0;i<da;i++)buf[i]=sa[i];
    for(i=0;i<db;i++)buf[da+i]=sb[i];
    r=hhFromDec(hh,buf,da+db);
    hh->free(sa);
    hh->free(sb);
    hh->free(buf);
    return numFromBig(hh,r);
}

// ---------- API ----------

void HalHash_init(HalHash* hh,void*(*mallocFunc)(unsigned long long),void*(*callocFunc)(unsigned long long,unsigned long long),void(*freeFunc)(void*))
{
    hh->malloc=mallocFunc;
    hh->calloc=callocFunc;
    hh->free=freeFunc;
}

char* HalHash_Str2Hash(HalHash* hh,const char* str,unsigned int length,unsigned int count,unsigned long long maxIteration)
{
    unsigned long long size=0;
    while(str[size])size++;
    return HalHash_Bin2Hash(hh,(const unsigned char*)str,size,length,count,maxIteration);
}

char* HalHash_Bin2Hex(HalHash* hh,const unsigned char* bytes,unsigned long long size)
{
    unsigned long long j;
    char* out=hh->malloc(size*2+1);
    for(j=0;j<size;j++) {
        out[j*2]=HHHex[bytes[j]>>4];
        out[j*2+1]=HHHex[bytes[j]&0x0f];
    }
    out[size*2]=0;
    return out;
}

char* HalHash_Bin2Hash(HalHash* hh,const unsigned char* bytes,unsigned long long size,unsigned int length,unsigned int count,unsigned long long maxIteration)
{
    unsigned long long mc,c,g,g2,pos,sizeD,ind;
    unsigned int i,p,b,lc,total,digits;
    HHBig* d,x,gb,g2b,tp,iv,sq,t2,s2,cur,m,S;
    HalHashNum* n,gn,hi0,lo0,hi1,lo1,a0,a1,r0,r1;
    char* hex;

    if(count>length)count=length;
    if(count>size)count=(unsigned int)size;
    // count=0 (пустой вход или length=0): как в HalHash.js остаются только дополнительные числа
    sizeD=count?size/count:0;
    g=(unsigned long long)count*length;
    p=(length-count)/2;
    total=count+p;
    // Чисел нет: при length=0 HalHash.js тоже даёт "", при пустом входе и length=1 он зацикливается
    if(total==0) {
        hex=hh->malloc(1);
        hex[0]=0;
        return hex;
    }
    d=hh->malloc(sizeof(HHBig)*total);
    gb=hhFromU64(hh,g);

    // Куски входа, умноженные на g.
    for(i=0;i<count;i++) {
        x=hhFromBytesBE(hh,bytes+i*sizeD,sizeD);
        d[i]=hhMul(hh,x,gb);
        hhFree(hh,&x);
    }
    if(count&&size%count>0) {
        ind=count*sizeD-sizeD;
        x=hhFromBytesBE(hh,bytes+ind,size-ind);
        hhFree(hh,&d[count-1]);
        d[count-1]=hhMul(hh,x,gb);
        hhFree(hh,&x);
    }

    // Дополнительные числа: x = i+2, затем x = x*d[b] + g*(i+1) для каждого куска.
    for(i=0;i<p;i++) {
        cur=hhFromU64(hh,i+2);
        iv=hhFromU64(hh,i+1);
        tp=hhMul(hh,gb,iv);
        hhFree(hh,&iv);
        for(b=0;b<count;b++) {
            m=hhMul(hh,cur,d[b]);
            hhFree(hh,&cur);
            cur=hhAdd(hh,m,tp);
            hhFree(hh,&m);
        }
        hhFree(hh,&tp);
        d[count+i]=cur;
    }

    // g2 = сумма половин десятичной записи g.
    digits=HalHash_decDigits(g);
    g2=g/HHPow10[digits-digits/2]+g%HHPow10[digits-digits/2];
    g2b=hhFromU64(hh,g2);

    // Перемешивание: S = сумма (t^2 + t*g2), затем d[b] = t[b] + S.
    S=hhAlloc(hh,0);
    for(i=0;i<total;i++) {
        sq=hhMul(hh,d[i],d[i]);
        tp=hhMul(hh,d[i],g2b);
        t2=hhAdd(hh,sq,tp);
        s2=hhAdd(hh,S,t2);
        hhFree(hh,&sq);
        hhFree(hh,&tp);
        hhFree(hh,&t2);
        hhFree(hh,&S);
        S=s2;
    }
    n=hh->malloc(sizeof(HalHashNum)*total);
    for(i=0;i<total;i++) {
        s2=hhAdd(hh,d[i],S);
        hhFree(hh,&d[i]);
        n[i]=numFromBig(hh,s2);
    }
    hhFree(hh,&S);
    hhFree(hh,&gb);
    hhFree(hh,&g2b);
    hh->free(d);
    gn=numSmall(g2);

    // Подгонка длины до length hex-символов.
    lc=total-1;
    for(mc=0;;mc++) {
        c=0;
        for(i=0;i<total;i++)c+=numHexLen(&n[i]);
        if(c<length) {
            for(i=0;i<total;i++) {
                if(i==lc) {
                    r0=numDoublePlus(hh,n[i],gn);
                    numFree(hh,&n[i]);
                    n[i]=r0;
                } else {
                    numHalves(hh,&n[i],&hi0,&lo0);
                    numHalves(hh,&n[i+1],&hi1,&lo1);
                    a0=numAdd(hh,hi0,lo0);
                    a1=numAdd(hh,hi1,lo1);
                    numFree(hh,&hi0);
                    numFree(hh,&lo0);
                    numFree(hh,&hi1);
                    numFree(hh,&lo1);
                    r0=numConcat(hh,a0,a1);
                    r1=numConcat(hh,a1,a0);
                    numFree(hh,&a0);
                    numFree(hh,&a1);
                    numFree(hh,&n[i]);
                    numFree(hh,&n[i+1]);
                    n[i]=r0;
                    n[i+1]=r1;
                }
            }
        } else if(c>length) {
            if(mc>maxIteration)break;
            for(i=0;i<total;i++) {
                if(i==lc) {
                    numHalves(hh,&n[i],&hi0,&lo0);
                    a0=numAdd(hh,hi0,lo0);
                    r0=numAdd(hh,a0,gn);
                    numFree(hh,&hi0);
                    numFree(hh,&lo0);
                    numFree(hh,&a0);
                    numFree(hh,&n[i]);
                    n[i]=r0;
                } else {
                    numHalves(hh,&n[i],&hi0,&lo0);
                    numHalves(hh,&n[i+1],&hi1,&lo1);
                    r0=numAdd(hh,hi0,lo1);
                    r1=numAdd(hh,hi1,lo0);
                    numFree(hh,&hi0);
                    numFree(hh,&lo0);
                    numFree(hh,&hi1);
                    numFree(hh,&lo1);
                    numFree(hh,&n[i]);
                    numFree(hh,&n[i+1]);
                    n[i]=r0;
                    n[i+1]=r1;
                }
            }
        } else {
            break;
        }
    }

    // Результат: все числа в hex подряд; при обрезке по maxIteration - первые length символов.
    hex=hh->malloc(c+1);
    pos=0;
    for(i=0;i<total;i++) {
        pos+=numToHex(&n[i],hex+pos);
        numFree(hh,&n[i]);
    }
    hh->free(n);
    hex[c>length?length:c]=0;
    return hex;
}
