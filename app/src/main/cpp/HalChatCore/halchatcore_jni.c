//JNI HalChatCore (HalHash.c). Java: halwarsing.net.halchatandroid.main.HalHash
#include <jni.h>
#include <stdlib.h>
#include <HalHash.h>

#define HCC_JNI(ret,name) JNIEXPORT ret JNICALL Java_halwarsing_net_halchatandroid_main_HalHash_##name

static void* HML_malloc(unsigned long long size){return malloc(size);}
static void* HML_calloc(unsigned long long n,unsigned long long size){return calloc(n,size);}
static void HML_free(void* p){free(p);}

//Только читается после JNI_OnLoad, поэтому общий для всех потоков
static HalHash hh;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm,void* reserved) {
    HalHash_init(&hh,HML_malloc,HML_calloc,HML_free);
    return JNI_VERSION_1_6;
}

/**
 * HalHash_Bin2Hash над байтами из Java
 *
 * @param bytes Входные байты
 * @param length Длина хэша в hex-символах
 * @param count Количество кусков
 * @param maxIteration Предел итераций подгонки длины
 * @return Хэш строкой (hex в нижнем регистре)
 */
HCC_JNI(jstring,nativeBin2Hash)(JNIEnv* env,jclass cls,jbyteArray bytes,jint length,jint count,jlong maxIteration) {
    jsize size=(*env)->GetArrayLength(env,bytes);
    jbyte* b=(*env)->GetByteArrayElements(env,bytes,NULL);
    char* h;
    jstring out;
    if(b==NULL)return NULL;
    h=HalHash_Bin2Hash(&hh,(const unsigned char*)b,(unsigned long long)size,(unsigned int)length,(unsigned int)count,(unsigned long long)maxIteration);
    (*env)->ReleaseByteArrayElements(env,bytes,b,JNI_ABORT);
    out=(*env)->NewStringUTF(env,h);
    hh.free(h);
    return out;
}
